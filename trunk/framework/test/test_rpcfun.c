#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/skates.h"

static const char svr_edp[] = "127.0.0.1:30000";
static const char clt_edp[] = "127.0.0.1:20000";

static void client_event(void* arg);
static void server_event(void* arg);

static int a_stub(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* conn, RPCFUN_REQUEST_SEQ* seq, STREAM* in);
static void a_callback(int errcode, RPCFUN_ASYNC_OVERLAPPED* overlapped);
static int a_impl(RPCNET_GROUP* group, int arg1);
static int a_proxy(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap, int arg1);

static int b_stub(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* conn, RPCFUN_REQUEST_SEQ* seq, STREAM* in);
static void b_callback(int errcode, RPCFUN_ASYNC_OVERLAPPED* overlapped);
static int b_impl(RPCNET_GROUP* group, int arg1);
static int b_proxy(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap, int arg1);

static RPCFUN_FUNCTION_DESC a_desc = { {{NULL, NULL, NULL}, NULL, 0, 0}, 0, "a", a_stub, a_impl };
static RPCFUN_FUNCTION_DESC b_desc = { {{NULL, NULL, NULL}, NULL, 0, 0}, 0, "b", b_stub, b_impl };

static RPCFUN_ASYNC_OVERLAPPED a_overlap;
static RPCFUN_ASYNC_OVERLAPPED b_overlap;

int main(int argc, char* argv[])
{
	SOCK_ADDR addr;

	if(argc!=2 || (strcmp(argv[1], "S")!=0 && strcmp(argv[1], "C")!=0)) {
		printf("invlid paramter\n");
		return(-1);
	}

	mempool_init();
	sock_init();
	printf("fdwatch_init ret = %d\n",  fdwatch_init());
	printf("threadpool_init ret = %d\n",  threadpool_init(10));
	printf("rpcnet_init ret = %d\n",  rpcnet_init());
	printf("rpcfun_init ret = %d\n",  rpcfun_init());

	if(strcmp(argv[1], "S")==0) {
		sock_str2addr(svr_edp, &addr);
	} else {
		sock_str2addr(clt_edp, &addr);
	}
	printf("rpcnet_bind ret = %d\n", rpcnet_bind(&addr));

	rpcfun_register(&a_desc, 1);
	rpcfun_register(&b_desc, 1);

	if(strcmp(argv[1], "S")==0) {
		threadpool_queueitem(server_event, NULL);
	} else {
		threadpool_queueitem(client_event, NULL);
	}

	printf("press enter to exit!\n");
	getchar();

	rpcfun_unregister(&a_desc, 1);
	rpcfun_unregister(&b_desc, 1);

	printf("rpcnet_unbind ret = %d\n",  rpcnet_unbind());
	printf("rpcnet_shutdown ret = %d\n", rpcnet_shutdown());
	printf("rpcfun_final ret = %d\n",  rpcfun_final());
	printf("rpcnet_final ret = %d\n",  rpcnet_final());
	printf("threadpool_final ret = %d\n",  threadpool_final());
	printf("fdwatch_final = %d\n",  fdwatch_final());
	sock_final();
	mempool_final();

	return 0;
}

void client_event(void* ptr)
{
	SOCK_ADDR		addr;
	RPCNET_GROUP*	group;
	printf("do client_event!\n");
	if(sock_str2addr(svr_edp, &addr)==NULL) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		return;
	}
	group = rpcnet_getgroup(&addr);
	if(group==NULL) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		return;
	}

	printf("call b_proxy() return %d\n", b_proxy(group, NULL, 1));
	b_overlap.callback = b_callback;
	printf("call b_proxy() return %d\n", b_proxy(group, &b_overlap, 2));

	printf("call a_proxy() return %d\n", a_proxy(group, NULL, 1));
	a_overlap.callback = a_callback;
	printf("call a_proxy() return %d\n", a_proxy(group, &a_overlap, 2));
}

void server_event(void* ptr)
{
	printf("do server_event!\n");
}

int a_stub(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* conn, RPCFUN_REQUEST_SEQ* seq, STREAM* in)
{
	return ERR_NOERROR;
}

void a_callback(int errcode, RPCFUN_ASYNC_OVERLAPPED* overlapped)
{
}

int a_impl(RPCNET_GROUP* group, int arg1)
{
	return ERR_NOERROR;
}

int a_proxy(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap, int arg1)
{
	return ERR_NOERROR;
}

int b_stub(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* conn, RPCFUN_REQUEST_SEQ* seq, STREAM* in)
{
	// define parameter

	// step  1: fill parameter list
	// step  2: read parameter from stream
	// step  3: fill pointer
	// step  4: call impl
	// step  5: fill parameter
	// step  6: get stream
	// step  7: write result
	// step  8: write parameter to stream

	// if async_call then
	// step  9: alloc connection
	// endif
	
	// step 10: send stream;

	struct {
		int arg1;
	} _reserved;
	int ret;
	STREAM*					stream = NULL;
	RPCFUN_PARAMETER_ITEM	list[RPCFUN_MAX_PARAMETER];

	// step  1: fill parameter list
	list[0].mode = 0;
	list[0].max = 0;
	list[0].count = 1;
	list[0].size = sizeof(int);
	list[0].buf = &_reserved.arg1;

	// step  2: read parameter from stream
	ret = rpcfun_serialize_read(in, list, 1, ctx);
	if(ret!=ERR_NOERROR) goto SEND_ERROR;

	// step  3: fill pointer

	// step  4: call impl
	ret = b_impl(group, _reserved.arg1);
	if(ret!=ERR_NOERROR) goto SEND_ERROR;

	// step  5: fill parameter

	// step  6: get stream
	stream = rpcnet_context_getstream(ctx);
	if(stream==NULL) goto SEND_ERROR;

	// step  7: write result
	ret = rpcfun_write_result(stream, seq, ret);
	if(ret!=ERR_NOERROR) goto SEND_ERROR;

	// step  8: write parameter to stream
	ret = rpcfun_serialize_write(stream, list, 1, ctx);
	if(ret!=ERR_NOERROR) goto SEND_ERROR;

	// if async_call then
	if(seq!=NULL) {
		// step  9: alloc connection
		conn = rpcnet_context_getconn(ctx, group);
		if(conn==NULL) return ERR_UNKNOWN;
	}
	// endif
	os_sleep(100);
	// step 10: send stream;
	ret = rpcnet_conn_write(conn, seq==NULL?RPCNET_SUBSYS_SYNCRPC_RESPONSE:RPCNET_SUBSYS_ASYNCRPC_RESPONSE, stream);
	if(ret!=ERR_NOERROR) return ERR_UNKNOWN;

	// done.
	return ERR_NOERROR;
	
SEND_ERROR:
	if(conn==NULL) {
		conn = rpcnet_context_getconn(ctx, group);
		if(conn==NULL) return ERR_UNKNOWN;
	}
	if(stream==NULL) {
		stream = rpcnet_context_getstream(ctx);
		if(stream==NULL) return ERR_UNKNOWN;
	} else {
		stream_clear(stream);
	}
	ret = rpcfun_write_result(stream, seq, ret);
	if(ret!=ERR_NOERROR) return ERR_UNKNOWN;
	ret = rpcnet_conn_write(conn, seq==NULL?RPCNET_SUBSYS_SYNCRPC_RESPONSE:RPCNET_SUBSYS_ASYNCRPC_RESPONSE, stream);
	if(ret!=ERR_NOERROR) return ERR_UNKNOWN;
	return ERR_UNKNOWN;
}

void b_callback(int errcode, RPCFUN_ASYNC_OVERLAPPED* overlapped)
{
	SOCK_ADDR		addr;
	RPCNET_GROUP*	group;
	printf("do client_event!\n");
	if(sock_str2addr(svr_edp, &addr)==NULL) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		return;
	}
	group = rpcnet_getgroup(&addr);
	if(group==NULL) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		return;
	}

	printf("call b_proxy() return %d\n", b_proxy(group, NULL, 1));

	b_overlap.callback = b_callback;
	printf("call b_proxy() return %d\n", b_proxy(group, &b_overlap, 2));
}

int b_impl(RPCNET_GROUP* group, int arg1)
{
	printf("on b_impl(%d)!\n", arg1);

	return ERR_NOERROR;
}

int b_proxy(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap, int arg1)
{
	// step  1: full parameter list
	// step  2: alloc thread context
	// step  3: alloc connection
	// step  4: get stream

	// if async_rpc then ** async call
	// step  5: register request sequse
	// endif

	// step  6: write header to stream
	// step  7: write parameter to stream
	// step  8: send stream

	// if sync_rpc then ** sync call
	// step  9: get stream
	// step 10: receive stream
	// step 11: read result from stream
	// step 12: read parameter from stream
	// endif

	// step 12: free thread context
	// step 13: return RETCODE

	struct {
		RPCNET_THREAD_CONTEXT*		ctx;
		RPCNET_CONNECTION*			conn;
		RPCFUN_REQUEST_SEQ				seq;
		STREAM*							stream;

		RPCFUN_PARAMETER_ITEM*			list;
		int*							list_count;

		RPCFUN_PARAMETER_ITEM			a_list[RPCFUN_MAX_PARAMETER];
		int								a_list_count;
		int								retcode;
	} _reserved;

	// ** start
	if(overlap==NULL) {
		_reserved.list			= _reserved.a_list;
		_reserved.list_count	= &_reserved.a_list_count;
	} else {
		_reserved.list			= overlap->_reserved.list;
		_reserved.list_count	= &overlap->_reserved.list_count;
	}

	// step  1: full parameter list
	*_reserved.list_count = 1;
	_reserved.list[0].mode = 0;
	_reserved.list[0].max = 0;
	_reserved.list[0].count = 1;
	_reserved.list[0].size = sizeof(int);
	_reserved.list[0].buf = &arg1;

	// step  2: alloc thread context
	_reserved.ctx = rpcnet_context_get();
	if(_reserved.ctx==NULL) return ERR_NOT_WORKER;

	// step  3: alloc connection
	_reserved.conn = rpcnet_context_getconn(_reserved.ctx, group);
	if(_reserved.conn==NULL) { rpcnet_context_free(_reserved.ctx); return ERR_CONNECTION_FAILED; }

	// step  4: get stream
	_reserved.stream = rpcnet_context_getstream(_reserved.ctx);
	if(_reserved.stream==NULL) { rpcnet_context_free(_reserved.ctx); return ERR_UNKNOWN; }

	// if async_rpc then ** async call
	// step  5: register request sequse
	// endif
	if(overlap!=NULL) {
		_reserved.retcode = rpcfun_register_overlap(group, overlap, &_reserved.seq);
		if(_reserved.retcode!=ERR_NOERROR) {
			rpcnet_context_free(_reserved.ctx);
			return _reserved.retcode;
		}
	}

	// step  6: write header to stream
	_reserved.retcode = rpcfun_write_head(_reserved.stream, "b", overlap==NULL?NULL:&_reserved.seq);
	if(_reserved.retcode!=ERR_NOERROR) { rpcnet_context_free(_reserved.ctx); return _reserved.retcode; }

	// step  7: write parameter to stream
	_reserved.retcode = rpcfun_serialize_write(_reserved.stream, _reserved.list, *_reserved.list_count, NULL);
	if(_reserved.retcode!=ERR_NOERROR) { rpcnet_context_free(_reserved.ctx); return _reserved.retcode; }

	// step  8: send stream
	_reserved.retcode = rpcnet_conn_write(_reserved.conn, overlap==NULL?RPCNET_SUBSYS_SYNCRPC:RPCNET_SUBSYS_ASYNCRPC_REQUEST, _reserved.stream);
	if(_reserved.retcode!=ERR_NOERROR) { rpcnet_context_free(_reserved.ctx); return _reserved.retcode; }
	
	// if sync_rpc then ** sync call
	if(overlap==NULL) {
		// step  9: get stream
		_reserved.stream = rpcnet_context_getstream(_reserved.ctx);
		if(_reserved.stream==NULL) { rpcnet_context_free(_reserved.ctx); return ERR_UNKNOWN; }

		// step 10: receive stream
		_reserved.retcode = rpcnet_conn_read(group, _reserved.conn, RPCNET_SUBSYS_SYNCRPC_RESPONSE, _reserved.stream);
		if(_reserved.retcode!=ERR_NOERROR) { rpcnet_context_free(_reserved.ctx); return _reserved.retcode; }

		// step 11: read result from stream
		_reserved.retcode = rpcfun_read_result(_reserved.stream, NULL, &_reserved.retcode);
		if(_reserved.retcode!=ERR_NOERROR) { rpcnet_context_free(_reserved.ctx); return _reserved.retcode; }

		// step 12: read parameter from stream
		_reserved.retcode = rpcfun_serialize_read(_reserved.stream, _reserved.list, *_reserved.list_count, NULL);
		if(_reserved.retcode!=ERR_NOERROR) { rpcnet_context_free(_reserved.ctx); return _reserved.retcode; }
	}
	// endif

	// step 12: free thread context
	rpcnet_context_free(_reserved.ctx);

	// step 13: return RETCODE
	return overlap==NULL?ERR_NOERROR:ERR_PENDING;
}
