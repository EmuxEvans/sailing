#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/stream.h"
#include "../../inc/skates/sock.h"
#include "../../inc/skates/rpc_net.h"
#include "../../inc/skates/mempool.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/fdwatch.h"
#include "../../inc/skates/threadpool.h"
#include "../../inc/skates/applog.h"

#define RPCNET_SUBSYS_NONE					255

#define RPCCONN_TYPE_INCOMING	1
#define RPCCONN_TYPE_OUTGOING	2
#define RPCCONN_TYPE_LISTENER	3

#define RPCCONN_STATE_AVAILABLE	1
#define RPCCONN_STATE_SHUTDOWN	2
#define RPCCONN_STATE_INUSE		2
#define RPCCONN_STATE_CLOSED	3

typedef struct RPCNET_STREAM {
	STREAM_INTERFACE* i;
	RPCNET_THREAD_CONTEXT* ctx;
	unsigned int cur;
} RPCNET_STREAM;

struct RPCNET_CONNECTION {
	FDWATCH_ITEM	item_fdw;
	RLIST_ITEM		item_grp;
	RLIST_ITEM		item_ctx;

	int fd;
	int type;
	int st_flag;

	RPCNET_GROUP*			group;
	union {
		RPCNET_THREAD_CONTEXT*	ctx;
		void* ctx_void;
	};
};

struct RPCNET_GROUP {
	SOCK_ADDR		endpoint;
	RLIST_HEAD		in_conns_list;
	RLIST_HEAD		out_conns_list;
	
	void*	subsys_data[RPCNET_SUBSYS_MAXCOUNT];
};

struct RPCNET_THREAD_CONTEXT {
	int ref_count;
	RLIST_HEAD hold_conns;

	unsigned char mem_block[RPCNET_MEMORYBLOCK_LENGTH];
	int mem_cur;

	struct {
		unsigned int len;
		unsigned int subsys;
		unsigned char body[RPCNET_PACKAGE_LENGTH];
	} pkg;

	RPCNET_STREAM stream;
};

// rpcnet_stream
static void rpcnet_stream_clear(RPCNET_STREAM* stream);
static void rpcnet_stream_destroy(RPCNET_STREAM* stream);
static int rpcnet_stream_skip(RPCNET_STREAM* stream, unsigned int len);
static int rpcnet_stream_seek(RPCNET_STREAM* stream, unsigned int loc);
static int rpcnet_stream_put(RPCNET_STREAM* stream, void* buf, unsigned int len);
static int rpcnet_stream_get(RPCNET_STREAM* stream, void* buf, unsigned int len);

static STREAM_INTERFACE rpcnet_stream = 
STREAM_INTERFACE_DEFINE(
rpcnet_stream_clear,
rpcnet_stream_destroy,
rpcnet_stream_skip,
rpcnet_stream_seek,
rpcnet_stream_put,
rpcnet_stream_get);

// connection pool&count
static MEMPOOL_HANDLE conn_pool = NULL;
static int conn_incoming_count = 0;
static int conn_outgoing_count = 0;
// local listener & group
static RPCNET_CONNECTION listener;
static RPCNET_GROUP* local_group = NULL;
static RPCNET_GROUP local_group_i;
// connction operator
static RPCNET_CONNECTION* conn_alloc(int fd, int type);
static void conn_free(RPCNET_CONNECTION* conn);
static void conn_shutdown(RPCNET_CONNECTION* conn);
// group pool
static MEMPOOL_HANDLE group_pool = NULL;
static os_mutex_t group_mutex;
static RPCNET_GROUP* group_map[RPCNEWTORK_EPINDEX_MAXVALUE];
//static os_mutex_t group_notify_mutex;
// group operator
static int is_local_endpoint(const SOCK_ADDR* endpoint);					// thread safe
static RPCNET_GROUP* group_get(const SOCK_ADDR* endpoint);
static void group_free(RPCNET_GROUP* group);
static void group_addconn(RPCNET_GROUP* group, RPCNET_CONNECTION* conn); // thread safe
static void group_delconn(RPCNET_CONNECTION* conn); // thread safe
static void group_notify(RPCNET_GROUP* group, int type);
static RPCNET_CONNECTION* group_get_outgoing(RPCNET_GROUP* group, RPCNET_THREAD_CONTEXT* ctx);		// thread safe
// thread context
static int threads_ctx_count;
static RPCNET_THREAD_CONTEXT* threads_ctx[THREADPOOL_MAX_WORKERTHREAD];
// thread context operator
static void context_addconn(RPCNET_THREAD_CONTEXT* ctx, RPCNET_CONNECTION* conn);
static void context_delconn(RPCNET_CONNECTION* conn);
// epoll fd
static FDWATCH_HANDLE epoll_fd = NULL;
static void accept_callback(FDWATCH_ITEM* item, int events);
static void client_callback(FDWATCH_ITEM* item, int events);

// epoll ionthread
static unsigned int ZION_CALLBACK ionthread_proc(void* param);
static int rpcnet_quitflag; 		// nonzero = quit
static os_thread_t ionthread_tid;
// threadpool callback
static void listener_threadevent(void* ptr);
static void connection_threadevent(void* ptr);
// subsys
static RPCNET_EVENT subsys_events[RPCNET_SUBSYS_MAXCOUNT];
// ep2idx
struct EP2IDX_ITEM;
typedef struct EP2IDX_ITEM EP2IDX_ITEM;
struct EP2IDX_ITEM {
	int					index;
	EP2IDX_ITEM*		next;
	SOCK_ADDR			ep;
};
static os_mutex_t	ep2idx_mutex;
static int				ep2idx_count;
static EP2IDX_ITEM		ep2idx_array[RPCNEWTORK_EPINDEX_MAXVALUE];
static EP2IDX_ITEM*		ep2idx_map[65536];

int rpcnet_init()
{
	int ret;
	threads_ctx_count = 0;
	ep2idx_count = 0;

	// data init
	memset(subsys_events, 0, sizeof(subsys_events));
	memset(threads_ctx, 0, sizeof(threads_ctx));
	memset(ep2idx_map, 0, sizeof(ep2idx_map));
	memset(group_map, 0, sizeof(group_map));
	// init mutexs
	os_mutex_init(&group_mutex);
	os_mutex_init(&ep2idx_mutex);
	// create pools, how many items to be inited
	conn_pool	= mempool_create("RPCNET_CONN", sizeof(RPCNET_CONNECTION), 0);
	group_pool	= mempool_create("RPCNET_GROUP", sizeof(RPCNET_GROUP), 0);
	if(conn_pool==NULL || group_pool==NULL) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_initialize: respool_create() failed.\n");
		if(conn_pool!=NULL)		{ mempool_destroy(conn_pool); conn_pool = NULL; }
		if(group_pool!=NULL)	{ mempool_destroy(group_pool); group_pool = NULL; }
		os_mutex_destroy(&group_mutex);
		return ERR_UNKNOWN;
	}
	// create epoll fd
	epoll_fd = fdwatch_create();
	if(epoll_fd==NULL) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_initialize: epoll_create().\n");
		mempool_destroy(conn_pool); conn_pool = NULL;
		mempool_destroy(group_pool); group_pool = NULL;
		os_mutex_destroy(&group_mutex);
		return ERR_UNKNOWN;
	}
	// create epoll ionthread
	rpcnet_quitflag = 0;
	ret = os_thread_begin(&ionthread_tid, ionthread_proc, NULL);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_initialize: pthread_create() return %d.\n", errno);
		mempool_destroy(conn_pool); conn_pool = NULL;
		mempool_destroy(group_pool); group_pool = NULL;
		os_mutex_destroy(&group_mutex);
		fdwatch_destroy(epoll_fd); epoll_fd = NULL;
		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

int rpcnet_final()
{
	int i;

	// free all rpcnetwork thread contexts
	for(i=0; i<THREADPOOL_MAX_WORKERTHREAD; i++) {
		if(threads_ctx[i]!=NULL) {
			free(threads_ctx[i]);
			threads_ctx[i] = NULL;
		}
	}
	// clear data;
	mempool_destroy(conn_pool); conn_pool = NULL;
	mempool_destroy(group_pool); group_pool = NULL;
	os_mutex_destroy(&ep2idx_mutex);
	os_mutex_destroy(&group_mutex);

	return ERR_NOERROR;
}

int rpcnet_shutdown()
{
	RLIST_ITEM* item;
	RPCNET_CONNECTION* conn;
	RPCNET_GROUP* group;
	int l;

	// close all incoming connections
	while(conn_incoming_count>0) {
		os_mutex_lock(&group_mutex);
		for(l=0; l<RPCNEWTORK_EPINDEX_MAXVALUE; l++) {
			group = group_map[l];
			if(group==NULL) continue;

			for(item=rlist_front(&group->in_conns_list); !rlist_is_head(&group->in_conns_list, item); item=rlist_next(item)) {
				conn = (RPCNET_CONNECTION*)rlist_get_userdata(item);
				conn_shutdown(conn);
				if(conn->ctx!=NULL) {
					group_delconn(conn);
					break;
				} else {
					continue;
				}
			}
		}
		os_mutex_unlock(&group_mutex);
	}

	// send quit_message to epoll-ionthread
	rpcnet_quitflag = 1;
	os_thread_wait(ionthread_tid, NULL);
	// close epoll fds
	fdwatch_destroy(epoll_fd);
	epoll_fd = NULL;

	do {
		// close all outgoing connections & free all groups
		os_mutex_lock(&group_mutex);
		for(l=0; l<RPCNEWTORK_EPINDEX_MAXVALUE; l++) {
			group = group_map[l];
			if(group==NULL) continue;

			while(!rlist_empty(&group->out_conns_list)) {
				conn = (RPCNET_CONNECTION*)rlist_get_userdata(rlist_front(&group->out_conns_list));
				if(conn->ctx!=NULL) {
					conn_shutdown(conn);
					group_delconn(conn);
				} else {
					conn_shutdown(conn);
					group_delconn(conn);
					conn_free(conn);
				}
			}
			
			group_free(group_map[l]);
			group_map[l] = NULL;
		}
		os_mutex_unlock(&group_mutex);
	} while(conn_outgoing_count>0);

	return ERR_NOERROR;
}

int rpcnet_bind(SOCK_ADDR* endpoint)
{
	SOCK_HANDLE sock;
	int ret;

	if(threadpool_getcount()<=0) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_init: threadpool_getcount()<=0");
		return ERR_UNKNOWN;
	}

	// bind tcp port
	sock = sock_bind(endpoint, SOCK_REUSEADDR);
	if(sock==SOCK_INVALID_HANDLE) return ERR_UNKNOWN;

	memcpy(&local_group_i.endpoint, endpoint, sizeof(SOCK_ADDR));
	local_group = &local_group_i;

	// add listener_fd to epoll_fd
	listener.fd = sock;
	listener.type = RPCCONN_TYPE_LISTENER;
	listener.group = NULL;
	listener.st_flag = RPCCONN_STATE_AVAILABLE;

	fdwatch_set(&listener.item_fdw, sock, FDWATCH_READ|FDWATCH_ONCE, accept_callback, &listener);
	ret = fdwatch_add(epoll_fd, &listener.item_fdw);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_bind : epoll_ctl() failed at %d.\n", ret);
		sock_unbind(sock);
		local_group = NULL;
		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

int rpcnet_unbind()
{
	local_group = NULL;
	// del listener_fd from epoll_fd
	listener.st_flag = RPCCONN_STATE_SHUTDOWN;
	// wait listener
	while(listener.st_flag!=RPCCONN_STATE_CLOSED) {
		os_sleep(10);
	}
	SYSLOG(LOG_INFO, MODULE_NAME, "rpcnet_unbind");
	sock_unbind(listener.fd);
	listener.fd = SOCK_INVALID_HANDLE;
	return ERR_NOERROR;
}

const SOCK_ADDR* rpcnet_get_bindep()
{
	return local_group?&local_group->endpoint:NULL;
}

void accept_callback(FDWATCH_ITEM* item, int events)
{
	threadpool_queueitem(listener_threadevent, item);
}

void client_callback(FDWATCH_ITEM* item, int events)
{
	threadpool_queueitem(connection_threadevent, item);
}

unsigned int ZION_CALLBACK ionthread_proc(void* param)
{
	int ret;

	while(0==rpcnet_quitflag) {
		if(listener.st_flag==RPCCONN_STATE_SHUTDOWN) {
			ret = fdwatch_remove(epoll_fd, &listener.item_fdw);
			if(ret!=ERR_NOERROR) {
				SYSLOG(LOG_ERROR, MODULE_NAME, "ionthread_proc(): epoll_ctl(listener.item_fdw) failed at %d.\n", ret);
			}
			listener.st_flag = RPCCONN_STATE_CLOSED;
		}

		ret = fdwatch_dispatch(epoll_fd, 10);
		if(ret!=ERR_NOERROR) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "ionthread_proc(): epoll_ctl(epoll_fd) failed at %d.\n", ret);
		}
	}

	return 0;
}

void listener_threadevent(void* ptr)
{
	SOCK_HANDLE sub_sock;
	RPCNET_GROUP *group;
	RPCNET_CONNECTION *conn;
	SOCK_ADDR na;

	for(;;) {
		if(sock_wait_read(listener.fd, 0)!=ERR_NOERROR) {
			break;
		}

		sub_sock = sock_accept(listener.fd, &na);
		if(sub_sock==SOCK_INVALID_HANDLE) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "listener_threadevent(): accept() failed at %d.\n", errno);	// error occured
			break;
		}
		if(sock_readbuf(sub_sock, &na, sizeof(na))!=0) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "listener_threadevent : fail to receive client endpoint.\n");
			sock_close(sub_sock);
			continue;
		}
		// get group
		group = rpcnet_getgroup(&na);
		if(group==NULL) {
			sock_close(sub_sock);
			continue;
		}
		assert(na.ip==rpcnet_group_get_endpoint(group, NULL)->ip);
		assert(na.port==rpcnet_group_get_endpoint(group, NULL)->port);

		// alloc conn
		conn = conn_alloc(sub_sock, RPCCONN_TYPE_INCOMING);
		if(NULL==conn) {
			sock_close(sub_sock);
			continue;
		}
		// add conn to group
		os_mutex_lock(&group_mutex);
		group_addconn(group, conn);
		os_mutex_unlock(&group_mutex);
		// add conn to epoll
		fdwatch_set(&conn->item_fdw, sub_sock, FDWATCH_READ|FDWATCH_ONCE, client_callback, conn);
		if(fdwatch_add(epoll_fd, &conn->item_fdw)!=ERR_NOERROR) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "listener_threadevent(): epoll_ctl() failed at %d.\n", errno);
			os_mutex_unlock(&group_mutex);
			group_delconn(conn);
			os_mutex_unlock(&group_mutex);
			conn_free(conn);
			continue;
		}
	}

	if(fdwatch_rearm(epoll_fd, &listener.item_fdw)!=0) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "listener_threadevent(): epoll_ctl() rearm listener failed at %d.", errno);
	}
}

void connection_threadevent(void* ptr)
{
	RPCNET_CONNECTION *conn;
	RPCNET_THREAD_CONTEXT *ctx;
	RPCNET_EVENTDATA ed;

	conn = (RPCNET_CONNECTION *)ptr;
	// get thread context
	if(NULL==(ctx=rpcnet_context_get())) return;
	//
	context_addconn(ctx, conn);
	// recv data to ctx
	if(ERR_NOERROR==rpcnet_conn_read(conn->group, conn, RPCNET_SUBSYS_NONE, (STREAM*)((void*)(&ctx->stream)))) {
		// call subsys event
		ed.DATA.context	= ctx;
		ed.DATA.group	= conn->group;
		ed.DATA.conn	= conn;
		ed.DATA.stream	= (STREAM*)(void*)&ctx->stream;
		ctx->stream.cur	= 0;
		// call
		subsys_events[ctx->pkg.subsys](RPCNET_EVENTTYPE_DATA, &ed);
	} else {
		// if read_failed then
		//     rearm fd
		//     release conn in epoll thread
	}
	// release thread context
	rpcnet_context_free(ctx);
}

RPCNET_GROUP* rpcnet_getgroup(const SOCK_ADDR* endpoint)
{
	RPCNET_GROUP *group;

	if(endpoint==NULL || is_local_endpoint(endpoint)) return local_group;

	os_mutex_lock(&group_mutex);
	group = group_get(endpoint);
	os_mutex_unlock(&group_mutex);

	return group;
}

int rpcnet_register_subsys(unsigned char id, RPCNET_EVENT event)
{
	subsys_events[id] = event;
	return ERR_NOERROR;
}

int rpcnet_ep2idx(const SOCK_ADDR* ep)
{
	int idx;
	unsigned int hashcode;
	EP2IDX_ITEM* item;
	hashcode = (ep->ip>>16)&0xffff;
	os_mutex_lock(&ep2idx_mutex);
	for(item=ep2idx_map[hashcode]; item!=NULL; item=item->next) {
		if(memcmp(&item->ep, ep, sizeof(SOCK_ADDR))==0) break;
	}
	if(item==NULL) {
		if(ep2idx_count<RPCNEWTORK_EPINDEX_MAXVALUE) {
			ep2idx_array[ep2idx_count].index = ep2idx_count;
			ep2idx_array[ep2idx_count].next = ep2idx_map[hashcode];
			memcpy(&ep2idx_array[ep2idx_count].ep, ep, sizeof(SOCK_ADDR));
			ep2idx_map[hashcode] = &ep2idx_array[ep2idx_count];
			idx = ep2idx_count++;
		} else {
			idx = -1;
		}
	} else {
		idx = item->index;
	}
	os_mutex_unlock(&ep2idx_mutex);
	return idx;
}

int rpcnet_group_islocal(RPCNET_GROUP *group)
{
	return group==local_group?1:0;
}

const SOCK_ADDR* rpcnet_group_get_endpoint(RPCNET_GROUP* group, SOCK_ADDR* addr)
{
	if(addr==NULL) {
		return &group->endpoint;
	} else {
		memcpy(addr, &group->endpoint, sizeof(SOCK_ADDR));
		return addr;
	}	
}

void* rpcnet_group_get_subsysdata(RPCNET_GROUP* group, unsigned char id)
{
	return group->subsys_data[(int)id];
}

void rpcnet_group_set_subsysdata(RPCNET_GROUP* group, unsigned char id, void* data)
{
	group->subsys_data[(int)id] = data;
}

RPCNET_THREAD_CONTEXT* rpcnet_context_get()
{
	int index;

	index = threadpool_getindex();
	if(index<0) return NULL;
	if(threads_ctx[index]==NULL) {
		threads_ctx[index] = (RPCNET_THREAD_CONTEXT*)malloc(sizeof(RPCNET_THREAD_CONTEXT));
		if(threads_ctx[index]==NULL)  return NULL;
		threads_ctx[index]->ref_count = 1;
		rlist_init(&threads_ctx[index]->hold_conns);
		threads_ctx[index]->mem_cur = 0;
		threads_ctx[index]->stream.i = &rpcnet_stream;
		threads_ctx[index]->stream.ctx = threads_ctx[index];
	} else {
		threads_ctx[index]->ref_count++;
	}
	if(threads_ctx[index]->ref_count==1) atom_inc((unsigned int*)&threads_ctx_count);
	return threads_ctx[index];
}

void rpcnet_context_free(RPCNET_THREAD_CONTEXT* ctx)
{
	ctx->ref_count--;
	if(ctx->ref_count!=0) return;

	atom_dec((unsigned int*)&threads_ctx_count);

	// detach all connections
	while(!rlist_empty(&ctx->hold_conns)) {
		rpcnet_context_freeconn(ctx, (RPCNET_CONNECTION*)rlist_get_userdata(rlist_front(&ctx->hold_conns)));
	}
}

RPCNET_CONNECTION* rpcnet_context_getconn(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP *group)
{
	RLIST_ITEM* item;
	RPCNET_CONNECTION *conn;
	SOCK_HANDLE sock;
	SOCK_ADDR endp;

	if(rpcnet_quitflag!=0) return NULL;

	// find exists conn from ctx
	for(item=rlist_front(&ctx->hold_conns); !rlist_is_head(&ctx->hold_conns, item); item=rlist_next(item)) {
		conn = (RPCNET_CONNECTION*)rlist_get_userdata(item);
		if(conn->group==group) return conn;
	}
	
	// find idle out_conn from group
	conn = group_get_outgoing(group, ctx);
	if(conn!=NULL) return conn;

	// create a new conn
	sock = sock_connect(&group->endpoint, 0);
	if(sock==SOCK_INVALID_HANDLE) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "failed to rpcnet_context_getconn(): socket(), errno=%d.\n", errno);
		return NULL;
	}
	// send local ep
	if(sock_sockname(sock, &endp)!=0) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_context_getconn : fail to getsockname(), ret = %d\n", errno);
		sock_close(sock); 
		return NULL; 
	}
	assert(endp.ip!=0);
	endp.port = local_group?local_group->endpoint.port:0;
	if(sock_writebuf(sock, &endp, sizeof(endp))!=0) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_context_getconn : fail to getsockname(), ret = %d\n", errno);
		sock_close(sock); 
		return NULL; 
	}
	assert(endp.ip!=0);
	// alloc conn
	conn = conn_alloc(sock, RPCCONN_TYPE_OUTGOING);
	if(conn==NULL) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_context_getconn : fail to getsockname(), ret = %d\n", errno);
		sock_close(sock);
		return NULL;
	}
	// add conn to thread context
	context_addconn(ctx, conn);
	// add conn to group
	os_mutex_lock(&group_mutex);
	group_addconn(group, conn);
	os_mutex_unlock(&group_mutex);

	return conn;
}

RPCNET_CONNECTION* rpcnet_context_getconn_force(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP *group)
{
	RPCNET_CONNECTION *conn;
	SOCK_HANDLE sock;
	SOCK_ADDR endp;

	if(rpcnet_quitflag!=0) return NULL;

	// find idle out_conn from group
	conn = group_get_outgoing(group, ctx);
	if(conn!=NULL) return conn;

	// create a new conn
	sock = sock_connect(&group->endpoint, 0);
	if(sock==SOCK_INVALID_HANDLE) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "failed to rpcnet_context_getconn(): socket(), errno=%d.\n", errno);
		return NULL;
	}
	// send local ep
	if(sock_sockname(sock, &endp)!=0) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_context_getconn : fail to getsockname(), ret = %d\n", errno);
		sock_close(sock); 
		return NULL; 
	}
	assert(endp.ip!=0);
	endp.port = local_group?local_group->endpoint.port:0;
	if(sock_writebuf(sock, &endp, sizeof(endp))!=0) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_context_getconn : fail to getsockname(), ret = %d\n", errno);
		sock_close(sock); 
		return NULL; 
	}
	assert(endp.ip!=0);
	// alloc conn
	conn = conn_alloc(sock, RPCCONN_TYPE_OUTGOING);
	if(conn==NULL) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_context_getconn : fail to getsockname(), ret = %d\n", errno);
		sock_close(sock);
		return NULL;
	}
	// add conn to thread context
	context_addconn(ctx, conn);
	// add conn to group
	os_mutex_lock(&group_mutex);
	group_addconn(group, conn);
	os_mutex_unlock(&group_mutex);

	return conn;
}

int rpcnet_context_chkconn(RPCNET_THREAD_CONTEXT* ctx, RPCNET_CONNECTION* conn)
{
	RLIST_ITEM* item;
	for(item=rlist_front(&ctx->hold_conns); !rlist_is_head(&ctx->hold_conns, item); item=rlist_next(item)) {
		if(conn==(RPCNET_CONNECTION*)rlist_get_userdata(item)) return 1;
	}
	return 0;
}

int rpcnet_context_freeconn(RPCNET_THREAD_CONTEXT* ctx, RPCNET_CONNECTION* conn)
{
	if(conn->type==RPCCONN_TYPE_OUTGOING) {
		// remove connection from thread_context
		os_mutex_lock(&group_mutex);
		context_delconn(conn);
		os_mutex_unlock(&group_mutex);
	} else {
		if(conn->st_flag==RPCCONN_STATE_AVAILABLE) {
			os_mutex_lock(&group_mutex);
			// remove connection
			context_delconn(conn);
			if(conn->group==NULL) conn = NULL;
			os_mutex_unlock(&group_mutex);

			if(conn!=NULL) {
				// rearm incoming connection
				if(fdwatch_rearm(epoll_fd, &conn->item_fdw)!=ERR_NOERROR) {
					SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_context_freeconn(): epoll_ctl() rearm incoming connection failed at %d.\n", errno);
					os_mutex_lock(&group_mutex);
					group_delconn(conn);
					os_mutex_unlock(&group_mutex);
					conn_free(conn);
				}
			}
		} else {
			os_mutex_lock(&group_mutex);
			group_delconn(conn);
			context_delconn(conn);
			os_mutex_unlock(&group_mutex);
		}
	}

	return ERR_NOERROR;
}

void rpcnet_context_closeconn(RPCNET_THREAD_CONTEXT* ctx, RPCNET_CONNECTION* conn)
{
	// shutdown
	conn_shutdown(conn);
	os_mutex_lock(&group_mutex);
	group_delconn(conn);
	context_delconn(conn);
	os_mutex_unlock(&group_mutex);
}

STREAM* rpcnet_context_getstream(RPCNET_THREAD_CONTEXT* context)
{
	context->pkg.len = 0;
	context->stream.cur = 0;
	return (STREAM*)((void*)&context->stream);
}

int rpcnet_conn_write(RPCNET_CONNECTION* conn, unsigned char subsys, STREAM* stream)
{
	RPCNET_THREAD_CONTEXT* ctx;
	int ret;
	//
	ctx = ((RPCNET_STREAM*)stream)->ctx;
	ctx->pkg.subsys = subsys;

	// send head & body
	ret = sock_writebuf(conn->fd, &ctx->pkg, sizeof(ctx->pkg.len)+sizeof(ctx->pkg.subsys)+ctx->pkg.len);
	if(ret!=0) goto ON_ERROR;
	// done
	return ERR_NOERROR;
	
ON_ERROR:
	if(conn->type==RPCCONN_TYPE_INCOMING) {
		if(fdwatch_remove(epoll_fd, &conn->item_fdw)!=0) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_conn_write(): epoll_ctl() failed at %d.\n", errno);
		}
	}
	conn_shutdown(conn);
	os_mutex_lock(&group_mutex);
	group_delconn(conn);
	os_mutex_unlock(&group_mutex);
	return ret;
}

int rpcnet_conn_read(RPCNET_GROUP* group, RPCNET_CONNECTION* conn, unsigned char subsys, STREAM* stream)
{
	RPCNET_THREAD_CONTEXT* ctx;
	RPCNET_EVENTDATA ed;
	int ret;

	//
	ctx = ((RPCNET_STREAM*)stream)->ctx;

ON_START:
	//start

	// read head
	ret = sock_readbuf(conn->fd, &ctx->pkg, sizeof(ctx->pkg.len)+sizeof(ctx->pkg.subsys));
	if(ret!=ERR_NOERROR) goto ON_ERROR;

	// read body
	ret = sock_readbuf(conn->fd, &ctx->pkg.body, ctx->pkg.len);
	if(ret!=0) goto ON_ERROR;
	// done
	ctx->stream.cur	= 0;

	if(subsys!=RPCNET_SUBSYS_NONE && ctx->pkg.subsys!=subsys) {
		assert(ctx->pkg.subsys<sizeof(subsys_events)/sizeof(subsys_events[0]));

		// call subsys event
		ed.DATA.context	= ctx;
		ed.DATA.group	= group;
		ed.DATA.conn	= conn;
		ed.DATA.stream	= (STREAM*)(void*)&ctx->stream;
		// call
		subsys_events[ctx->pkg.subsys](RPCNET_EVENTTYPE_DATA, &ed);
		goto ON_START;
	}

	return ERR_NOERROR;

ON_ERROR:
	if(conn->type==RPCCONN_TYPE_INCOMING) {
		if(fdwatch_remove(epoll_fd, &conn->item_fdw)!=0) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_conn_write(): epoll_ctl() failed at %d.\n", errno);
		}
	}
	conn_shutdown(conn);
	os_mutex_lock(&group_mutex);
	group_delconn(conn);
	os_mutex_unlock(&group_mutex);
	return ret;
}

void* rpcnet_memory_alloc(RPCNET_THREAD_CONTEXT* context, int size)
{
	void* ret;
	if(context->mem_cur+size>RPCNET_MEMORYBLOCK_LENGTH) return NULL;
	ret = context->mem_block + context->mem_cur;
	context->mem_cur += size;
	return(ret);	
}

int rpcnet_memory_getbase(RPCNET_THREAD_CONTEXT* context)
{
	return(context->mem_cur);
}

void rpcnet_memory_setbase(RPCNET_THREAD_CONTEXT* context, int base)
{
	context->mem_cur = base;
}

// Internal functions
int is_local_endpoint(const SOCK_ADDR* endpoint)
{
	if(!local_group) return 0;
	return memcmp(endpoint, &local_group->endpoint, sizeof(SOCK_ADDR))==0?1:0;
}

RPCNET_CONNECTION * conn_alloc(int fd, int conn_type)
{
	RPCNET_CONNECTION *conn = NULL;

	conn = mempool_alloc(conn_pool);
	if(NULL==conn) return NULL;

	rlist_clear(&conn->item_ctx, conn);
	rlist_clear(&conn->item_grp, conn);
	conn->ctx			= NULL;
	conn->group			= NULL;
	conn->fd			= fd;
	conn->type			= conn_type;
	conn->st_flag		= RPCCONN_STATE_AVAILABLE;

	if(RPCCONN_TYPE_OUTGOING==conn_type) {
		atom_inc((unsigned int*)&conn_outgoing_count);
	} else {
		atom_inc((unsigned int*)&conn_incoming_count);
	}

	return conn;
}

void conn_free(RPCNET_CONNECTION *conn)
{
	sock_close(conn->fd);
	assert(conn->group==NULL);
	assert(conn->ctx==NULL);
	if(conn->type==RPCCONN_TYPE_INCOMING) atom_dec((unsigned int*)&conn_incoming_count);
	if(conn->type==RPCCONN_TYPE_OUTGOING) atom_dec((unsigned int*)&conn_outgoing_count);
	mempool_free(conn_pool, conn);
}

void conn_shutdown(RPCNET_CONNECTION* conn)
{
	if(RPCCONN_STATE_AVAILABLE==atom_cas((unsigned int*)&conn->st_flag, RPCCONN_STATE_SHUTDOWN, RPCCONN_STATE_AVAILABLE)) {
		if(sock_disconnect(conn->fd)!=0) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "error in here");
		}
	}
}

void rpcnet_stream_clear(RPCNET_STREAM* stream)
{
	stream->cur = 0;
	stream->ctx->pkg.len = 0;
}

void rpcnet_stream_destroy(RPCNET_STREAM* stream)
{
}

int rpcnet_stream_skip(RPCNET_STREAM* stream, unsigned int len)
{
	return ERR_UNKNOWN;
}

int rpcnet_stream_seek(RPCNET_STREAM* stream, unsigned int loc)
{
	return ERR_UNKNOWN;
}

int rpcnet_stream_put(RPCNET_STREAM* stream, void* buf, unsigned int len)
{
	if(stream->cur+len>sizeof(stream->ctx->pkg.body)) return ERR_NO_ENOUGH_MEMORY;
	memcpy(stream->ctx->pkg.body+stream->cur, buf, len);
	stream->cur += len;
	if(stream->cur>stream->ctx->pkg.len) stream->ctx->pkg.len = stream->cur;
	return ERR_NOERROR;
}

int rpcnet_stream_get(RPCNET_STREAM* stream, void* buf, unsigned int len)
{
	if(stream->cur+len>stream->ctx->pkg.len) return ERR_NO_ENOUGH_MEMORY;
	memcpy(buf, stream->ctx->pkg.body+stream->cur, len);
	stream->cur += len;
	return ERR_NOERROR;
}

void context_addconn(RPCNET_THREAD_CONTEXT* ctx, RPCNET_CONNECTION* conn)
{
	rlist_push_back(&ctx->hold_conns, &conn->item_ctx);
	conn->ctx = ctx;
}

void context_delconn(RPCNET_CONNECTION* conn)
{
	if(conn->ctx!=NULL) {
		rlist_remove(&conn->ctx->hold_conns, &conn->item_ctx);
		conn->ctx = NULL;
		if(conn->group==NULL) conn_free(conn);
	}
}


RPCNET_GROUP* group_get(const SOCK_ADDR* endpoint)
{
	int index;
	RPCNET_GROUP* group;

	index = rpcnet_ep2idx(endpoint);
	if(index<0) return NULL;
	group = group_map[index];
	if(group!=NULL) return group;
	
	// create new group and put it into hashtable
	group = mempool_alloc(group_pool);
	if(group==NULL) return NULL;

	group_map[index] = group;

	// init new group
	memcpy(&group->endpoint, endpoint, sizeof(SOCK_ADDR));
	rlist_init(&group->in_conns_list);
	rlist_init(&group->out_conns_list);

	// notify
	group_notify(group, RPCNET_EVENTTYPE_INIT);

	return group;
}

void group_free(RPCNET_GROUP* group)
{
	assert(rlist_empty(&group->in_conns_list));
	assert(rlist_empty(&group->out_conns_list));

	mempool_free(group_pool, group);
}

void group_addconn(RPCNET_GROUP* group, RPCNET_CONNECTION* conn)
{
	conn->group = group;
	if(conn->type==RPCCONN_TYPE_INCOMING) {
		rlist_push_back(&group->in_conns_list, &conn->item_grp);
	} else {
		rlist_push_back(&group->out_conns_list, &conn->item_grp);
	}
}

void group_delconn(RPCNET_CONNECTION* conn)
{
	RPCNET_GROUP* group;
	RPCNET_CONNECTION* c;

	if(conn->group!=NULL) {
		group = conn->group;
		conn->group = NULL;
		if(conn->type==RPCCONN_TYPE_INCOMING) {
			rlist_remove(&group->in_conns_list, &conn->item_grp);
		} else {
			rlist_remove(&group->out_conns_list, &conn->item_grp);
		}

		if(conn->type==RPCCONN_TYPE_INCOMING && rlist_empty(&group->in_conns_list)) {
			// disconnection all outgoing connections
			while(!rlist_empty(&group->out_conns_list)) {
				c = (RPCNET_CONNECTION*)rlist_get_userdata(rlist_front(&group->out_conns_list));
				conn_shutdown(c);
				group_delconn(c);
				if(c->ctx==NULL) conn_free(c);
			}

			// call notify
			group_notify(group, RPCNET_EVENTTYPE_RESET);
		}
	}
}

void group_notify(RPCNET_GROUP* group, int type)
{
	int i;
	RPCNET_EVENTDATA evtdata;

	switch(type) {
	case RPCNET_EVENTTYPE_RESET:
		evtdata.RESET.group = group;
		break;
	case RPCNET_EVENTTYPE_INIT:
		evtdata.INIT.group = group;
		break;
	default:
		assert("invalid type"==NULL);
		return;
	}

	for(i=0; i<RPCNET_SUBSYS_MAXCOUNT; i++) {
		if(subsys_events[i]!=NULL) {
			subsys_events[i](type, &evtdata);
		}
	}
}

RPCNET_CONNECTION* group_get_outgoing(RPCNET_GROUP* group, RPCNET_THREAD_CONTEXT* ctx)
{
	RLIST_ITEM* item;
	RPCNET_CONNECTION* conn = NULL;
	for(item=rlist_front(&group->out_conns_list); !rlist_is_head(&group->out_conns_list, item); item=rlist_next(item)) {
		conn = (RPCNET_CONNECTION*)rlist_get_userdata(rlist_front(&group->out_conns_list));
		if(atom_cas_ptr(&conn->ctx_void, ctx, NULL)==NULL) {
			break;
		}
	}
	if(conn!=NULL) context_addconn(ctx, conn);
	return conn;
}

