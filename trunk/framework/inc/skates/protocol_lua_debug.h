#ifndef __PROTOCOL_LUA_DEBUG_INCLUDE_
#define __PROTOCOL_LUA_DEBUG_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LUADEBUG_STATEINFO {
	unsigned int sid;
	char name[100];
	char client_ep[40];
	unsigned int cid;
} LUADEBUG_STATEINFO;

typedef struct LUADEBUG_CALLSTACK {
	char name[100];	/* (n) */
	char namewhat[100];	/* (n) `global', `local', `field', `method' */
	char what[100];	/* (S) `Lua', `C', `main', `tail' */
	char source[100];	/* (S) */
	int currentline;	/* (l) */
	int nups;		/* (u) number of upvalues */
	int linedefined;	/* (S) */
	int lastlinedefined;	/* (S) */
	char short_src[100]; /* (S) */
} LUADEBUG_CALLSTACK;

ZION_API void protocol_lua_init();
ZION_API void protocol_lua_final();

ZION_API int protocol_lua_state(LUADEBUG_STATEINFO* infos, int* count);

ZION_API lua_State* protocol_lua_newstate(lua_CFunction panic, const char* name);
ZION_API void protocol_lua_closestate(lua_State* L);

ZION_API void protocol_lua_debugbreak(lua_State* L);
ZION_API void protocol_lua_debugmsg(lua_State* L, int type, const char* msg);

#ifdef __cplusplus
}
#endif

#endif
