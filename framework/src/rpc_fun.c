#include <string.h>
#include <assert.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/stream.h"
#include "../inc/sock.h"
#include "../inc/rpc_net.h"
#include "../inc/rlist.h"
#include "../inc/hashmap.h"
#include "../inc/rpc_fun.h"
#include "../inc/mempool.h"
#include "../inc/applog.h"
#include "../inc/threadpool.h"

//#include "internal_config.h"
#define RPCFUN_OVERLAP_MAXCOUNT		1000

//
typedef struct SEQ_ITEM {
	ATOM_SLIST_ENTRY			entry;
	RPCFUN_REQUEST_SEQ			seq;
	RPCFUN_ASYNC_OVERLAPPED*	overlap;
} SEQ_ITEM;
//

static RPCFUN_ASYNC_OVERLAPPED  NOCALLBACK_OVERLAPPED;
RPCFUN_ASYNC_OVERLAPPED* RPCFUN_NOCALLBACK = &NOCALLBACK_OVERLAPPED;

static os_mutex_t		response_mutex;
static os_mutex_t		seq_mutex;
static SEQ_ITEM				seq_list[RPCFUN_OVERLAP_MAXCOUNT];
static ATOM_SLIST_HEADER	seq_header;
static unsigned int			seq_genseq;
//
static os_mutex_t		func_mutex;
static HASHMAP_HANDLE		func_map;
//
static os_mutex_t		overlap_mutex;
//
static MEMPOOL_HANDLE		stream_pool;
static STREAM_INTERFACE		stream_interface;
//
static int rpcfun_append_function(RPCFUN_FUNCTION_DESC* desc);
static int rpcfun_remove_function(RPCFUN_FUNCTION_DESC* desc);
static RPCFUN_FUNCTION_DESC* function_get(const char* funcname);
static void function_free(RPCFUN_FUNCTION_DESC* desc);
//
static int requestseq_insert(RPCFUN_REQUEST_SEQ* seq, RPCFUN_ASYNC_OVERLAPPED* overlap);
static RPCFUN_ASYNC_OVERLAPPED* requestseq_remove(RPCFUN_REQUEST_SEQ* seq);
//
static void overlap_insert(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap);
static void overlap_remove(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap);
//
static void syncrpc_event(int etype, RPCNET_EVENTDATA* data);
static void asyncrpc_request_event(int etype, RPCNET_EVENTDATA* data);
static void asyncrpc_response_event(int etype, RPCNET_EVENTDATA* data);
static void asyncrpc_nocallback_event(int etype, RPCNET_EVENTDATA* data);
//
static void syncrpc_request(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* conn, STREAM* stream);
static void asyncrpc_request(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, STREAM* stream);
static void asyncrpc_initgroup(RPCNET_GROUP* group);
static void asyncrpc_response(RPCNET_GROUP* group, STREAM* stream);
static void asyncrpc_resetgroup(RPCNET_GROUP* group);
static void asyncrpc_nocallback(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, STREAM* stream);
//
static void localasync_proc(void* arg);

static os_mutex_t		stream_mutex;
static RLIST_HEAD			stream_list;

typedef struct RPCFUN_STREAM {
	MEM_STREAM					stream;
	RLIST_ITEM					item;
	RPCFUN_ASYNC_OVERLAPPED*	overlap;
} RPCFUN_STREAM;

static void stream_push(STREAM* stream, RPCFUN_ASYNC_OVERLAPPED* overlap)
{
	rlist_set_userdata(&((RPCFUN_STREAM*)stream)->item, stream);
	((RPCFUN_STREAM*)stream)->overlap = overlap;
	os_mutex_lock(&stream_mutex);
	rlist_push_back(&stream_list, &((RPCFUN_STREAM*)stream)->item);
	os_mutex_unlock(&stream_mutex);
}

static void stream_remove(STREAM* stream, RPCFUN_ASYNC_OVERLAPPED* overlap)
{
	RLIST_ITEM* item;
	os_mutex_lock(&stream_mutex);
	for(item=rlist_front(&stream_list); !rlist_is_head(&stream_list, item); item=rlist_next(item)) {
		if(rlist_get_userdata(item)==stream) {
			rlist_remove(&stream_list, item);
			os_mutex_unlock(&stream_mutex);
			return;			
		}
	}
	os_mutex_unlock(&stream_mutex);
	assert(0);
}

int rpcfun_init()
{
	int i;
	MEM_STREAM s;

	rlist_init(&stream_list);

	os_mutex_init(&response_mutex);
	os_mutex_init(&seq_mutex);
	os_mutex_init(&func_mutex);
	os_mutex_init(&overlap_mutex);

	memstream_init(&s, &i, sizeof(i), 0);
	memcpy(&stream_interface, s.i, sizeof(STREAM_INTERFACE));
	stream_interface.destroy = rpcfun_stream_free;
	stream_pool = mempool_create(sizeof(RPCFUN_STREAM)+RPCNET_PACKAGE_LENGTH, 0);
	if(stream_pool==NULL) return ERR_UNKNOWN;

	atom_slist_init(&seq_header);
	for(i=0; i<RPCFUN_OVERLAP_MAXCOUNT; i++) {
		seq_list[i].seq.index = i;
		seq_list[i].overlap = NULL;
		atom_slist_push(&seq_header, &seq_list[i].entry);
	}
	seq_genseq = 0;

	func_map = hashmap_create(6, NULL, 0);
	if(func_map==NULL) return ERR_UNKNOWN;
	rpcnet_register_subsys(RPCNET_SUBSYS_SYNCRPC, syncrpc_event);
	rpcnet_register_subsys(RPCNET_SUBSYS_ASYNCRPC_REQUEST, asyncrpc_request_event);
	rpcnet_register_subsys(RPCNET_SUBSYS_ASYNCRPC_RESPONSE, asyncrpc_response_event);
	rpcnet_register_subsys(RPCNET_SUBSYS_ASYNCRPC_NOCALLBACK, asyncrpc_nocallback_event);
	return ERR_NOERROR;
}

int rpcfun_final()
{
	mempool_destroy(stream_pool);
	hashmap_destroy(func_map);

	os_mutex_destroy(&response_mutex);
	os_mutex_destroy(&seq_mutex);
	os_mutex_destroy(&func_mutex);
	os_mutex_destroy(&overlap_mutex);

	return ERR_NOERROR;
}

int rpcfun_register(RPCFUN_FUNCTION_DESC* desc, int count)
{
	int i, ret = 0;
	if(count==0) {
		for(;;) {
			if(desc[count].funcname==NULL) break;
			if(desc[count].stub==NULL) break;
			if(desc[count].impl==NULL) break;
			count++;
		}
	}
	for(i=0; i<count; i++) {
		ret = rpcfun_append_function(desc+i);
		if(ret!=ERR_NOERROR) break;
		
	}
	if(ret!=ERR_NOERROR) {
		for(i--; i>=0; i--) {
			rpcfun_remove_function(desc+i);
		}
		return ret;
	} else {
		return ERR_NOERROR;
	}
}

void rpcfun_unregister(RPCFUN_FUNCTION_DESC* desc, int count)
{
	int i;
	if(count==0) {
		for(;;) {
			if(desc[count].funcname==NULL) break;
			if(desc[count].stub==NULL) break;
			if(desc[count].impl==NULL) break;
			count++;
		}
	}
	for(i=0; i<count; i++) {
		rpcfun_remove_function(desc+i);
	}
}

RPCFUN_FUNCTION_DESC* rpcfun_get_function(const char* funcname)
{
	RPCFUN_FUNCTION_DESC* desc;
	os_mutex_lock(&func_mutex);
	desc = (RPCFUN_FUNCTION_DESC*)hashmap_get(func_map, funcname, (unsigned int)strlen(funcname));
	if(desc!=NULL) atom_inc((unsigned int*)&desc->ref_count);
	os_mutex_unlock(&func_mutex);
	return desc;
}

void rpcfun_release_function(RPCFUN_FUNCTION_DESC* desc)
{
	atom_dec((unsigned int*)&desc->ref_count);
}

int rpcfun_write_head(STREAM* stream, const char* funcname, const RPCFUN_REQUEST_SEQ* seq)
{
	int ret, len;
	len = (int)strlen(funcname)+1;
	ret = stream_put(stream, &len, sizeof(unsigned char));
	if(ret!=ERR_NOERROR) return ret;
	ret = stream_put(stream, funcname, len);
	if(ret!=ERR_NOERROR) return ret;
	if(seq!=NULL) {
		ret = stream_put(stream, seq, sizeof(*seq));
		if(ret!=ERR_NOERROR) return ret;
	}
	return ERR_NOERROR;	
}

int rpcfun_read_head(STREAM* stream, char* funcname, RPCFUN_REQUEST_SEQ* seq)
{
	int ret, len;
	len = 0;
	ret = stream_get(stream, &len, sizeof(unsigned char));
	if(ret!=ERR_NOERROR) return ret;
	ret = stream_get(stream, funcname, len);
	if(ret!=ERR_NOERROR) return ret;
	if(seq!=NULL) {
		ret = stream_get(stream, seq, sizeof(*seq));
		if(ret!=ERR_NOERROR) return ret;
	}
	return ERR_NOERROR;
}

int rpcfun_write_result(STREAM* stream, const RPCFUN_REQUEST_SEQ* seq, const int retcode)
{
	int ret;
	if(seq!=NULL) {
		ret = stream_put(stream, seq, sizeof(*seq));
		if(ret!=ERR_NOERROR) return ret;
	}
	ret = stream_put(stream, &retcode, sizeof(retcode));
	if(ret!=ERR_NOERROR) return ret;
	return ERR_NOERROR;
}

int rpcfun_read_result(STREAM* stream, RPCFUN_REQUEST_SEQ* seq, int* retcode)
{
	int ret;
	if(seq!=NULL) {
		ret = stream_get(stream, seq, sizeof(*seq));
		if(ret!=ERR_NOERROR) return ret;
	}
	ret = stream_get(stream, retcode, sizeof(*retcode));
	if(ret!=ERR_NOERROR) return ret;
	return ERR_NOERROR;
}

int rpcfun_register_overlap(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap, RPCFUN_REQUEST_SEQ* seq)
{
	int ret;
	ret = requestseq_insert(seq, overlap);
	if(ret!=ERR_NOERROR) return ret;
	overlap_insert(group, overlap);
	return ERR_NOERROR;
}

RPCFUN_ASYNC_OVERLAPPED* rpcfun_unregister_overlap(RPCNET_GROUP* group, RPCFUN_REQUEST_SEQ* seq)
{
	RPCFUN_ASYNC_OVERLAPPED* overlap;
	overlap = requestseq_remove(seq);
	if(overlap!=NULL) overlap_remove(group, overlap);
	return overlap;
}

int rpcfun_serialize_write(STREAM* stream, RPCFUN_PARAMETER_ITEM* list, int count, RPCNET_THREAD_CONTEXT* ctx)
{
	int i, ret;
	for(i=0; i<count; i++) {
		switch(list[i].mode) {
		case 0x00: // value
			if(ctx==NULL) {
				ret = stream_put(stream, list[i].buf, list[i].size*list[i].count);
				if(ret!=ERR_NOERROR) return ret;
			}
			break;
        case 0|RPCFUN_SERIALIZE_IN: // in
			if(ctx==NULL) {
				ret = stream_put(stream, &list[i].count, sizeof(list[i].count));
				if(ret!=ERR_NOERROR) return ret;
				ret = stream_put(stream, list[i].buf, list[i].size*list[i].count);
				if(ret!=ERR_NOERROR) return ret;
			}
			break;
        case 0|RPCFUN_SERIALIZE_OUT: // out
			if(ctx==NULL) {
				ret = stream_put(stream, &list[i].max, sizeof(list[i].max));
				if(ret!=ERR_NOERROR) return ret;
			} else {
				ret = stream_put(stream, &list[i].count, sizeof(list[i].count));
				if(ret!=ERR_NOERROR) return ret;
				ret = stream_put(stream, list[i].buf, list[i].size*list[i].count);
				if(ret!=ERR_NOERROR) return ret;
			}
			break;
        case 0|RPCFUN_SERIALIZE_IN|RPCFUN_SERIALIZE_OUT: // in out
			if(ctx==NULL) {
				ret = stream_put(stream, &list[i].max, sizeof(list[i].max));
				if(ret!=ERR_NOERROR) return ret;
			}
			ret = stream_put(stream, &list[i].count, sizeof(list[i].count));
			if(ret!=ERR_NOERROR) return ret;
			ret = stream_put(stream, list[i].buf, list[i].size*list[i].count);
			if(ret!=ERR_NOERROR) return ret;
			break;
		case 0|RPCFUN_SERIALIZE_REF:
			if(ctx!=NULL) { assert(0); return ERR_UNKNOWN; }
			ret = stream_put(stream, &list[i].buf, sizeof(list[i].buf));
			if(ret!=ERR_NOERROR) return ret;
			break;
		default:
			return ERR_INVALID_DATA;
		}
	}
	return ERR_NOERROR;
}

int rpcfun_serialize_read(STREAM* stream, RPCFUN_PARAMETER_ITEM* list, int count, RPCNET_THREAD_CONTEXT* ctx)
{
	int i, ret;
	for(i=0; i<count; i++) {
        switch(list[i].mode) {
        case 0: // value
			if(ctx!=NULL) {
				ret = stream_get(stream, list[i].buf, list[i].size*list[i].count);
				if(ret!=ERR_NOERROR) return ret;
			}
            break;
        case 0|RPCFUN_SERIALIZE_IN: // in
			if(ctx!=NULL) {
				ret = stream_get(stream, &list[i].count, sizeof(list[i].count));
				if(ret!=ERR_NOERROR) return ret;
				list[i].buf = rpcnet_memory_alloc(ctx, list[i].size*list[i].count);
				if(list[i].buf==NULL) return ERR_NO_ENOUGH_MEMORY;
				ret = stream_get(stream, list[i].buf, list[i].size*list[i].count);
				if(ret!=ERR_NOERROR) return ret;
			}
			break;
        case 0|RPCFUN_SERIALIZE_OUT: // out
			if(ctx!=NULL) {
	            ret = stream_get(stream, &list[i].max, sizeof(list[i].max));
    	        if(ret!=ERR_NOERROR) return ret;
				list[i].buf = rpcnet_memory_alloc(ctx, list[i].size*list[i].max);
				if(list[i].buf==NULL) return ERR_NO_ENOUGH_MEMORY;
			} else {
				ret = stream_get(stream, &list[i].count, sizeof(list[i].count));
				if(ret!=ERR_NOERROR) return ret;
				ret = stream_get(stream, list[i].buf, list[i].size*list[i].count);
				if(ret!=ERR_NOERROR) return ret;
			}
            break;
        case 0|RPCFUN_SERIALIZE_IN|RPCFUN_SERIALIZE_OUT: // in out
			if(ctx!=NULL) {
				ret = stream_get(stream, &list[i].max, sizeof(list[i].max));
				if(ret!=ERR_NOERROR) return ret;
				list[i].buf = rpcnet_memory_alloc(ctx, list[i].size*list[i].max);
				if(list[i].buf==NULL) return ERR_NO_ENOUGH_MEMORY;
			}
			ret = stream_get(stream, &list[i].count, sizeof(list[i].count));
			if(ret!=ERR_NOERROR) return ret;
			ret = stream_get(stream, list[i].buf, list[i].size*list[i].count);
			if(ret!=ERR_NOERROR) return ret;
			break;
		case 0|RPCFUN_SERIALIZE_REF:
			assert(ctx!=NULL);
			ret = stream_get(stream, &list[i].buf, sizeof(list[i].buf));
			if(ret!=ERR_NOERROR) return ret;
			break;
        default:
            return ERR_INVALID_DATA;
        }
    }
	return ERR_NOERROR;
}

//
int rpcfun_append_function(RPCFUN_FUNCTION_DESC* desc)
{
	int ret;
	desc->ref_count = 0;
	os_mutex_lock(&func_mutex);
	ret = (hashmap_add(func_map, &desc->item, desc->funcname, (int)strlen(desc->funcname))==&desc->item?ERR_NOERROR:ERR_EXISTED);
	os_mutex_unlock(&func_mutex);
	return ret;
}

int rpcfun_remove_function(RPCFUN_FUNCTION_DESC* desc)
{
	os_mutex_lock(&func_mutex);
	hashmap_erase(func_map, &desc->item);
	os_mutex_unlock(&func_mutex);
	while(desc->ref_count>0);
	return ERR_NOERROR;
}

RPCFUN_FUNCTION_DESC* function_get(const char* funcname)
{
	RPCFUN_FUNCTION_DESC* desc;
	
	os_mutex_lock(&func_mutex);
	desc = (RPCFUN_FUNCTION_DESC*)hashmap_get(func_map, funcname, (int)strlen(funcname));
	if(desc!=NULL) atom_inc((unsigned int*)&desc->ref_count);
	os_mutex_unlock(&func_mutex);
	return desc;
}

void function_free(RPCFUN_FUNCTION_DESC* desc)
{
	atom_dec((unsigned int*)&desc->ref_count);
}

int requestseq_insert(RPCFUN_REQUEST_SEQ* seq, RPCFUN_ASYNC_OVERLAPPED* overlap)
{
	SEQ_ITEM* item;
	item = (SEQ_ITEM*)atom_slist_pop(&seq_header);
	if(item==NULL) return ERR_UNKNOWN;
	item->seq.seq = (unsigned short)atom_inc((unsigned int*)&seq_genseq);
	seq->index	= item->seq.index;
	seq->seq	= item->seq.seq;
	os_mutex_lock(&seq_mutex);
	item->overlap = overlap;
	os_mutex_unlock(&seq_mutex);
	return ERR_NOERROR;
}

RPCFUN_ASYNC_OVERLAPPED* requestseq_remove(RPCFUN_REQUEST_SEQ* seq)
{
	RPCFUN_ASYNC_OVERLAPPED* ret = NULL;
	if(seq->index<RPCFUN_OVERLAP_MAXCOUNT) {
		os_mutex_lock(&seq_mutex);
		if(seq_list[seq->index].seq.seq==seq->seq) {
			ret = seq_list[seq->index].overlap;
			seq_list[seq->index].overlap = NULL;
		}
		os_mutex_unlock(&seq_mutex);
		if(ret!=NULL)
			atom_slist_push(&seq_header, &seq_list[seq->index].entry);
	}
	return ret;
}

void overlap_insert(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap)
{
	os_mutex_lock(&overlap_mutex);
	overlap->_reserved.next = rpcnet_group_get_subsysdata(group, RPCNET_SUBSYS_ASYNCRPC_RESPONSE);
	overlap->_reserved.prev = NULL;
	rpcnet_group_set_subsysdata(group, RPCNET_SUBSYS_ASYNCRPC_RESPONSE, overlap);
	os_mutex_unlock(&overlap_mutex);
}

void overlap_remove(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap)
{
	os_mutex_lock(&overlap_mutex);
	if(overlap->_reserved.next!=NULL) overlap->_reserved.next->_reserved.prev = overlap->_reserved.prev;
	if(overlap->_reserved.prev!=NULL) overlap->_reserved.prev->_reserved.next = overlap->_reserved.next;
	if(overlap->_reserved.prev==NULL) rpcnet_group_set_subsysdata(group, RPCNET_SUBSYS_ASYNCRPC_RESPONSE, overlap->_reserved.next->_reserved.prev);
	os_mutex_unlock(&overlap_mutex);
}

void syncrpc_event(int etype, RPCNET_EVENTDATA* data)
{
	int base;

	switch(etype) {
	case RPCNET_EVENTTYPE_DATA:
		base = rpcnet_memory_getbase(data->DATA.context);
		syncrpc_request(data->DATA.context, data->DATA.group, data->DATA.conn, data->DATA.stream);
		rpcnet_memory_setbase(data->DATA.context, base);
		break;
	}
}

void asyncrpc_request_event(int etype, RPCNET_EVENTDATA* data)
{
	int base;

	switch(etype) {
	case RPCNET_EVENTTYPE_DATA:
		rpcnet_context_freeconn(data->DATA.context, data->DATA.conn);
		base = rpcnet_memory_getbase(data->DATA.context);
		asyncrpc_request(data->DATA.context, data->DATA.group, data->DATA.stream);
		rpcnet_memory_setbase(data->DATA.context, base);
		break;
	}
}

void asyncrpc_response_event(int etype, RPCNET_EVENTDATA* data)
{
	switch(etype) {
	case RPCNET_EVENTTYPE_DATA:
		rpcnet_context_freeconn(data->DATA.context, data->DATA.conn);
		asyncrpc_response(data->DATA.group, data->DATA.stream);
		break;
	case RPCNET_EVENTTYPE_RESET:
		asyncrpc_resetgroup(data->RESET.group);
		break;
	case RPCNET_EVENTTYPE_INIT:
		asyncrpc_initgroup(data->INIT.group);
		break;
	}
}

void asyncrpc_nocallback_event(int etype, RPCNET_EVENTDATA* data)
{
	int base;

	switch(etype) {
	case RPCNET_EVENTTYPE_DATA:
		rpcnet_context_freeconn(data->DATA.context, data->DATA.conn);
		base = rpcnet_memory_getbase(data->DATA.context);
		asyncrpc_nocallback(data->DATA.context, data->DATA.group, data->DATA.stream);
		rpcnet_memory_setbase(data->DATA.context, base);
		break;
	}
}
void syncrpc_request(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* conn, STREAM* stream)
{
	int ret;
	char funcname[RPCFUN_FUNCNAME_MAXLEN];
	RPCFUN_FUNCTION_DESC* desc;

	ret = rpcfun_read_head(stream, funcname, NULL);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "syncrpc_request(): rpcfun_read_head() failed at %d.\n", ret);
		rpcnet_context_closeconn(ctx, conn);
		return;
	}

	desc = function_get(funcname);
	if(desc==NULL) {
		stream = rpcnet_context_getstream(ctx);
		rpcfun_write_result(stream, NULL, ERR_NOT_IMPLEMENT);
		rpcnet_conn_write(conn, RPCNET_SUBSYS_SYNCRPC_RESPONSE, stream);
		return;
	}

	desc->stub(ctx, group, conn, NULL, stream);
	function_free(desc);
}

void asyncrpc_request(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, STREAM* stream)
{
	int ret;
	char funcname[RPCFUN_FUNCNAME_MAXLEN];
	RPCFUN_FUNCTION_DESC*	desc;
	RPCFUN_REQUEST_SEQ		seq;
	RPCNET_CONNECTION*	conn;

	ret = rpcfun_read_head(stream, funcname, &seq);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "syncrpc_request(): rpcfun_read_head() failed at %d.\n", ret);
		return;
	}
	
	desc = function_get(funcname);
    if(desc==NULL) {
		conn = rpcnet_context_getconn(ctx, group);
		if(conn==NULL) return; //
        stream = rpcnet_context_getstream(ctx);
        rpcfun_write_result(stream, NULL, ERR_NOT_IMPLEMENT);
        rpcnet_conn_write(conn, RPCNET_SUBSYS_SYNCRPC_RESPONSE, stream);
        return;
    }

	desc->stub(ctx, group, NULL, &seq, stream);
	function_free(desc);
}

void asyncrpc_initgroup(RPCNET_GROUP* group)
{
	rpcnet_group_set_subsysdata(group, RPCNET_SUBSYS_ASYNCRPC_RESPONSE, NULL);
}

void asyncrpc_response(RPCNET_GROUP* group, STREAM* stream)
{
	int ret, retcode;
	RPCFUN_REQUEST_SEQ			seq;
	RPCFUN_ASYNC_OVERLAPPED*	overlap;

	// get seq&retcode
	ret = rpcfun_read_result(stream, &seq, &retcode);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "asyncrpc_reponse(): failed to rpcfun_read_result, return %d\n", ret);
		return;
	}
	// get overlap
	overlap = requestseq_remove(&seq);
	if(overlap==NULL) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "asyncrpc_response(): overlap_get() return NULL, seq(%d,%d)\n", seq.index, seq.seq);
		return;
	}
	// restore parameter
	if(retcode==ERR_NOERROR) {
		ret = rpcfun_serialize_read(stream, overlap->_reserved.list, overlap->_reserved.list_count, NULL);
		if(ret!=ERR_NOERROR) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "asyncrpc_response(): failedd to rpcfun_serialize_read(), return %d.\n", ret);
			retcode = ret;
		}
	}

	// call event
	overlap->callback(retcode, overlap);
}

void asyncrpc_resetgroup(RPCNET_GROUP* group)
{
	RPCFUN_ASYNC_OVERLAPPED* overlap;
	RPCFUN_ASYNC_OVERLAPPED* overlap_list;;

	os_mutex_lock(&response_mutex);
	// set
	overlap_list = (RPCFUN_ASYNC_OVERLAPPED*)rpcnet_group_get_subsysdata(group, RPCNET_SUBSYS_ASYNCRPC_RESPONSE);
	// remove seq
	while(overlap_list!=NULL) {
		overlap = overlap_list;
		overlap_list = overlap->_reserved.next;
		requestseq_remove(&overlap->_reserved.seq);
	}
	//
	overlap_list = (RPCFUN_ASYNC_OVERLAPPED*)rpcnet_group_get_subsysdata(group, RPCNET_SUBSYS_ASYNCRPC_RESPONSE);
	rpcnet_group_set_subsysdata(group, RPCNET_SUBSYS_ASYNCRPC_RESPONSE, NULL);
	os_mutex_unlock(&response_mutex);
	//
	while(overlap_list!=NULL) {
		overlap = overlap_list;
		overlap_list = overlap->_reserved.next;
		overlap->callback(ERR_TIME_OUT, overlap);
	}
}

void asyncrpc_nocallback(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, STREAM* stream)
{
	int ret;
	char funcname[RPCFUN_FUNCNAME_MAXLEN];
	RPCFUN_FUNCTION_DESC*	desc;

	ret = rpcfun_read_head(stream, funcname, NULL);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "syncrpc_request(): rpcfun_read_head() failed at %d.\n", ret);
		return;
	}
	
	desc = function_get(funcname);
	if(desc==NULL) return;
	desc->stub(ctx, group, NULL, NULL, stream);
	function_free(desc);
}

void localasync_proc(void* arg)
{
	STREAM*						stream;
	RPCFUN_ASYNC_OVERLAPPED*	overlap;
	RPCFUN_FUNCTION_DESC*		desc;
	RPCNET_THREAD_CONTEXT*	ctx;
	int base;
	int ret;
	
	stream	= NULL;
	overlap	= NULL;
//	stream	= (STREAM*)arg1;
//	overlap	= (RPCFUN_ASYNC_OVERLAPPED*)arg2;

	// get desc
	ret = stream_get(stream, &desc, sizeof(desc));
	if(ret!=ERR_NOERROR) goto ON_CALLBACK;
	// get thread context
	ctx = rpcnet_context_get();
	if(ctx==NULL) goto ON_CALLBACK;
	// get memory base address
	base = rpcnet_memory_getbase(ctx);
	// call_stub();.....
	ret = desc->stub(ctx, rpcnet_getgroup(NULL), NULL, NULL, stream);
	if(ret!=ERR_NOERROR) {
		DBGLOG(LOG_ERROR, MODULE_NAME, "desc(%s)->stub , errno=%d", desc->funcname, ret);
	}
	// set memory base address
	rpcnet_memory_setbase(ctx, base);
	// release
	rpcfun_release_function(desc);
	// release thread context
	rpcnet_context_free(ctx);

ON_CALLBACK:
	stream_remove(stream, overlap);
	stream_destroy(stream);
	if(overlap!=RPCFUN_NOCALLBACK) overlap->callback(ret, overlap);
}

int rpcfun_localasync_post(STREAM* stream, RPCFUN_ASYNC_OVERLAPPED* overlap)
{
	stream_push(stream, overlap);
	return threadpool_queueitem(localasync_proc, stream);
}

STREAM* rpcfun_stream_alloc()
{
	MEM_STREAM* ret;
	ret = (MEM_STREAM*)mempool_alloc(stream_pool);
	if(ret==NULL) return NULL;
	memstream_init(ret, (char*)ret+sizeof(RPCFUN_STREAM), RPCNET_PACKAGE_LENGTH, 0);
	ret->i = &stream_interface;
	return((STREAM*)ret);
}

void rpcfun_stream_free(STREAM* stream)
{
	mempool_free(stream_pool, stream);
}

int rpcfun_stream_isvalid(STREAM* stream)
{
	return(stream->i==&stream_interface?ERR_NOERROR:ERR_UNKNOWN);
}

