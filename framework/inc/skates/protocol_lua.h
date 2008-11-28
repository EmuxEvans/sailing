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

ZION_API int protocol_lua_initstate(lua_State* L);

ZION_API void protocol_lua_newstruct(lua_State* L, PROTOCOL_TYPE* type, void* ptr);
ZION_API void protocol_lua_newobject(lua_State* L, PROTOCOL_LUA_CLASS* cls, void* ptr);

ZION_API int protocol_lua_getvalue(lua_State* L, int idx, PROTOCOL_LUA_PARAMETER* p, void* v);
ZION_API void protocol_lua_pushvalue(lua_State* L, PROTOCOL_LUA_PARAMETER* p, void* v);

ZION_API int luaL_isstruct(lua_State* L, int idx, PROTOCOL_TYPE* type);
ZION_API int luaL_isobject(lua_State* L, int idx, PROTOCOL_LUA_CLASS* cls);
ZION_API void* luaL_tostruct(lua_State* L, int idx, PROTOCOL_TYPE* type);
ZION_API void* luaL_toobject(lua_State* L, int idx, PROTOCOL_LUA_CLASS* cls);
ZION_API PROTOCOL_TYPE* luaL_getstruct(lua_State* L, int idx);
ZION_API PROTOCOL_LUA_CLASS* luaL_getclass(lua_State* L, int idx);

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

#ifdef __cplusplus
}
#endif

#endif
