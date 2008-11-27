#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/dymempool.h"
#include "../../inc/skates/applog.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/sock.h"
#include "../../inc/skates/stream.h"
#include "../../inc/skates/hashmap.h"
#include "../../inc/skates/rpc_net.h"
#include "../../inc/skates/rpc_fun.h"
#include "../../inc/skates/protocol_def.h"
#include "../../inc/skates/protocol_lua.h"
#include "../../inc/skates/protocol.h"
#include "../../inc/skates/threadpool.h"
#include "protocol_lua.h"

//static int luaL_isstruct(lua_State* L, int idx, PROTOCOL_TYPE* type);
//static int luaL_isobject(lua_State* L, int idx, PROTOCOL_LUA_CLASS* cls);
//static PROTOCOL_LUA_OBJECT* luaL_tostruct(lua_State* L, int idx, PROTOCOL_TYPE* type);
//static PROTOCOL_LUA_OBJECT* luaL_toobject(lua_State* L, int idx, PROTOCOL_LUA_CLASS* cls);

struct PROTOCOL_LUA_CLIENT {
	RPCNET_GROUP* host;
	unsigned int sid;
	unsigned int cid;

	PROTOCOL_LUA_DEBUG_CALLBACK callback;
};

typedef struct PROTOCOL_LUA_STATE{
	unsigned int	sid;
	char			name[100];
	lua_State*		L;

	RPCNET_GROUP*	host;
	unsigned int	cid;

	RPCNET_GROUP*	debug_host;
	unsigned int	debug_cid;
	lua_State*		dbgL;
} PROTOCOL_LUA_STATE;

static void struct_return(lua_State* L, PROTOCOL_TYPE* type, int n_var, int idx, void* ptr);
static void struct_make(lua_State* L, PROTOCOL_TYPE* type, int n_var, int idx, const void* ptr);
static int struct_get(lua_State* L);
static int struct_set(lua_State* L);
static int struct_gc(lua_State* L);
static int struct_tostring(lua_State* L);

static int object_get(lua_State* L);
static int object_gc(lua_State* L);

PROTOCOL_LUA_STATE state_list[20];
unsigned int state_seq = 0x2008;
PROTOCOL_LUA_CLIENT client_list[100];

static PROTOCOL_LUA_CLIENT* client_get(unsigned int cid);
static PROTOCOL_LUA_STATE* state_get(unsigned int sid);

int luaL_isstruct(lua_State* L, int idx, PROTOCOL_TYPE* type)
{
	PROTOCOL_LUA_OBJECT* obj;
	if(lua_type(L, idx)!=LUA_TUSERDATA) return 0;
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, idx);
	if(obj->type!=PROTOCOL_TYPE_STRUCT) return 0;
	if(obj->s.n_var<0) {
		return obj->s.type==type;
	}
	if(obj->s.type!=type) return 0;
	if(obj->s.type->var_list[obj->s.n_var].type&PROTOCOL_TYPE_ARRAY) {
		return obj->s.idx>=0;
	} else {
		return 0;
	}
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

void* luaL_tostruct(lua_State* L, int idx, PROTOCOL_TYPE* type)
{
	PROTOCOL_LUA_OBJECT* obj;
	if(lua_type(L, idx)!=LUA_TUSERDATA) return NULL;
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, idx);
	if(obj->type!=PROTOCOL_TYPE_STRUCT) return NULL;
	if(obj->s.n_var<0) {
		if(obj->s.type==type) {
			return obj->s.ptr;
		} else {
			return NULL;
		}
	}
	if(obj->s.type!=type) return NULL;
	if(obj->s.type->var_list[obj->s.n_var].type&PROTOCOL_TYPE_ARRAY) {
		return (char*)obj->s.ptr + obj->s.type->var_list[obj->s.n_var].offset + obj->s.type->var_list[obj->s.n_var].prelen * obj->s.idx;
	} else {
		return (char*)obj->s.ptr + obj->s.type->var_list[obj->s.n_var].offset;
	}
}

void* luaL_toobject(lua_State* L, int idx, PROTOCOL_LUA_CLASS* cls)
{
	PROTOCOL_LUA_OBJECT* obj;
	if(lua_type(L, idx)!=LUA_TUSERDATA) return NULL;
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, idx);
	if(obj->type!=PROTOCOL_TYPE_OBJECT) return NULL;
	if(obj->o.t_class!=cls) return NULL;
	return obj->o.ptr;
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
	if(lua_isuserdata(L, 1)) {
		luaL_error(L, "invalid parameter type.\n");
		return 0;
	}
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, 1);
	if(lua_isnumber(L, 2)) {
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
	if(lua_isstring(L, 2)) {
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

int object_get(lua_State* L)
{
	PROTOCOL_LUA_OBJECT* obj;
	const char* name;
	int i;

	if(lua_gettop(L)!=2) {
		luaL_error(L, "invalid parameter count.\n");
		return 0;
	}
	if(lua_type(L, 1)!=LUA_TUSERDATA) {
		luaL_error(L, "invalid parameter type.\n");
		return 0;
	}
	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, 1);
	if(!lua_isstring(L, 2)) {
		luaL_error(L, "invalid parameter type.\n");
		return 0;
	}
	name = lua_tolstring(L, 2, NULL);
	for(i=0; i<obj->o.t_class->funcs_count; i++) {
		if(strcmp(obj->o.t_class->funcs[i].name, name)==0) {
			lua_pushcfunction(L, obj->o.t_class->funcs[i].lua_func);
			return 1;
		}
	}

	luaL_error(L, "invalid function.\n");
	return 0;
}

int object_gc(lua_State* L)
{
	return 1;
}

void protocol_lua_init()
{
	memset(state_list, 0, sizeof(state_list));
	memset(client_list, 0, sizeof(client_list));
	rpcfun_register(__protocol_lua_desc, 0);
}

void protocol_lua_final()
{
	int idx;

	rpcfun_unregister(__protocol_lua_desc, 0);

	for(idx=0; idx<sizeof(state_list)/sizeof(state_list[0]); idx++) {
		if(!state_list[idx].L) continue;
		SYSLOG(LOG_WARNING, MODULE_NAME, "protocol_lua_final: state(%s) not free", state_list[idx].name);
	}
	for(idx=0; idx<sizeof(client_list)/sizeof(client_list[0]); idx++) {
		if(!client_list[idx].host) continue;
		SYSLOG(LOG_WARNING, MODULE_NAME, "protocol_lua_final: client(%d) not detach", idx);
	}
}

int protocol_lua_initstate(lua_State* L)
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

	ret = luaL_newmetatable(L, PROTOCOL_OBJECT_METATABLE);
	if(ret==0) return ERR_UNKNOWN;
	lua_pushvalue(L, -1);
	lua_pushstring(L, PROTOCOL_OBJECT_METATABLE);
	lua_settable(L, LUA_REGISTRYINDEX); /* reg[mt] = type_name */
	lua_pushstring(L,"__index");
	lua_pushcfunction(L, object_get);
	lua_rawset(L,-3);
	lua_pushstring(L,"__gc");
	lua_pushcfunction(L, object_gc);
	lua_rawset(L,-3);
	lua_pop(L,1);

	
	return ERR_NOERROR;
}

static void* protocol_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	(void)ud;
	(void)osize;
	if(nsize==0) {
		dymempool_free(ptr);
		return NULL;
	} else
		return dymempool_realloc(ptr, nsize);
}

static int protocol_panic (lua_State *L) {
  (void)L;  /* to avoid warnings */
  fprintf(stderr, "PANIC: unprotected error in call to Lua API (%s)\n",
                   lua_tostring(L, -1));
  return 0;
}

lua_State* protocol_lua_newstate(lua_CFunction panic, const char* name)
{
	unsigned int idx;
	lua_State* L;

	for(idx=0; idx<sizeof(state_list)/sizeof(state_list[0]); idx++) {
		if(!state_list[idx].L) break;
	}
	if(idx==sizeof(state_list)/sizeof(state_list[0])) return NULL;

	L = lua_newstate(protocol_alloc, NULL);
	if(!L) return NULL;

	lua_atpanic(L, panic?panic:protocol_panic);
	protocol_lua_initstate(L);

	state_list[idx].sid		= (((state_seq++)&0x7fff)<<16) | idx;
	state_list[idx].L		= L;
	strcpy(state_list[idx].name, name);
	state_list[idx].host	= NULL;
	state_list[idx].cid		= 0;
	state_list[idx].debug_host	= NULL;
	state_list[idx].debug_cid	= 0;
	state_list[idx].dbgL		= NULL;

	lua_pushstring(L, "PROTOCOL_LUA_ID");
	lua_pushinteger(L, (lua_Integer)state_list[idx].sid);
	lua_rawset(L, LUA_REGISTRYINDEX);

	return L;
}

void protocol_lua_closestate(lua_State* L)
{
	unsigned int idx;

	lua_pushstring(L, "PROTOCOL_LUA_ID");
	lua_rawget(L, LUA_REGISTRYINDEX);
	if(!lua_isnumber(L, -1)) {
		lua_pop(L, -1);
		return;
	}
	idx = lua_tointeger(L, -1);
	lua_pop(L, -1);
	assert((idx&0xffff)>=0 && (idx&0xffff)<sizeof(state_list)/sizeof(state_list[0]));
	assert(state_list[idx&0xffff].sid==idx);
	assert(state_list[idx&0xffff].L==L);

	lua_close(L);
	memset(&state_list[idx&0xffff], 0, sizeof(state_list[0]));
}

void protocol_lua_newstruct(lua_State* L, PROTOCOL_TYPE* type, void* ptr)
{
	struct_return(L, type, -1, -1, ptr); 
}

void protocol_lua_newobject(lua_State* L, PROTOCOL_LUA_CLASS* cls, void* ptr)
{
	PROTOCOL_LUA_OBJECT* obj;
	obj = (PROTOCOL_LUA_OBJECT*)lua_newuserdata(L, sizeof(PROTOCOL_LUA_OBJECT));
	luaL_getmetatable(L, PROTOCOL_OBJECT_METATABLE);
	lua_setmetatable(L, -2);
	obj->type = PROTOCOL_TYPE_OBJECT;
	obj->o.t_class = cls;
	obj->o.ptr = ptr;
}

int protocol_lua_getvalue(lua_State* L, int idx, PROTOCOL_LUA_PARAMETER* p, void* v)
{
	switch(p->i_type) {
	case PROTOCOL_TYPE_CHAR:
		if(!lua_isnumber(L, idx)) return ERR_UNKNOWN;
		*((os_char*)v) = (os_char)lua_tointeger(L, idx);
		return ERR_NOERROR;
	case PROTOCOL_TYPE_SHORT:
		if(!lua_isnumber(L, idx)) return ERR_UNKNOWN;
		*((os_short*)v) = (os_short)lua_tointeger(L, idx);
		return ERR_NOERROR;
	case PROTOCOL_TYPE_INT:
		if(!lua_isnumber(L, idx)) return ERR_UNKNOWN;
		*((os_int*)v) = (os_int)lua_tointeger(L, idx);
		return ERR_NOERROR;
	case PROTOCOL_TYPE_LONG:
		if(!lua_isnumber(L, idx)) return ERR_UNKNOWN;
		*((os_long*)v) = (os_long)lua_tointeger(L, idx);
		return ERR_NOERROR;
	case PROTOCOL_TYPE_BYTE:
		if(!lua_isnumber(L, idx)) return ERR_UNKNOWN;
		*((os_byte*)v) = (os_byte)lua_tointeger(L, idx);
		return ERR_NOERROR;
	case PROTOCOL_TYPE_WORD:
		if(!lua_isnumber(L, idx)) return ERR_UNKNOWN;
		*((os_word*)v) = (os_word)lua_tointeger(L, idx);
		return ERR_NOERROR;
	case PROTOCOL_TYPE_DWORD:
		if(!lua_isnumber(L, idx)) return ERR_UNKNOWN;
		*((os_dword*)v) = (os_dword)lua_tointeger(L, idx);
		return ERR_NOERROR;
	case PROTOCOL_TYPE_QWORD:
		if(!lua_isnumber(L, idx)) return ERR_UNKNOWN;
		*((os_qword*)v) = (os_qword)lua_tointeger(L, idx);
		return ERR_NOERROR;
	case PROTOCOL_TYPE_FLOAT:
		if(!lua_isnumber(L, idx)) return ERR_UNKNOWN;
		*((os_float*)v) = (os_float)lua_tonumber(L, idx);
		return ERR_NOERROR;
	case PROTOCOL_TYPE_STRING:
		if(!lua_isstring(L, idx)) return ERR_UNKNOWN;
		*((char**)v) = (char*)lua_tolstring(L, idx, NULL);
		return ERR_NOERROR;
	case PROTOCOL_TYPE_STRUCT:
		if(!luaL_isstruct(L, idx, p->p_type)) return ERR_UNKNOWN;
		*((void**)v) = luaL_tostruct(L, idx, p->p_type);
		return ERR_NOERROR;
	case PROTOCOL_TYPE_OBJECT:
		if(!luaL_isobject(L, idx, p->c_type)) return ERR_UNKNOWN;
		*((void**)v) = luaL_toobject(L, idx, p->c_type);
		return ERR_UNKNOWN;
	}

	return ERR_UNKNOWN;
}

void protocol_lua_pushvalue(lua_State* L, PROTOCOL_LUA_PARAMETER* p, void* v)
{
	switch(p->i_type) {
	case PROTOCOL_TYPE_CHAR:
		lua_pushinteger(L, (lua_Integer)*((os_char*)v));
		return;
	case PROTOCOL_TYPE_SHORT:
		lua_pushinteger(L, (lua_Integer)*((os_short*)v));
		return;
	case PROTOCOL_TYPE_INT:
		lua_pushinteger(L, (lua_Integer)*((os_int*)v));
		return;
	case PROTOCOL_TYPE_LONG:
		lua_pushinteger(L, (lua_Integer)*((os_long*)v));
		return;
	case PROTOCOL_TYPE_BYTE:
		lua_pushinteger(L, (lua_Integer)*((os_byte*)v));
		return;
	case PROTOCOL_TYPE_WORD:
		lua_pushinteger(L, (lua_Integer)*((os_word*)v));
		return;
	case PROTOCOL_TYPE_DWORD:
		lua_pushinteger(L, (lua_Integer)*((os_dword*)v));
		return;
	case PROTOCOL_TYPE_QWORD:
		lua_pushinteger(L, (lua_Integer)*((os_qword*)v));
		return;
	case PROTOCOL_TYPE_FLOAT:
		lua_pushnumber(L, (lua_Number)*((os_float*)v));
		return;
	case PROTOCOL_TYPE_STRING:
		lua_pushstring(L, *((char**)v));
		return;
	case PROTOCOL_TYPE_STRUCT:
		struct_return(L, p->p_type, -1, -1, *((void**)v));
		return;
	case PROTOCOL_TYPE_OBJECT:
		{
			PROTOCOL_LUA_OBJECT* obj;
			obj = (PROTOCOL_LUA_OBJECT*)lua_newuserdata(L, sizeof(PROTOCOL_LUA_OBJECT));
			luaL_getmetatable(L, PROTOCOL_STRUCT_METATABLE);
			lua_setmetatable(L, -2);
			obj->type = PROTOCOL_TYPE_OBJECT;
			obj->o.t_class = p->c_type;
			obj->o.ptr = *((void**)v);
			return;
		}
	}
}

int LuaDebugClientRpc_Attach_impl(RPCNET_GROUP* group, os_dword cid, os_dword sid)
{
	PROTOCOL_LUA_CLIENT* clt;
	clt = client_get(cid);
	if(!clt) return ERR_NOT_FOUND;

	clt->callback.attach(clt);

	return ERR_NOERROR;
}

int LuaDebugClientRpc_Detach_impl(RPCNET_GROUP* group, os_dword cid, os_dword sid)
{
	PROTOCOL_LUA_CLIENT* clt;
	clt = client_get(cid);
	if(!clt) return ERR_NOT_FOUND;

	clt->callback.detach(clt);
	clt->host = NULL;
	clt->sid = 0;
	clt->cid = 0;

	return ERR_NOERROR;
}

int LuaDebugClientRpc_BreakPoint_impl(RPCNET_GROUP* group, os_dword cid)
{
	PROTOCOL_LUA_CLIENT* clt;
	clt = client_get(cid);
	if(!clt) return ERR_NOT_FOUND;

	clt->callback.debugbreak(clt);

	return ERR_NOERROR;
}

int LuaDebugClientRpc_DebugMsg_impl(RPCNET_GROUP* group, os_dword cid, int Type, const char* Msg)
{
	PROTOCOL_LUA_CLIENT* clt;
	clt = client_get(cid);
	if(!clt) return ERR_NOT_FOUND;

	clt->callback.debugmsg(clt, Type, Msg);

	return ERR_NOERROR;
}

int LuaDebugHostRpc_Attach_impl(RPCNET_GROUP* group, os_dword sid, os_dword cid)
{
	PROTOCOL_LUA_STATE* state;
	int ret;

	state = state_get(sid);
	if(!state) return ERR_NOT_FOUND;

	state->host = group;
	state->cid = cid;

	ret = LuaDebugClientRpc_Attach(group, cid, sid);
	if(ret!=ERR_NOERROR) {
		state->host = NULL;
		state->cid = 0;
	}

	return ret;
}

int LuaDebugHostRpc_Detach_impl(RPCNET_GROUP* group, os_dword sid, os_dword cid)
{
	PROTOCOL_LUA_STATE* state;
	int ret;

	state = state_get(sid);
	if(!state) return ERR_NOT_FOUND;

	state->host = NULL;
	state->cid = 0;
	ret = LuaDebugClientRpc_Detach(group, cid, sid);

	return ret;
}

int LuaDebugHostRpc_GetStateList_impl(RPCNET_GROUP* group, LUADEBUG_STATEINFO* infos, int* count)
{
	int i, m;

	if(*count>sizeof(state_list)/sizeof(state_list[0])) {
		*count = sizeof(state_list)/sizeof(state_list[0]);
	}

	for(m=i=0; i<*count; i++) {
		RPCNET_GROUP* grp;

		if(!state_list[i].L) continue;
		grp = state_list[i].host;

		infos[m].sid = state_list[i].sid;
		strcpy(infos[m].name, state_list[i].name);
		if(grp) {
			SOCK_ADDR sa;
			sock_addr2str(rpcnet_group_get_endpoint(grp, &sa), infos[m].client_ep);
		} else {
			infos[m].client_ep[0] = '\0';
		}
		m++;
	}

	*count = m;
	return ERR_NOERROR;
}

int LuaDebugHostRpc_GetCallStack_impl(RPCNET_GROUP* group, os_dword sid, LUADEBUG_CALLSTACK* stacks, int* depth)
{
	PROTOCOL_LUA_STATE* state;
	int i;
	lua_Debug ar;

	state = state_get(sid);
	if(!state) return ERR_NOT_FOUND;
	if(!state->dbgL) return ERR_OPT_FORBIDDEN;

	for(i=1; i<*depth; i++) {
		memset(&ar, 0, sizeof(ar));
		if(!lua_getstack(state->dbgL, i, &ar)) break;
		if(!lua_getinfo(state->dbgL, "flnSu", &ar)) {
			assert(0);
			break;
		}

		if(ar.name) strcpy(stacks[i-1].name, ar.name);				else strcpy(stacks[i-1].name, "");
		if(ar.namewhat) strcpy(stacks[i-1].namewhat, ar.namewhat);	else strcpy(stacks[i-1].namewhat, "");
		if(ar.what) strcpy(stacks[i-1].what, ar.what);				else strcpy(stacks[i-1].what, "");
		if(ar.source) strcpy(stacks[i-1].source, ar.source);		else strcpy(stacks[i-1].source, "");
		stacks[i-1].currentline = ar.currentline;
		stacks[i-1].nups = ar.nups;
		stacks[i-1].linedefined = ar.linedefined;
		stacks[i-1].lastlinedefined = ar.lastlinedefined;
		strcpy(stacks[i-1].short_src, ar.short_src);
	}

	*depth = i - 1;
	return ERR_NOERROR;
}

int LuaDebugHostRpc_RunCmd_impl(RPCNET_GROUP* group, os_dword sid, const char* Cmd)
{
	PROTOCOL_LUA_STATE* state;
	int ret;

	state = state_get(sid);
	if(!state) return ERR_NOT_FOUND;
	if(!state->dbgL) return ERR_OPT_FORBIDDEN;

	ret = luaL_dostring(state->dbgL, Cmd);
	if(!ret) {
		lua_pop(state->dbgL, 1);
		return ERR_UNKNOWN;
	} else {
		return ERR_NOERROR;
	}
}

int protocol_lua_attach(RPCNET_GROUP* grp, unsigned int sid, PROTOCOL_LUA_DEBUG_CALLBACK* callback)
{
	unsigned int cid;
	int ret;

	if(grp==NULL) return ERR_INVALID_PARAMETER;

	for(cid=0; cid<sizeof(client_list)/sizeof(client_list[0]); cid++) {
		if(!client_list[cid].host) break;
	}
	if(cid==sizeof(client_list)/sizeof(client_list[0])) return ERR_FULL;

	client_list[cid].host = grp;
	cid = ((state_seq++)<<16) | cid;

	threadpool_s();
	ret = LuaDebugHostRpc_Attach(grp, sid, cid);
	threadpool_e();

	if(ret==ERR_NOERROR) {
		assert(client_list[cid&0xffff].host==grp);
		assert(client_list[cid&0xffff].sid==sid);
		assert(client_list[cid&0xffff].cid==cid);
		return ERR_NOERROR;
	} else {
		client_list[cid&0xffff].host = NULL;
		client_list[cid&0xffff].sid = 0;
		client_list[cid&0xffff].cid = 0;
		memcpy(&client_list[cid&0xffff].callback, callback, sizeof(*callback));
		return ret;
	}
}

int protocol_lua_detach(PROTOCOL_LUA_CLIENT* client)
{
	int ret;

	assert(client->host);

	threadpool_s();
	ret = LuaDebugHostRpc_Detach(client->host, client->sid, client->cid);
	threadpool_e();

	if(ret==ERR_NOERROR) {
		assert(client->host==NULL);
		assert(client->sid==0);
		assert(client->cid==0);
	} else {
		client->host = NULL;
		client->sid = 0;
		client->cid = 0;
	}

	return ret;
}

int protocol_lua_runcmd(PROTOCOL_LUA_CLIENT* client, const char* cmd)
{
	int ret;

	assert(client->host!=NULL);
	//assert(client->sid==0);
	//assert(client->cid==0);

	threadpool_s();
	ret = LuaDebugHostRpc_RunCmd(client->host, client->sid, cmd);
	threadpool_e();

	return ret;
}

int protocol_lua_getstack(PROTOCOL_LUA_CLIENT* client, LUADEBUG_CALLSTACK* stack, int* count)
{
	return 0;
}

void protocol_lua_debugbreak(lua_State* L)
{
	unsigned int idx;

	lua_pushstring(L, "PROTOCOL_LUA_ID");
	lua_rawget(L, LUA_REGISTRYINDEX);
	if(!lua_isnumber(L, -1)) {
		lua_pop(L, -1);
		return;
	}

	idx = lua_tointeger(L, -1);
	lua_pop(L, -1);
	assert((idx&0xffff)<sizeof(state_list)/sizeof(state_list[0]));
	assert(state_list[idx&0xffff].sid==idx);
	idx = idx & 0xffff;

	state_list[idx].debug_host = state_list[idx].host;
	if(state_list[idx].debug_host) {
		state_list[idx].debug_cid = state_list[idx].cid;
		state_list[idx].dbgL = L;

		threadpool_s();
		LuaDebugClientRpc_BreakPoint(state_list[idx].debug_host, state_list[idx].cid);
		threadpool_e();
	}
}

void protocol_lua_debugmsg(lua_State* L, int type, const char* msg)
{
	unsigned int idx;

	lua_pushstring(L, "PROTOCOL_LUA_ID");
	lua_rawget(L, LUA_REGISTRYINDEX);
	if(!lua_isnumber(L, -1)) {
		lua_pop(L, -1);
		return;
	}
	idx = lua_tointeger(L, -1);
	lua_pop(L, -1);

	assert((idx&0xffff)<sizeof(state_list)/sizeof(state_list[0]));
	assert(state_list[idx&0xffff].sid==idx);
	idx = idx & 0xffff;

	state_list[idx].debug_host = state_list[idx].host;
	if(state_list[idx].debug_host) {
		state_list[idx].debug_cid = state_list[idx].cid;
		state_list[idx].dbgL = L;

		threadpool_s();
		LuaDebugClientRpc_DebugMsg(state_list[idx].debug_host, state_list[idx].cid, type, msg);
		threadpool_e();
	}
}

PROTOCOL_LUA_CLIENT* client_get(unsigned int cid)
{
	unsigned int idx;
	idx = cid & 0xffff;
	if(idx>=sizeof(client_list)/sizeof(client_list[0])) return NULL;
	if(client_list[idx].cid!=cid) return NULL;
	return &client_list[idx];
}

PROTOCOL_LUA_STATE* state_get(unsigned int sid)
{
	unsigned int idx;
	idx = sid & 0xffff;
	if(idx>=sizeof(state_list)/sizeof(state_list[0])) return NULL;
	if(state_list[idx].sid!=sid) return NULL;
	return &state_list[idx];
}
