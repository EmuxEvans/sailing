#include <string.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/protocol_def.h"
#include "../../inc/skates/protocol_lua.h"

struct PROTOCOL_LUA_OBJECT;
typedef struct PROTOCOL_LUA_OBJECT PROTOCOL_LUA_OBJECT;;

struct PROTOCOL_LUA_OBJECT {
	PROTOCOL_TYPE*			type;
	int						n_var;
	int						idx;
	void*					ptr;
};

static void lua_return_value(lua_State* L, PROTOCOL_TYPE* type, int n_var, int idx, void* ptr);
static int lua_set_value(lua_State* L, PROTOCOL_TYPE* type, int n_var, int idx, const void* ptr);
static int protocol_lua_set(lua_State* L);
static int protocol_lua_gc(lua_State* L);

void lua_return_value(lua_State* L, PROTOCOL_TYPE* type, int n_var, int idx, void* ptr)
{
	PROTOCOL_LUA_OBJECT* obj;
	int v_type;
	void* v_ptr;

	if(idx<0) {
		v_type	= type->var_list[n_var].type;
		v_ptr	= (char*)ptr + type->var_list[n_var].offset;
	} else {
		v_type	= (type->var_list[n_var].type&0xff);
		v_ptr	= (char*)ptr + type->var_list[n_var].offset + type->var_list[n_var].prelen * idx;
	}

	switch(type->var_list[n_var].type&(idx<0?0x0fff:0xff)) {
	case PROTOCOL_TYPE_CHAR:
		lua_pushinteger(L, (lua_Integer)*((os_char*)v_ptr));
		return;
	case PROTOCOL_TYPE_SHORT:
		lua_pushinteger(L, (lua_Integer)*((os_short*)v_ptr));
		return;
	case PROTOCOL_TYPE_INT:
		lua_pushinteger(L, (lua_Integer)*((os_int*)v_ptr));
		return;
	case PROTOCOL_TYPE_LONG:
		lua_pushinteger(L, (lua_Integer)*((os_long*)v_ptr));
		return;
	case PROTOCOL_TYPE_BYTE:
		lua_pushinteger(L, (lua_Integer)*((os_byte*)v_ptr));
		return;
	case PROTOCOL_TYPE_WORD:
		lua_pushinteger(L, (lua_Integer)*((os_word*)v_ptr));
		return;
	case PROTOCOL_TYPE_DWORD:
		lua_pushinteger(L, (lua_Integer)*((os_dword*)v_ptr));
		return;
	case PROTOCOL_TYPE_QWORD:
		lua_pushinteger(L, (lua_Integer)*((os_qword*)v_ptr));
		return;
	case PROTOCOL_TYPE_FLOAT:
		lua_pushnumber(L, (lua_Number)*((os_float*)v_ptr));
		return;
	case PROTOCOL_TYPE_STRING:
		lua_pushlstring(L, (const char*)ptr, type->var_list[n_var].prelen);
		return;
	}

	obj = (PROTOCOL_LUA_OBJECT*)lua_newuserdata(L, sizeof(PROTOCOL_LUA_OBJECT));
    luaL_getmetatable(L, PROTOCOL_LUA_METATABLE);
    lua_setmetatable(L, -2);
	obj->type = type;
	obj->n_var = n_var;
	obj->idx = idx;
	obj->ptr = ptr;
}

int lua_set_value(lua_State* L, PROTOCOL_TYPE* type, int n_var, int idx, const void* ptr)
{
	int v_type;
	void* v_ptr;

	if(idx<0) {
		v_type	= type->var_list[n_var].type;
		v_ptr	= (char*)ptr + type->var_list[n_var].offset;
	} else {
		v_type	= (type->var_list[n_var].type&0xff);
		v_ptr	= (char*)ptr + type->var_list[n_var].offset + type->var_list[n_var].prelen * idx;
	}

	if(v_type>PROTOCOL_TYPE_CHAR && v_type<PROTOCOL_TYPE_FLOAT) {
		if(lua_type(L, 3)!=LUA_TNUMBER) {
			luaL_error(L, "invalid parameter type, array.\n");
			return 0;
		}
		switch(v_type) {
		case PROTOCOL_TYPE_CHAR:
			*((os_char*)v_ptr) = (os_char)lua_tointeger(L, 3);
			return 1;
		case PROTOCOL_TYPE_SHORT:
			*((os_short*)v_ptr) = (os_short)lua_tointeger(L, 3);
			return 1;
		case PROTOCOL_TYPE_INT:
			*((os_int*)v_ptr) = (os_int)lua_tointeger(L, 3);
			return 1;
		case PROTOCOL_TYPE_LONG:
			*((os_long*)v_ptr) = (os_long)lua_tointeger(L, 3);
			return 1;
		case PROTOCOL_TYPE_BYTE:
			*((os_byte*)v_ptr) = (os_byte)lua_tointeger(L, 3);
			return 1;
		case PROTOCOL_TYPE_WORD:
			*((os_word*)v_ptr) = (os_word)lua_tointeger(L, 3);
			return 1;
		case PROTOCOL_TYPE_DWORD:
			*((os_dword*)v_ptr) = (os_dword)lua_tointeger(L, 3);
			return 1;
		case PROTOCOL_TYPE_QWORD:
			*((os_qword*)v_ptr) = (os_qword)lua_tointeger(L, 3);
			return 1;
		case PROTOCOL_TYPE_FLOAT:
			*((os_float*)v_ptr) = (os_float)lua_tonumber(L, 3);
			return 1;
		}
	}

	if(v_type==PROTOCOL_TYPE_STRING) {
		const char* value;
		if(lua_type(L, 3)!=LUA_TSTRING) {
			luaL_error(L, "invalid parameter type, array.\n");
			return 0;
		}
		value = lua_tolstring(L, 3, NULL);
		if(strlen(value)>=type->var_list[n_var].prelen) {
			luaL_error(L, "string so long.\n");
			return 0;
		}
		strcpy((char*)ptr, value);
		return 1;
	}

	luaL_error(L, "invalid parameter type, array.\n");
	return 0;
}

static int protocol_lua_get(lua_State* L)
{
	PROTOCOL_LUA_OBJECT* obj;
	if(lua_gettop(L)!=2) {
		luaL_error(L, "invalid parameter count.\n");
		return 0;
	}
	if(lua_type(L, 1)!=LUA_TUSERDATA) {
		luaL_error(L, "invalid parameter type.\n");
		return 0;
	}
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, 1);
	if(lua_type(L, 2)==LUA_TNUMBER) {
		int idx;

		if(obj->n_var<0 || !(obj->type->var_list[obj->n_var].type&PROTOCOL_TYPE_ARRAY) || obj->idx>=0) {
			luaL_error(L, "invalid parameter type, array.\n");
			return 0;
		}


		idx = lua_tointeger(L, 2);
		if(idx<0 || idx>=(int)obj->type->var_list[obj->n_var].maxlen) {
			lua_pushnil(L);
			return 1;
		}

		lua_return_value(L, obj->type, obj->n_var, idx, (char*)obj->ptr);

		return 1;
	}
	if(lua_type(L, 2)==LUA_TSTRING) {
		PROTOCOL_TYPE* type;
		unsigned int offset;
		const char* name;
		int i;
		name = lua_tolstring(L, 2, NULL);
		if(obj->n_var<0) {
			type = obj->type;
			offset = 0;
		} else {
			if(obj->idx<0 && (obj->type->var_list[obj->n_var].type&PROTOCOL_TYPE_ARRAY)) {
				luaL_error(L, "invalid parameter type, array.\n");
				return 0;
			}
			type = obj->type->var_list[obj->n_var].obj_type;
			offset = obj->type->var_list[obj->n_var].offset;
		}
		for(i=0; i<type->var_count; i++)  {
			if(strcmp(type->var_list[i].name, name)==0) break;
		}
		if(i==type->var_count) {
			lua_pushnil(L);
			return 1;
		}

		lua_return_value(L, type, i, obj->idx, (char*)obj->ptr + offset);

		return 1;
	}
	luaL_error(L, "invalid key type, array.\n");
	return 0;
}

int protocol_lua_set(lua_State* L)
{
	PROTOCOL_LUA_OBJECT* obj;
	int ret;
	ret = lua_gettop(L);
	if(lua_gettop(L)!=3) {
		luaL_error(L, "invalid parameter count.\n");
		return 0;
	}
	if(lua_type(L, 1)!=LUA_TUSERDATA) {
		luaL_error(L, "invalid parameter type.\n");
		return 0;
	}
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, 1);
	if(lua_type(L, 2)==LUA_TNUMBER) {
		int idx;

		if(obj->n_var<0 || !(obj->type->var_list[obj->n_var].type&PROTOCOL_TYPE_ARRAY) || obj->idx>=0) {
			luaL_error(L, "invalid parameter type, array.\n");
			return 0;
		}

		idx = lua_tointeger(L, 2);
		if(idx<0 || idx>=(int)obj->type->var_list[obj->n_var].maxlen) {
			luaL_error(L, "invalid array index.\n");
			return 0;
		}

		ret = lua_set_value(L, obj->type, obj->n_var, idx, (char*)obj->ptr);
		if(ret<0) return ret;
		return 1;
	}
	if(lua_type(L, 2)==LUA_TSTRING) {
		PROTOCOL_TYPE* type;
		unsigned int offset;
		const char* name;
		int i;
		name = lua_tolstring(L, 2, NULL);
		if(obj->n_var<0) {
			type = obj->type;
			offset = 0;
		} else {
			type = obj->type->var_list[obj->n_var].obj_type;
			offset = obj->type->var_list[obj->n_var].offset;
			if(obj->idx<0 && (obj->type->var_list[obj->n_var].type&PROTOCOL_TYPE_ARRAY)) {
				luaL_error(L, "invalid parameter type, array.\n");
				return 0;
			}
		}
		for(i=0; i<type->var_count; i++)  {
			if(strcmp(type->var_list[i].name, name)==0) break;
		}
		if(i==type->var_count) {
			lua_pushnil(L);
			return 1;
		}

		ret = lua_set_value(L, type, i, obj->idx, (char*)obj->ptr + offset);

		return 1;
	}
	luaL_error(L, "invalid key type, array.\n");
	return 1;
}

int protocol_lua_gc(lua_State* L)
{
	PROTOCOL_LUA_OBJECT* obj;
	if(lua_gettop(L)!=1) {
		luaL_error(L, "invalid parameter count.\n");
		return 0;
	}
	if(lua_type(L, 1)!=LUA_TUSERDATA) {
		luaL_error(L, "invalid parameter type.\n");
		return 0;
	}
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, 1);
	return 1;
}

int protocol_lua_create(lua_State* L, PROTOCOL_TYPE* type, void* ptr)
{
	lua_return_value(L, type, -1, -1, ptr); 
	return 1;
}

int protocol_lua_init(lua_State* L)
{
	int ret;
	ret = luaL_newmetatable(L, PROTOCOL_LUA_METATABLE);
	if(ret==0) return ERR_UNKNOWN;
	lua_pushvalue(L, -1);
	lua_pushstring(L, PROTOCOL_LUA_METATABLE);
	lua_settable(L, LUA_REGISTRYINDEX); /* reg[mt] = type_name */
	lua_pushstring(L,"__index");
	lua_pushcfunction(L, protocol_lua_get);
	lua_rawset(L,-3);
	lua_pushstring(L,"__newindex");
	lua_pushcfunction(L, protocol_lua_set);
	lua_rawset(L,-3);
	lua_pushstring(L,"__gc");
	lua_pushcfunction(L, protocol_lua_gc);
	lua_rawset(L,-3);
	lua_pop(L,1);
	return ERR_NOERROR;
}
