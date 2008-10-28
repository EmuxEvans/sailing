#ifndef _RPCNET_H_
#define _RPCNET_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RPCNET_PACKAGE_LENGTH				(64 *1024)	// (2*1024*1024)
#define RPCNET_MEMORYBLOCK_LENGTH			(256*1024)	// (4*1024*1204)
#define RPCNEWTORK_EPINDEX_MAXVALUE			(1000)
#define RPCNET_SUBSYS_MAXCOUNT				5

#define RPCNET_EVENTTYPE_DATA				1
#define RPCNET_EVENTTYPE_RESET				2
#define RPCNET_EVENTTYPE_INIT				3

struct RPCNET_CONNECTION;
typedef struct RPCNET_CONNECTION RPCNET_CONNECTION;

struct RPCNET_GROUP;
typedef struct RPCNET_GROUP RPCNET_GROUP;

struct RPCNET_THREAD_CONTEXT;
typedef struct RPCNET_THREAD_CONTEXT RPCNET_THREAD_CONTEXT;

typedef union RPCNET_EVENTDATA {
	struct {
		RPCNET_THREAD_CONTEXT*	context;
		RPCNET_GROUP*			group;
		RPCNET_CONNECTION*		conn;
		STREAM*					stream;
	} DATA;
	struct {
		RPCNET_GROUP* group;
	} RESET;
	struct {
		RPCNET_GROUP* group;
	} INIT;
} RPCNET_EVENTDATA;

typedef void (*RPCNET_EVENT)(int etype, RPCNET_EVENTDATA* data);

ZION_API int rpcnet_init();
ZION_API int rpcnet_final();
ZION_API int rpcnet_shutdown();

ZION_API int rpcnet_bind(SOCK_ADDR* endpoint);
ZION_API int rpcnet_unbind();
ZION_API const SOCK_ADDR* rpcnet_get_bindep();

ZION_API RPCNET_GROUP* rpcnet_getgroup(const SOCK_ADDR* endpoint);
ZION_API int rpcnet_register_subsys(unsigned char id, RPCNET_EVENT event);

ZION_API int rpcnet_ep2idx(const SOCK_ADDR* ep);

ZION_API int rpcnet_group_islocal(RPCNET_GROUP *group);
ZION_API const SOCK_ADDR* rpcnet_group_get_endpoint(RPCNET_GROUP* group, SOCK_ADDR* addr);
ZION_API void* rpcnet_group_get_subsysdata(RPCNET_GROUP* group, unsigned char id);            // thread unsafe
ZION_API void rpcnet_group_set_subsysdata(RPCNET_GROUP* group, unsigned char id, void* data); // thread unsafe

ZION_API RPCNET_THREAD_CONTEXT* rpcnet_context_get();
ZION_API void rpcnet_context_free(RPCNET_THREAD_CONTEXT* context);
ZION_API RPCNET_CONNECTION* rpcnet_context_getconn(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group);
ZION_API RPCNET_CONNECTION* rpcnet_context_getconn_force(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group);
ZION_API int rpcnet_context_chkconn(RPCNET_THREAD_CONTEXT* ctx, RPCNET_CONNECTION* conn);
ZION_API int rpcnet_context_freeconn(RPCNET_THREAD_CONTEXT* ctx, RPCNET_CONNECTION* conn);
ZION_API void rpcnet_context_closeconn(RPCNET_THREAD_CONTEXT* ctx, RPCNET_CONNECTION* conn);
ZION_API STREAM* rpcnet_context_getstream(RPCNET_THREAD_CONTEXT* context);

ZION_API int rpcnet_conn_write(RPCNET_CONNECTION* conn, unsigned char subsys, STREAM* stream);
ZION_API int rpcnet_conn_read(RPCNET_GROUP* group, RPCNET_CONNECTION* conn, unsigned char subsys, STREAM* stream);

ZION_API void* rpcnet_memory_alloc(RPCNET_THREAD_CONTEXT* context, int size);
ZION_API int rpcnet_memory_getbase(RPCNET_THREAD_CONTEXT* context); 
ZION_API void rpcnet_memory_setbase(RPCNET_THREAD_CONTEXT* context, int base);

#ifdef __cplusplus
}
#endif

#endif
