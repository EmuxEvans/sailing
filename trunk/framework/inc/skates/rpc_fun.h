#ifndef _RPC_FUN_H_
#define _RPC_FUN_H_

#ifdef __cplusplus
extern "C" {
#endif

// define rpc_net:subsys
#define RPCNET_SUBSYS_SYNCRPC				0
#define RPCNET_SUBSYS_ASYNCRPC_REQUEST		1
#define RPCNET_SUBSYS_ASYNCRPC_RESPONSE		2
#define RPCNET_SUBSYS_ASYNCRPC_NOCALLBACK	3
#define RPCNET_SUBSYS_SYNCRPC_RESPONSE		101

#define RPCFUN_MAX_PARAMETER		10
#define RPCFUN_FUNCNAME_MAXLEN		250

#define RPCFUN_SERIALIZE_IN			0x010
#define RPCFUN_SERIALIZE_OUT		0x001
#define RPCFUN_SERIALIZE_REF		0x100

struct RPCFUN_PARAMETER_ITEM;
typedef struct RPCFUN_PARAMETER_ITEM RPCFUN_PARAMETER_ITEM;

struct RPCFUN_ASYNC_OVERLAPPED;
typedef struct RPCFUN_ASYNC_OVERLAPPED RPCFUN_ASYNC_OVERLAPPED;

struct RPCFUN_REQUEST_SEQ;
typedef struct RPCFUN_REQUEST_SEQ RPCFUN_REQUEST_SEQ;

struct RPCFUN_FUNCTION_DESC;
typedef struct RPCFUN_FUNCTION_DESC RPCFUN_FUNCTION_DESC;	

typedef int (*RPCFUN_FUNCTION_STUB)(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* conn, RPCFUN_REQUEST_SEQ* seq, STREAM* in);
typedef void (*RPCFUN_ASYNC_CALLBACK)(int errcode, RPCFUN_ASYNC_OVERLAPPED* overlapped);

struct RPCFUN_REQUEST_SEQ {
	unsigned short index;
	unsigned short seq;
};

struct RPCFUN_FUNCTION_DESC {
	HASHMAP_ITEM			item;
	//
	int						ref_count;
	//
	char*					funcname;
	RPCFUN_FUNCTION_STUB	stub;
	void *					impl;
};

struct RPCFUN_PARAMETER_ITEM {
	int mode, max, count, size; void* buf;
};

struct RPCFUN_ASYNC_OVERLAPPED {
	struct {
		RPCFUN_ASYNC_OVERLAPPED*	next;
		RPCFUN_ASYNC_OVERLAPPED*	prev;
		RPCFUN_PARAMETER_ITEM		list[RPCFUN_MAX_PARAMETER];
		int							list_count;
		RPCFUN_REQUEST_SEQ			seq;
	} _reserved;

	// user data
	RPCFUN_ASYNC_CALLBACK callback;
};

extern RPCFUN_ASYNC_OVERLAPPED* RPCFUN_NOCALLBACK;

ZION_API int rpcfun_init();
ZION_API int rpcfun_final();

ZION_API int rpcfun_register(RPCFUN_FUNCTION_DESC* desc, int count);
ZION_API void rpcfun_unregister(RPCFUN_FUNCTION_DESC* desc, int count);

ZION_API RPCFUN_FUNCTION_DESC* rpcfun_get_function(const char* funcname);
ZION_API void rpcfun_release_function(RPCFUN_FUNCTION_DESC* desc);

ZION_API int rpcfun_write_head(STREAM* stream, const char* funcname, const RPCFUN_REQUEST_SEQ* seq);
ZION_API int rpcfun_read_head(STREAM* stream, char* funcname, RPCFUN_REQUEST_SEQ* seq);

ZION_API int rpcfun_write_result(STREAM* stream, const RPCFUN_REQUEST_SEQ* seq, const int retcode);
ZION_API int rpcfun_read_result(STREAM* stream, RPCFUN_REQUEST_SEQ* seq, int* retcode);

ZION_API int rpcfun_register_overlap(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap, RPCFUN_REQUEST_SEQ* seq);
ZION_API RPCFUN_ASYNC_OVERLAPPED* rpcfun_unregister_overlap(RPCNET_GROUP* group, RPCFUN_REQUEST_SEQ* seq);

ZION_API int rpcfun_serialize_read(STREAM* stream, RPCFUN_PARAMETER_ITEM* list, int count, RPCNET_THREAD_CONTEXT* ctx);
ZION_API int rpcfun_serialize_write(STREAM* stream, RPCFUN_PARAMETER_ITEM* list, int count, RPCNET_THREAD_CONTEXT* ctx);

ZION_API int rpcfun_localasync_post(STREAM* stream, RPCFUN_ASYNC_OVERLAPPED* overlap);

ZION_API STREAM* rpcfun_stream_alloc();
ZION_API void rpcfun_stream_free(STREAM* stream);
ZION_API int rpcfun_stream_isvalid(STREAM* stream);

#ifdef __cplusplus
}
#endif

#endif
