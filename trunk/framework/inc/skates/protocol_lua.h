#ifndef __PROTOCOL_LUA_INCLUDE_
#define __PROTOCOL_LUA_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "lua/lua.h"
#include "lua/lauxlib.h"

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
};

struct PROTOCOL_LUA_CLASS {
	const char*					name;
	PROTOCOL_LUA_FUNCTION*		funcs;
	int							funcs_count;
};

ZION_API void protocol_lua_create(lua_State* L, PROTOCOL_TYPE* type, void* ptr);
ZION_API int protocol_lua_init(lua_State* L);

typedef struct PROTOCOL_LUA_OBJECT {
	int type;

	union {
		struct {
			PROTOCOL_LUA_CLASS*		t_class;
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

#ifdef __cplusplus
}
#endif

#endif
