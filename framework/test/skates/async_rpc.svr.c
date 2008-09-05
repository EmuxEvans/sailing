#include <stdio.h>
#include <skates/skates.h>

#include "async_rpc.h"

static int test_rpc1_stub(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* _reserve_conn, RPCFUN_REQUEST_SEQ* _reserve_seq, STREAM* _reserve_in);
static int test_rpc2_stub(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* _reserve_conn, RPCFUN_REQUEST_SEQ* _reserve_seq, STREAM* _reserve_in);

RPCFUN_FUNCTION_DESC __async_rpc_desc[3] = {
	{ {{NULL, NULL, NULL}, NULL, 0, 0}, 0, "test_rpc1", test_rpc1_stub, test_rpc1_impl },
	{ {{NULL, NULL, NULL}, NULL, 0, 0}, 0, "test_rpc2", test_rpc2_stub, test_rpc2_impl },
	{ {{NULL, NULL, NULL}, NULL, 0, 0}, 0, "", NULL, NULL },
};

int test_rpc1_stub(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* _reserve_conn, RPCFUN_REQUEST_SEQ* _reserve_seq, STREAM* _reserve_in)
{
	// define parameter
	int r1;
	int* r2;
	int* r3;
	int* r4;
	// reserve
	int _reserve_ret;
	STREAM* _reserve_stream = NULL;
	RPCFUN_PARAMETER_ITEM	_reserve_list[4];


	// step  1: fill parameter list
	_reserve_list[0].mode = (0);
	_reserve_list[0].max = 0;
	_reserve_list[0].count = 1;
	_reserve_list[0].size = sizeof(int);
	_reserve_list[0].buf = &r1;
	_reserve_list[1].mode = (0|RPCFUN_SERIALIZE_IN);
	_reserve_list[1].max = 0;
	_reserve_list[1].count = 1;
	_reserve_list[1].size = sizeof(int);
	_reserve_list[1].buf = NULL;
	_reserve_list[2].mode = (0|RPCFUN_SERIALIZE_OUT);
	_reserve_list[2].max = 0;
	_reserve_list[2].count = 1;
	_reserve_list[2].size = sizeof(int);
	_reserve_list[2].buf = NULL;
	_reserve_list[3].mode = (0|RPCFUN_SERIALIZE_OUT);
	_reserve_list[3].max = 0;
	_reserve_list[3].count = 1;
	_reserve_list[3].size = sizeof(int);
	_reserve_list[3].buf = NULL;

	// step  2: read parameter from stream
	_reserve_ret = rpcfun_serialize_read(_reserve_in, _reserve_list, 4, ctx);
	if(_reserve_ret!=ERR_NOERROR) {
		DBGLOG(LOG_INFO, MODULE_NAME, "rpc serialize read error in %s, ret=%d", __FUNCTION__, _reserve_ret);
		if(_reserve_conn==NULL && _reserve_seq==NULL) {
			return(_reserve_ret);
		} else {
			goto SEND_ERROR;
		}
	}

	// step  3: fill pointer
	r2 = (int*)_reserve_list[1].buf;
	r3 = (int*)_reserve_list[2].buf;
	r4 = (int*)_reserve_list[3].buf;

	// step  4: call impl
	_reserve_ret = test_rpc1_impl(group, r1, r2, r3, r4);
	if(_reserve_conn==NULL && _reserve_seq==NULL) return(_reserve_ret);
	if(_reserve_ret!=ERR_NOERROR) goto SEND_ERROR;

	// step  5: fill parameter
	_reserve_list[2].count = 1;
	_reserve_list[3].count = 1;

	// step  6: get stream
	_reserve_stream = rpcnet_context_getstream(ctx);
	if(_reserve_stream==NULL) goto SEND_ERROR;

	// step  7: write result
	_reserve_ret = rpcfun_write_result(_reserve_stream, _reserve_seq, _reserve_ret);
	if(_reserve_ret!=ERR_NOERROR) {
		goto SEND_ERROR;
	}

	// step  8: write parameter to stream
	_reserve_ret = rpcfun_serialize_write(_reserve_stream, _reserve_list, 4, ctx);
	if(_reserve_ret!=ERR_NOERROR) {
		DBGLOG(LOG_INFO, MODULE_NAME, "rpc serialize write error in %s, ret=%d", __FUNCTION__, _reserve_ret);
		goto SEND_ERROR;
	}

	// if async_call then
	if(_reserve_seq!=NULL) {
		// step  9: alloc connection
		_reserve_conn = rpcnet_context_getconn(ctx, group);
		if(_reserve_conn==NULL) return(ERR_UNKNOWN);
	}
	// endif

	// step 10: send stream;
	_reserve_ret = rpcnet_conn_write(_reserve_conn, _reserve_seq==NULL?RPCNET_SUBSYS_SYNCRPC_RESPONSE:RPCNET_SUBSYS_ASYNCRPC_RESPONSE, _reserve_stream);
	if(_reserve_ret!=ERR_NOERROR) return(_reserve_ret);


	// done.
	return(ERR_NOERROR);

SEND_ERROR:
	if(_reserve_conn==NULL) {
		_reserve_conn = rpcnet_context_getconn(ctx, group);
		if(_reserve_conn==NULL) return(ERR_UNKNOWN);
	}
	if(_reserve_stream==NULL) {
		_reserve_stream = rpcnet_context_getstream(ctx);
		if(_reserve_stream==NULL) return(ERR_UNKNOWN);
	} else {
		stream_clear(_reserve_stream);
	}
	_reserve_ret = rpcfun_write_result(_reserve_stream, _reserve_seq, _reserve_ret);
	if(_reserve_ret!=ERR_NOERROR) {
		return(_reserve_ret);
	}
	_reserve_ret = rpcnet_conn_write(_reserve_conn, _reserve_seq==NULL?RPCNET_SUBSYS_SYNCRPC_RESPONSE:RPCNET_SUBSYS_ASYNCRPC_RESPONSE, _reserve_stream);
	if(_reserve_ret!=ERR_NOERROR) {
		return(_reserve_ret);
	}
	return(ERR_NOERROR);
}

int test_rpc2_stub(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* _reserve_conn, RPCFUN_REQUEST_SEQ* _reserve_seq, STREAM* _reserve_in)
{
	// define parameter
	int* r1;
	int* r2;
	int* r3;
	int* r4;
	// reserve
	int _reserve_ret;
	STREAM* _reserve_stream = NULL;
	RPCFUN_PARAMETER_ITEM	_reserve_list[4];


	// step  1: fill parameter list
	_reserve_list[0].mode = (0|RPCFUN_SERIALIZE_IN|RPCFUN_SERIALIZE_OUT);
	_reserve_list[0].max = 0;
	_reserve_list[0].count = 1;
	_reserve_list[0].size = sizeof(int);
	_reserve_list[0].buf = NULL;
	_reserve_list[1].mode = (0|RPCFUN_SERIALIZE_IN|RPCFUN_SERIALIZE_OUT);
	_reserve_list[1].max = 0;
	_reserve_list[1].count = 1;
	_reserve_list[1].size = sizeof(int);
	_reserve_list[1].buf = NULL;
	_reserve_list[2].mode = (0|RPCFUN_SERIALIZE_IN|RPCFUN_SERIALIZE_OUT);
	_reserve_list[2].max = 0;
	_reserve_list[2].count = 1;
	_reserve_list[2].size = sizeof(int);
	_reserve_list[2].buf = NULL;
	_reserve_list[3].mode = (0|RPCFUN_SERIALIZE_IN|RPCFUN_SERIALIZE_OUT);
	_reserve_list[3].max = 0;
	_reserve_list[3].count = 1;
	_reserve_list[3].size = sizeof(int);
	_reserve_list[3].buf = NULL;

	// step  2: read parameter from stream
	_reserve_ret = rpcfun_serialize_read(_reserve_in, _reserve_list, 4, ctx);
	if(_reserve_ret!=ERR_NOERROR) {
		DBGLOG(LOG_INFO, MODULE_NAME, "rpc serialize read error in %s, ret=%d", __FUNCTION__, _reserve_ret);
		if(_reserve_conn==NULL && _reserve_seq==NULL) {
			return(_reserve_ret);
		} else {
			goto SEND_ERROR;
		}
	}

	// step  3: fill pointer
	r1 = (int*)_reserve_list[0].buf;
	r2 = (int*)_reserve_list[1].buf;
	r3 = (int*)_reserve_list[2].buf;
	r4 = (int*)_reserve_list[3].buf;

	// step  4: call impl
	_reserve_ret = test_rpc2_impl(group, r1, r2, r3, r4);
	if(_reserve_conn==NULL && _reserve_seq==NULL) return(_reserve_ret);
	if(_reserve_ret!=ERR_NOERROR) goto SEND_ERROR;

	// step  5: fill parameter
	_reserve_list[0].count = 1;
	_reserve_list[1].count = 2;
	_reserve_list[2].count = 1;
	_reserve_list[3].count = 2;

	// step  6: get stream
	_reserve_stream = rpcnet_context_getstream(ctx);
	if(_reserve_stream==NULL) goto SEND_ERROR;

	// step  7: write result
	_reserve_ret = rpcfun_write_result(_reserve_stream, _reserve_seq, _reserve_ret);
	if(_reserve_ret!=ERR_NOERROR) {
		goto SEND_ERROR;
	}

	// step  8: write parameter to stream
	_reserve_ret = rpcfun_serialize_write(_reserve_stream, _reserve_list, 4, ctx);
	if(_reserve_ret!=ERR_NOERROR) {
		DBGLOG(LOG_INFO, MODULE_NAME, "rpc serialize write error in %s, ret=%d", __FUNCTION__, _reserve_ret);
		goto SEND_ERROR;
	}

	// if async_call then
	if(_reserve_seq!=NULL) {
		// step  9: alloc connection
		_reserve_conn = rpcnet_context_getconn(ctx, group);
		if(_reserve_conn==NULL) return(ERR_UNKNOWN);
	}
	// endif

	// step 10: send stream;
	_reserve_ret = rpcnet_conn_write(_reserve_conn, _reserve_seq==NULL?RPCNET_SUBSYS_SYNCRPC_RESPONSE:RPCNET_SUBSYS_ASYNCRPC_RESPONSE, _reserve_stream);
	if(_reserve_ret!=ERR_NOERROR) return(_reserve_ret);


	// done.
	return(ERR_NOERROR);

SEND_ERROR:
	if(_reserve_conn==NULL) {
		_reserve_conn = rpcnet_context_getconn(ctx, group);
		if(_reserve_conn==NULL) return(ERR_UNKNOWN);
	}
	if(_reserve_stream==NULL) {
		_reserve_stream = rpcnet_context_getstream(ctx);
		if(_reserve_stream==NULL) return(ERR_UNKNOWN);
	} else {
		stream_clear(_reserve_stream);
	}
	_reserve_ret = rpcfun_write_result(_reserve_stream, _reserve_seq, _reserve_ret);
	if(_reserve_ret!=ERR_NOERROR) {
		return(_reserve_ret);
	}
	_reserve_ret = rpcnet_conn_write(_reserve_conn, _reserve_seq==NULL?RPCNET_SUBSYS_SYNCRPC_RESPONSE:RPCNET_SUBSYS_ASYNCRPC_RESPONSE, _reserve_stream);
	if(_reserve_ret!=ERR_NOERROR) {
		return(_reserve_ret);
	}
	return(ERR_NOERROR);
}


