#include <string.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/protocol_def.h"
#include "../../inc/skates/protocol_lua.h"
#include "../../inc/skates/protocol.h"

//static int luaL_isstruct(lua_State* L, int idx, PROTOCOL_TYPE* type);
//static int luaL_isobject(lua_State* L, int idx, PROTOCOL_LUA_CLASS* cls);
//static PROTOCOL_LUA_OBJECT* luaL_tostruct(lua_State* L, int idx, PROTOCOL_TYPE* type);
//static PROTOCOL_LUA_OBJECT* luaL_toobject(lua_State* L, int idx, PROTOCOL_LUA_CLASS* cls);

static void struct_return(lua_State* L, PROTOCOL_TYPE* type, int n_var, int idx, void* ptr);
static void struct_make(lua_State* L, PROTOCOL_TYPE* type, int n_var, int idx, const void* ptr);
static int struct_get(lua_State* L);
static int struct_set(lua_State* L);
static int struct_gc(lua_State* L);
static int struct_tostring(lua_State* L);

int luaL_isstruct(lua_State* L, int idx, PROTOCOL_TYPE* type)
{
	PROTOCOL_LUA_OBJECT* obj;
	if(lua_type(L, idx)!=LUA_TUSERDATA) return 0;
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, idx);
	if(obj->type!=PROTOCOL_TYPE_STRUCT) return 0;
	if(obj->s.type!=type) return 0;
	return 1;
}

int luaL_isobject(lua_State* L, int idx, PROTOCOL_LUA_CLASS* cls)
{
	PROTOCOL_LUA_OBJECT* obj;
	if(lua_type(L, idx)!=LUA_TUSERDATA) return 0;
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, idx);
	if(obj->type!=PROTOCOL_TYPE_OBJECT) return 0;
	if(obj->o.t_class!=cls) return 0;
	return 1;
}

PROTOCOL_LUA_OBJECT* luaL_tostruct(lua_State* L, int idx, PROTOCOL_TYPE* type)
{
	PROTOCOL_LUA_OBJECT* obj;
	if(lua_type(L, idx)!=LUA_TUSERDATA) return NULL;
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, idx);
	if(obj->type!=PROTOCOL_TYPE_STRUCT) return NULL;
	if(obj->s.type!=type) return NULL;
	return obj;
}

PROTOCOL_LUA_OBJECT* luaL_toobject(lua_State* L, int idx, PROTOCOL_LUA_CLASS* cls)
{
	PROTOCOL_LUA_OBJECT* obj;
	if(lua_type(L, idx)!=LUA_TUSERDATA) return NULL;
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, idx);
	if(obj->type!=PROTOCOL_TYPE_OBJECT) return NULL;
	if(obj->o.t_class!=cls) return NULL;
	return obj;
}

void struct_return(lua_State* L, PROTOCOL_TYPE* type, int n_var, int idx, void* ptr)
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
    luaL_getmetatable(L, PROTOCOL_STRUCT_METATABLE);
    lua_setmetatable(L, -2);
	obj->type = PROTOCOL_TYPE_STRUCT;
	obj->s.type = type;
	obj->s.n_var = n_var;
	obj->s.idx = idx;
	obj->s.ptr = ptr;
}

void struct_make(lua_State* L, PROTOCOL_TYPE* type, int n_var, int idx, const void* ptr)
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
			return;
		}
		switch(v_type) {
		case PROTOCOL_TYPE_CHAR:
			*((os_char*)v_ptr) = (os_char)lua_tointeger(L, 3);
			return;
		case PROTOCOL_TYPE_SHORT:
			*((os_short*)v_ptr) = (os_short)lua_tointeger(L, 3);
			return;
		case PROTOCOL_TYPE_INT:
			*((os_int*)v_ptr) = (os_int)lua_tointeger(L, 3);
			return;
		case PROTOCOL_TYPE_LONG:
			*((os_long*)v_ptr) = (os_long)lua_tointeger(L, 3);
			return;
		case PROTOCOL_TYPE_BYTE:
			*((os_byte*)v_ptr) = (os_byte)lua_tointeger(L, 3);
			return;
		case PROTOCOL_TYPE_WORD:
			*((os_word*)v_ptr) = (os_word)lua_tointeger(L, 3);
			return;
		case PROTOCOL_TYPE_DWORD:
			*((os_dword*)v_ptr) = (os_dword)lua_tointeger(L, 3);
			return;
		case PROTOCOL_TYPE_QWORD:
			*((os_qword*)v_ptr) = (os_qword)lua_tointeger(L, 3);
			return;
		case PROTOCOL_TYPE_FLOAT:
			*((os_float*)v_ptr) = (os_float)lua_tonumber(L, 3);
			return;
		}
	}

	if(v_type==PROTOCOL_TYPE_STRING) {
		const char* value;
		if(lua_type(L, 3)!=LUA_TSTRING) {
			luaL_error(L, "invalid parameter type, array.\n");
			return;
		}
		value = lua_tolstring(L, 3, NULL);
		if(strlen(value)>=type->var_list[n_var].prelen) {
			luaL_error(L, "string so long.\n");
			return;
		}
		strcpy((char*)ptr, value);
		return;
	}

	luaL_error(L, "invalid parameter type, array.\n");
}

int struct_get(lua_State* L)
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

		if(obj->s.n_var<0 || !(obj->s.type->var_list[obj->s.n_var].type&PROTOCOL_TYPE_ARRAY) || obj->s.idx>=0) {
			luaL_error(L, "invalid parameter type, array.\n");
			return 0;
		}


		idx = lua_tointeger(L, 2);
		if(idx<0 || idx>=(int)obj->s.type->var_list[obj->s.n_var].maxlen) {
			lua_pushnil(L);
			return 1;
		}

		struct_return(L, obj->s.type, obj->s.n_var, idx, (char*)obj->s.ptr);
		return 1;
	}
	if(lua_type(L, 2)==LUA_TSTRING) {
		PROTOCOL_TYPE* type;
		unsigned int offset;
		const char* name;
		int i;
		name = lua_tolstring(L, 2, NULL);
		if(obj->s.n_var<0) {
			type = obj->s.type;
			offset = 0;
		} else {
			if(obj->s.idx<0 && (obj->s.type->var_list[obj->s.n_var].type&PROTOCOL_TYPE_ARRAY)) {
				luaL_error(L, "invalid parameter type, array.\n");
				return 0;
			}
			type = obj->s.type->var_list[obj->s.n_var].obj_type;
			offset = obj->s.type->var_list[obj->s.n_var].offset;
		}
		for(i=0; i<type->var_count; i++)  {
			if(strcmp(type->var_list[i].name, name)==0) break;
		}
		if(i==type->var_count) {
			lua_pushnil(L);
			return 1;
		}

		struct_return(L, type, i, obj->s.idx, (char*)obj->s.ptr + offset);
		return 1;
	}
	luaL_error(L, "invalid key type, array.\n");
	return 0;
}

int struct_set(lua_State* L)
{
	PROTOCOL_LUA_OBJECT* obj;

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

		if(obj->s.n_var<0 || !(obj->s.type->var_list[obj->s.n_var].type&PROTOCOL_TYPE_ARRAY) || obj->s.idx>=0) {
			luaL_error(L, "invalid parameter type, array.\n");
			return 0;
		}

		idx = lua_tointeger(L, 2);
		if(idx<0 || idx>=(int)obj->s.type->var_list[obj->s.n_var].maxlen) {
			luaL_error(L, "invalid array index.\n");
			return 0;
		}

		struct_make(L, obj->s.type, obj->s.n_var, idx, (char*)obj->s.ptr);
		return 0;
	}
	if(lua_type(L, 2)==LUA_TSTRING) {
		PROTOCOL_TYPE* type;
		unsigned int offset;
		const char* name;
		int i;
		name = lua_tolstring(L, 2, NULL);
		if(obj->s.n_var<0) {
			type = obj->s.type;
			offset = 0;
		} else {
			type = obj->s.type->var_list[obj->s.n_var].obj_type;
			offset = obj->s.type->var_list[obj->s.n_var].offset;
			if(obj->s.idx<0 && (obj->s.type->var_list[obj->s.n_var].type&PROTOCOL_TYPE_ARRAY)) {
				luaL_error(L, "invalid parameter type, array.\n");
				return 0;
			}
		}
		for(i=0; i<type->var_count; i++)  {
			if(strcmp(type->var_list[i].name, name)==0) break;
		}
		if(i==type->var_count) {
			luaL_error(L, "invalid array index.\n");
			return 0;
		}

		struct_make(L, type, i, obj->s.idx, (char*)obj->s.ptr + offset);
		return 0;
	}
	luaL_error(L, "invalid key type, array.\n");
	return 0;
}

int struct_gc(lua_State* L)
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
	return 0;
}

int struct_tostring(lua_State* L)
{
	PROTOCOL_LUA_OBJECT* obj;
	char buf[10*1024];
	unsigned int buf_len;
	int ret;

	if(lua_gettop(L)!=1) {
		luaL_error(L, "invalid parameter count.\n");
		return 0;
	}
	if(lua_type(L, 1)!=LUA_TUSERDATA) {
		luaL_error(L, "invalid parameter type.\n");
		return 0;
	}
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, 1);
	buf_len = sizeof(buf);
	ret = protocol_text_write(obj->s.type, obj->s.ptr, buf, &buf_len);
	if(ret!=ERR_NOERROR) {
		buf[0] = '\0';
		buf_len = 0;
	}
	lua_pushlstring(L, buf, buf_len);
	return 1;
}

void protocol_lua_create(lua_State* L, PROTOCOL_TYPE* type, void* ptr)
{
	struct_return(L, type, -1, -1, ptr); 
}

int protocol_lua_init(lua_State* L)
{
	int ret;
	ret = luaL_newmetatable(L, PROTOCOL_STRUCT_METATABLE);
	if(ret==0) return ERR_UNKNOWN;
	lua_pushvalue(L, -1);
	lua_pushstring(L, PROTOCOL_STRUCT_METATABLE);
	lua_settable(L, LUA_REGISTRYINDEX); /* reg[mt] = type_name */
	lua_pushstring(L,"__index");
	lua_pushcfunction(L, struct_get);
	lua_rawset(L,-3);
	lua_pushstring(L,"__newindex");
	lua_pushcfunction(L, struct_set);
	lua_rawset(L,-3);
	lua_pushstring(L,"__gc");
	lua_pushcfunction(L, struct_gc);
	lua_rawset(L,-3);
	lua_pushstring(L,"__tostring");
	lua_pushcfunction(L, struct_tostring);
	lua_rawset(L,-3);
	lua_pop(L,1);
	return ERR_NOERROR;
}
