#include <stdio.h>
#include <skates/skates.h>

#include "async_rpc.h"

int test_rpc1(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap, int r1, const int* r2, int* r3, int* r4)
{
	struct {
		RPCNET_THREAD_CONTEXT*		ctx;
		RPCNET_CONNECTION*			conn;
		RPCFUN_REQUEST_SEQ				seq;
		STREAM*							stream;

		RPCFUN_PARAMETER_ITEM*			list;

		RPCFUN_PARAMETER_ITEM			a_list[5];
		int								retcode;
		int								result;

		RPCFUN_FUNCTION_DESC*			func_desc;
	} _reserved;

	if(rpcnet_group_islocal(group)!=0) {
		_reserved.func_desc = rpcfun_get_function("test_rpc1");
		if(_reserved.func_desc==NULL) return ERR_NOT_IMPLEMENT;
		if(overlap==NULL) {
			_reserved.retcode = ((int (*)(RPCNET_GROUP* group, int r1, const int* r2, int* r3, int* r4))_reserved.func_desc->impl)(group, r1, r2, r3, r4);
			rpcfun_release_function(_reserved.func_desc);
			return(_reserved.retcode);
		} else {
			if(overlap==RPCFUN_NOCALLBACK) {
				_reserved.list = _reserved.a_list;
			} else {
				_reserved.list = overlap->_reserved.list;
			}
			overlap->_reserved.list_count = 4;
		}
	} else {
		_reserved.func_desc = NULL;
		if(overlap==NULL || overlap==RPCFUN_NOCALLBACK) {
			_reserved.list = _reserved.a_list;
		} else {
			_reserved.list = overlap->_reserved.list;
			overlap->_reserved.list_count = 4;
		}
	}


	// step  1: full parameter list
	_reserved.list[0].mode = (0);
	_reserved.list[0].max = 0;
	_reserved.list[0].count = 1;
	_reserved.list[0].size = sizeof(int);
	_reserved.list[0].buf = (int*)&r1;
	_reserved.list[1].mode = (0|RPCFUN_SERIALIZE_IN);
	_reserved.list[1].max = 0;
	_reserved.list[1].count = 1;
	_reserved.list[1].size = sizeof(int);
	_reserved.list[1].buf = (int*)r2;
	_reserved.list[2].mode = (0|RPCFUN_SERIALIZE_OUT);
	_reserved.list[2].max = 1;
	_reserved.list[2].count = 0;
	_reserved.list[2].size = sizeof(int);
	_reserved.list[2].buf = (int*)r3;
	_reserved.list[3].mode = (0|RPCFUN_SERIALIZE_OUT);
	_reserved.list[3].max = 2;
	_reserved.list[3].count = 0;
	_reserved.list[3].size = sizeof(int);
	_reserved.list[3].buf = (int*)r4;
	
	// local async call
	if(overlap!=NULL && _reserved.func_desc!=NULL) {
		//
		_reserved.stream = rpcfun_stream_alloc();
		if(_reserved.stream==NULL) {
			rpcfun_release_function(_reserved.func_desc);
			return(ERR_NO_ENOUGH_MEMORY);
		}
		//
		_reserved.retcode = stream_put(_reserved.stream, &_reserved.func_desc, sizeof(_reserved.func_desc));
		if(_reserved.retcode!=ERR_NOERROR) {
			rpcfun_release_function(_reserved.func_desc);
			return(_reserved.retcode);
		}

		// serialize
		_reserved.retcode = rpcfun_serialize_write(_reserved.stream, _reserved.list, 4, NULL);
		if(_reserved.retcode!=ERR_NOERROR) {
			DBGLOG(LOG_INFO, MODULE_NAME, "rpc serialize write error in %s, ret=%d", __FUNCTION__, _reserved.retcode);
			rpcfun_release_function(_reserved.func_desc);
			return(_reserved.retcode);
		}
		//
		stream_seek(_reserved.stream, 0);
		// post
		_reserved.retcode = rpcfun_localasync_post(_reserved.stream, overlap);
		if(_reserved.retcode!=ERR_NOERROR){
			rpcfun_release_function(_reserved.func_desc);
			return(_reserved.retcode);
		} else {
			return(ERR_PENDING);
		}
	}

	// step  2: alloc thread context
	_reserved.ctx = rpcnet_context_get();
	if(_reserved.ctx==NULL) return ERR_NOT_WORKER;
	
	// step  3: alloc connection
	if(overlap==NULL) {
		_reserved.conn = rpcnet_context_getconn(_reserved.ctx, group);
		if(_reserved.conn==NULL) {
			rpcnet_context_free(_reserved.ctx);
			return ERR_CONNECTION_FAILED;
		}
	} else {
		_reserved.conn = rpcnet_context_getconn_force(_reserved.ctx, group);
		if(_reserved.conn==NULL) {
			rpcnet_context_free(_reserved.ctx);
			return ERR_CONNECTION_FAILED;
		}
	}
	
	// step  4: get stream
	_reserved.stream = rpcnet_context_getstream(_reserved.ctx);
	if(_reserved.stream==NULL) {
		rpcnet_context_free(_reserved.ctx);
		return ERR_UNKNOWN;
	}
	
	// if async_rpc then ** async call
	// step  5: register request sequse
	// endif
	if(overlap!=NULL && overlap!=RPCFUN_NOCALLBACK) {
		_reserved.retcode = rpcfun_register_overlap(group, overlap, &_reserved.seq);
		if(_reserved.retcode!=ERR_NOERROR) {
			rpcnet_context_free(_reserved.ctx);
			return _reserved.retcode;
		}
	}
	
	// step  6: write header to stream
	_reserved.retcode = rpcfun_write_head(_reserved.stream, "test_rpc1", (overlap==NULL || overlap==RPCFUN_NOCALLBACK)?NULL:&_reserved.seq);
	if(_reserved.retcode!=ERR_NOERROR) {
		rpcnet_context_free(_reserved.ctx);
		return _reserved.retcode;
	}
	
	// step  7: write parameter to stream
	_reserved.retcode = rpcfun_serialize_write(_reserved.stream, _reserved.list, 4, NULL);
	if(_reserved.retcode!=ERR_NOERROR) {
		DBGLOG(LOG_INFO, MODULE_NAME, "rpc serialize write error in %s, ret=%d", __FUNCTION__, _reserved.retcode);
		rpcnet_context_free(_reserved.ctx);
		return _reserved.retcode;
	}
	
	// step  8: send stream
	_reserved.retcode = rpcnet_conn_write(_reserved.conn,	overlap==NULL?RPCNET_SUBSYS_SYNCRPC:
							(overlap==RPCFUN_NOCALLBACK?RPCNET_SUBSYS_ASYNCRPC_NOCALLBACK:
							RPCNET_SUBSYS_ASYNCRPC_REQUEST), _reserved.stream);
	if(_reserved.retcode!=ERR_NOERROR) {
		rpcnet_context_free(_reserved.ctx);
		return _reserved.retcode;
	}
	
	// if sync_rpc then ** sync call
	if(overlap==NULL) {
		// step  9: get stream
		_reserved.stream = rpcnet_context_getstream(_reserved.ctx);
		if(_reserved.stream==NULL) {
			rpcnet_context_free(_reserved.ctx);
			return ERR_UNKNOWN;
		}
		
		// step 10: receive stream
		_reserved.retcode = rpcnet_conn_read(group, _reserved.conn, RPCNET_SUBSYS_SYNCRPC_RESPONSE, _reserved.stream);
		if(_reserved.retcode!=ERR_NOERROR) {
			rpcnet_context_free(_reserved.ctx);
			return _reserved.retcode;
		}
		
		// step 11: read result from stream
		_reserved.retcode = rpcfun_read_result(_reserved.stream, NULL, &_reserved.result);
		if(_reserved.retcode!=ERR_NOERROR) {
			rpcnet_context_free(_reserved.ctx);
			return _reserved.retcode;
		}
		if(_reserved.result!=ERR_NOERROR) {
			rpcnet_context_free(_reserved.ctx);
			return _reserved.result;
		}
		
		// step 12: read parameter from stream
		_reserved.retcode = rpcfun_serialize_read(_reserved.stream, _reserved.list, 4, NULL);
		if(_reserved.retcode!=ERR_NOERROR) {
			DBGLOG(LOG_INFO, MODULE_NAME, "rpc serialize read error in %s, ret=%d", __FUNCTION__, _reserved.retcode);
			rpcnet_context_free(_reserved.ctx);
			return _reserved.retcode;
		}
	} else {
		rpcnet_context_freeconn(_reserved.ctx, _reserved.conn);
	}
	// endif
	
	// step 12: free thread context
	rpcnet_context_free(_reserved.ctx);
	

	// step 13: return RETCODE
	return (overlap==NULL || overlap==RPCFUN_NOCALLBACK)?ERR_NOERROR:ERR_PENDING;
}

int test_rpc2(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap, int* r1, int* r2, int* r3, int* r4)
{
	struct {
		RPCNET_THREAD_CONTEXT*		ctx;
		RPCNET_CONNECTION*			conn;
		RPCFUN_REQUEST_SEQ				seq;
		STREAM*							stream;

		RPCFUN_PARAMETER_ITEM*			list;

		RPCFUN_PARAMETER_ITEM			a_list[5];
		int								retcode;
		int								result;

		RPCFUN_FUNCTION_DESC*			func_desc;
	} _reserved;

	if(rpcnet_group_islocal(group)!=0) {
		_reserved.func_desc = rpcfun_get_function("test_rpc2");
		if(_reserved.func_desc==NULL) return ERR_NOT_IMPLEMENT;
		if(overlap==NULL) {
			_reserved.retcode = ((int (*)(RPCNET_GROUP* group, int* r1, int* r2, int* r3, int* r4))_reserved.func_desc->impl)(group, r1, r2, r3, r4);
			rpcfun_release_function(_reserved.func_desc);
			return(_reserved.retcode);
		} else {
			if(overlap==RPCFUN_NOCALLBACK) {
				_reserved.list = _reserved.a_list;
			} else {
				_reserved.list = overlap->_reserved.list;
			}
			overlap->_reserved.list_count = 4;
		}
	} else {
		_reserved.func_desc = NULL;
		if(overlap==NULL || overlap==RPCFUN_NOCALLBACK) {
			_reserved.list = _reserved.a_list;
		} else {
			_reserved.list = overlap->_reserved.list;
			overlap->_reserved.list_count = 4;
		}
	}


	// step  1: full parameter list
	_reserved.list[0].mode = (0|RPCFUN_SERIALIZE_IN|RPCFUN_SERIALIZE_OUT);
	_reserved.list[0].max = 1;
	_reserved.list[0].count = 1;
	_reserved.list[0].size = sizeof(int);
	_reserved.list[0].buf = (int*)r1;
	_reserved.list[1].mode = (0|RPCFUN_SERIALIZE_IN|RPCFUN_SERIALIZE_OUT);
	_reserved.list[1].max = 2;
	_reserved.list[1].count = 1;
	_reserved.list[1].size = sizeof(int);
	_reserved.list[1].buf = (int*)r2;
	_reserved.list[2].mode = (0|RPCFUN_SERIALIZE_IN|RPCFUN_SERIALIZE_OUT);
	_reserved.list[2].max = 2;
	_reserved.list[2].count = 2;
	_reserved.list[2].size = sizeof(int);
	_reserved.list[2].buf = (int*)r3;
	_reserved.list[3].mode = (0|RPCFUN_SERIALIZE_IN|RPCFUN_SERIALIZE_OUT);
	_reserved.list[3].max = 3;
	_reserved.list[3].count = 1;
	_reserved.list[3].size = sizeof(int);
	_reserved.list[3].buf = (int*)r4;
	
	// local async call
	if(overlap!=NULL && _reserved.func_desc!=NULL) {
		//
		_reserved.stream = rpcfun_stream_alloc();
		if(_reserved.stream==NULL) {
			rpcfun_release_function(_reserved.func_desc);
			return(ERR_NO_ENOUGH_MEMORY);
		}
		//
		_reserved.retcode = stream_put(_reserved.stream, &_reserved.func_desc, sizeof(_reserved.func_desc));
		if(_reserved.retcode!=ERR_NOERROR) {
			rpcfun_release_function(_reserved.func_desc);
			return(_reserved.retcode);
		}

		// serialize
		_reserved.retcode = rpcfun_serialize_write(_reserved.stream, _reserved.list, 4, NULL);
		if(_reserved.retcode!=ERR_NOERROR) {
			DBGLOG(LOG_INFO, MODULE_NAME, "rpc serialize write error in %s, ret=%d", __FUNCTION__, _reserved.retcode);
			rpcfun_release_function(_reserved.func_desc);
			return(_reserved.retcode);
		}
		//
		stream_seek(_reserved.stream, 0);
		// post
		_reserved.retcode = rpcfun_localasync_post(_reserved.stream, overlap);
		if(_reserved.retcode!=ERR_NOERROR){
			rpcfun_release_function(_reserved.func_desc);
			return(_reserved.retcode);
		} else {
			return(ERR_PENDING);
		}
	}

	// step  2: alloc thread context
	_reserved.ctx = rpcnet_context_get();
	if(_reserved.ctx==NULL) return ERR_NOT_WORKER;
	
	// step  3: alloc connection
	if(overlap==NULL) {
		_reserved.conn = rpcnet_context_getconn(_reserved.ctx, group);
		if(_reserved.conn==NULL) {
			rpcnet_context_free(_reserved.ctx);
			return ERR_CONNECTION_FAILED;
		}
	} else {
		_reserved.conn = rpcnet_context_getconn_force(_reserved.ctx, group);
		if(_reserved.conn==NULL) {
			rpcnet_context_free(_reserved.ctx);
			return ERR_CONNECTION_FAILED;
		}
	}
	
	// step  4: get stream
	_reserved.stream = rpcnet_context_getstream(_reserved.ctx);
	if(_reserved.stream==NULL) {
		rpcnet_context_free(_reserved.ctx);
		return ERR_UNKNOWN;
	}
	
	// if async_rpc then ** async call
	// step  5: register request sequse
	// endif
	if(overlap!=NULL && overlap!=RPCFUN_NOCALLBACK) {
		_reserved.retcode = rpcfun_register_overlap(group, overlap, &_reserved.seq);
		if(_reserved.retcode!=ERR_NOERROR) {
			rpcnet_context_free(_reserved.ctx);
			return _reserved.retcode;
		}
	}
	
	// step  6: write header to stream
	_reserved.retcode = rpcfun_write_head(_reserved.stream, "test_rpc2", (overlap==NULL || overlap==RPCFUN_NOCALLBACK)?NULL:&_reserved.seq);
	if(_reserved.retcode!=ERR_NOERROR) {
		rpcnet_context_free(_reserved.ctx);
		return _reserved.retcode;
	}
	
	// step  7: write parameter to stream
	_reserved.retcode = rpcfun_serialize_write(_reserved.stream, _reserved.list, 4, NULL);
	if(_reserved.retcode!=ERR_NOERROR) {
		DBGLOG(LOG_INFO, MODULE_NAME, "rpc serialize write error in %s, ret=%d", __FUNCTION__, _reserved.retcode);
		rpcnet_context_free(_reserved.ctx);
		return _reserved.retcode;
	}
	
	// step  8: send stream
	_reserved.retcode = rpcnet_conn_write(_reserved.conn,	overlap==NULL?RPCNET_SUBSYS_SYNCRPC:
							(overlap==RPCFUN_NOCALLBACK?RPCNET_SUBSYS_ASYNCRPC_NOCALLBACK:
							RPCNET_SUBSYS_ASYNCRPC_REQUEST), _reserved.stream);
	if(_reserved.retcode!=ERR_NOERROR) {
		rpcnet_context_free(_reserved.ctx);
		return _reserved.retcode;
	}
	
	// if sync_rpc then ** sync call
	if(overlap==NULL) {
		// step  9: get stream
		_reserved.stream = rpcnet_context_getstream(_reserved.ctx);
		if(_reserved.stream==NULL) {
			rpcnet_context_free(_reserved.ctx);
			return ERR_UNKNOWN;
		}
		
		// step 10: receive stream
		_reserved.retcode = rpcnet_conn_read(group, _reserved.conn, RPCNET_SUBSYS_SYNCRPC_RESPONSE, _reserved.stream);
		if(_reserved.retcode!=ERR_NOERROR) {
			rpcnet_context_free(_reserved.ctx);
			return _reserved.retcode;
		}
		
		// step 11: read result from stream
		_reserved.retcode = rpcfun_read_result(_reserved.stream, NULL, &_reserved.result);
		if(_reserved.retcode!=ERR_NOERROR) {
			rpcnet_context_free(_reserved.ctx);
			return _reserved.retcode;
		}
		if(_reserved.result!=ERR_NOERROR) {
			rpcnet_context_free(_reserved.ctx);
			return _reserved.result;
		}
		
		// step 12: read parameter from stream
		_reserved.retcode = rpcfun_serialize_read(_reserved.stream, _reserved.list, 4, NULL);
		if(_reserved.retcode!=ERR_NOERROR) {
			DBGLOG(LOG_INFO, MODULE_NAME, "rpc serialize read error in %s, ret=%d", __FUNCTION__, _reserved.retcode);
			rpcnet_context_free(_reserved.ctx);
			return _reserved.retcode;
		}
	} else {
		rpcnet_context_freeconn(_reserved.ctx, _reserved.conn);
	}
	// endif
	
	// step 12: free thread context
	rpcnet_context_free(_reserved.ctx);
	

	// step 13: return RETCODE
	return (overlap==NULL || overlap==RPCFUN_NOCALLBACK)?ERR_NOERROR:ERR_PENDING;
}


