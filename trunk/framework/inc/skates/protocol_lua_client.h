#ifndef __PROTOCOL_LUA_CLIENT_INCLUDE_
#define __PROTOCOL_LUA_CLIENT_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

struct PROTOCOL_LUA_CLIENT;
typedef struct PROTOCOL_LUA_CLIENT PROTOCOL_LUA_CLIENT;

typedef struct PROTOCOL_LUA_DEBUG_CALLBACK {
	void (*attach)(PROTOCOL_LUA_CLIENT* pClient);
	void (*detach)(PROTOCOL_LUA_CLIENT* pClient);
	void (*debugbreak)(PROTOCOL_LUA_CLIENT* pClient);
	void (*debugmsg)(PROTOCOL_LUA_CLIENT* pClient, int type, const char* msg);
	void* userptr;
} PROTOCOL_LUA_DEBUG_CALLBACK;

void protocol_luaclt_init();
void protocol_luaclt_final();

ZION_API int protocol_luaclt_state(RPCNET_GROUP* host, LUADEBUG_STATEINFO* infos, int* count);

ZION_API PROTOCOL_LUA_CLIENT* protocol_luaclt_attach(RPCNET_GROUP* grp, unsigned int sid, PROTOCOL_LUA_DEBUG_CALLBACK* callback);
ZION_API void protocol_luaclt_detach(PROTOCOL_LUA_CLIENT* client);
ZION_API void* protocol_luaclt_userptr(PROTOCOL_LUA_CLIENT* client);

ZION_API int protocol_luaclt_runcmd(PROTOCOL_LUA_CLIENT* client, const char* cmd);
ZION_API int protocol_luaclt_getstack(PROTOCOL_LUA_CLIENT* client, LUADEBUG_CALLSTACK* stack, int* count);

#ifdef __cplusplus
}
#endif

#endif
