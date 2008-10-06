#ifndef __PROTOCOL_LUA_INCLUDE_
#define __PROTOCOL_LUA_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "lua/lua.h"
#include "lua/lauxlib.h"

typedef struct PROTOCOL_LUA_OBJECT {
	PROTOCOL_TYPE*			type;
	int						n_var;
	int						idx;
	void*					ptr;
} PROTOCOL_LUA_OBJECT;

ZION_API int protocol_lua_create(lua_State* L, PROTOCOL_TYPE* type, void* ptr);
ZION_API int protocol_lua_init(lua_State* L);

#define PROTOCOL_LUA_METATABLE		"protocol_lua_metatable"

#ifdef __cplusplus
}
#endif

#endif

