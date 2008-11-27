#ifndef __PROTOCOL_LUA_INCLUDE_
#define __PROTOCOL_LUA_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"

struct PROTOCOL_LUA_PARAMETER;
typedef struct PROTOCOL_LUA_PARAMETER PROTOCOL_LUA_PARAMETER;
struct PROTOCOL_LUA_FUNCTION;
typedef struct PROTOCOL_LUA_FUNCTION PROTOCOL_LUA_FUNCTION;
struct PROTOCOL_LUA_CLASS;
typedef struct PROTOCOL_LUA_CLASS PROTOCOL_LUA_CLASS;

struct PROTOCOL_LUA_PARAMETER {
	int							i_type;
	const char*					s_type;
	PROTOCOL_TYPE*				p_type;
	PROTOCOL_LUA_CLASS*			c_type;
	const char*					name;
};

struct PROTOCOL_LUA_FUNCTION {
	PROTOCOL_LUA_PARAMETER*		ret;
	const char*					name;
	PROTOCOL_LUA_PARAMETER*		params;
	int							params_count;
	int (*lua_func)(lua_State* L);
};

struct PROTOCOL_LUA_CLASS {
	const char*					name;
	PROTOCOL_LUA_FUNCTION*		funcs;
	int							funcs_count;
};

ZION_API void protocol_lua_init();
ZION_API void protocol_lua_final();

ZION_API int protocol_lua_initstate(lua_State* L);
ZION_API lua_State* protocol_lua_newstate(lua_CFunction panic, const char* name);
ZION_API void protocol_lua_closestate(lua_State* L);

ZION_API void protocol_lua_newstruct(lua_State* L, PROTOCOL_TYPE* type, void* ptr);
ZION_API void protocol_lua_newobject(lua_State* L, PROTOCOL_LUA_CLASS* cls, void* ptr);

ZION_API int protocol_lua_getvalue(lua_State* L, int idx, PROTOCOL_LUA_PARAMETER* p, void* v);
ZION_API void protocol_lua_pushvalue(lua_State* L, PROTOCOL_LUA_PARAMETER* p, void* v);

ZION_API int luaL_isstruct(lua_State* L, int idx, PROTOCOL_TYPE* type);
ZION_API int luaL_isobject(lua_State* L, int idx, PROTOCOL_LUA_CLASS* cls);
ZION_API void* luaL_tostruct(lua_State* L, int idx, PROTOCOL_TYPE* type);
ZION_API void* luaL_toobject(lua_State* L, int idx, PROTOCOL_LUA_CLASS* cls);

typedef struct PROTOCOL_LUA_OBJECT {
	int type;

	union {
		struct {
			PROTOCOL_LUA_CLASS*		t_class;
			void*					ptr;
		} o;

		struct {
			PROTOCOL_TYPE*			type;
			int						n_var;
			int						idx;
			void*					ptr;
		} s;
	};

} PROTOCOL_LUA_OBJECT;

#define PROTOCOL_STRUCT_METATABLE		"protocol_struct_metatable"
#define PROTOCOL_OBJECT_METATABLE		"protocol_object_metatable"

struct PROTOCOL_LUA_CLIENT;
typedef struct PROTOCOL_LUA_CLIENT PROTOCOL_LUA_CLIENT;

typedef struct PROTOCOL_LUA_DEBUG_CALLBACK {
	void (*attach)(PROTOCOL_LUA_CLIENT* pClient);
	void (*detach)(PROTOCOL_LUA_CLIENT* pClient);
	void (*debugbreak)(PROTOCOL_LUA_CLIENT* pClient);
	void (*debugmsg)(PROTOCOL_LUA_CLIENT* pClient, int type, const char* msg);
	void* userptr;
} PROTOCOL_LUA_DEBUG_CALLBACK;

typedef struct LUADEBUG_STATEINFO {
	unsigned int sid;
	char name[100];	/* (n) */
	char client_ep[40];
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

ZION_API int protocol_lua_attach(RPCNET_GROUP* grp, unsigned int sid, PROTOCOL_LUA_DEBUG_CALLBACK* callback);
ZION_API int protocol_lua_detach(PROTOCOL_LUA_CLIENT* client);
ZION_API int protocol_lua_runcmd(PROTOCOL_LUA_CLIENT* client, const char* cmd);
ZION_API int protocol_lua_getstack(PROTOCOL_LUA_CLIENT* client, LUADEBUG_CALLSTACK* stack, int* count);

ZION_API void protocol_lua_debugbreak(lua_State* L);
ZION_API void protocol_lua_debugmsg(lua_State* L, int type, const char* msg);

#ifdef __cplusplus
}
#endif

#endif
