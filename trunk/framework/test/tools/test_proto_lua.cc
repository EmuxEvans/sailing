#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <skates/skates.h>

#include "sample_p.proto.h"
#include "sample_p.proto.lua.h"

static lua_State* L;
static void lua_do();
static int my_struct_func(lua_State* L);
static int my_object_func(lua_State* L);
static int my_lua_func(lua_State* L);
static int my_obj_save(lua_State* L);
static int my_obj_load(lua_State* L);


static int want_return(lua_State* L)
{
	getchar();
	return 0;
}
static int debug_break(lua_State* L)
{
	protocol_lua_debugbreak(L);
	return 0;
}
static int debug_msg(lua_State* L)
{
	int type;
	const char* msg;

	if(lua_gettop(L)!=2 || !lua_isnumber(L, 1) || !lua_isstring(L, 2)) {
		luaL_error(L, "invalid parameter type.\n");
		return 0;
	}

	type = lua_tointeger(L, 1);
	msg = lua_tolstring(L, 2, NULL);

	protocol_lua_debugmsg(L, type, msg);

	return 0;
}

int main(int argc, char* argv[])
{
	SOCK_ADDR sa;
	int ret;

	switch(argc) {
	case 1:
		sock_str2addr("0.0.0.0:0", &sa);
		break;
	case 2:
		if(sock_str2addr(argv[1], &sa)==NULL) {
			printf("invalid args");
			return -1;
		}
		break;
	default:
		printf("invalid args");
		return -1;
	}

	mempool_init();
	sock_init();
	fdwatch_init();
	threadpool_init(10);
	rpcnet_init();
	rpcfun_init();
	dymempool_init(56, 2048);

	ret = rpcnet_bind(&sa);
	if(ret==ERR_NOERROR) {
		char tmp[100];
		sock_addr2str(&sa, tmp);
		printf("BIND %s\n", tmp);

		protocol_lua_init();

		lua_do();

		protocol_lua_final();
	} else {
		printf("error in rpcnet_bind(), return %d\n", ret);
	}

	dymempool_final();
	rpcfun_final();
	rpcnet_final();
	threadpool_final();
	fdwatch_final();
	sock_final();
	mempool_final();

	return 0;
}

void lua_do()
{
	L = protocol_lua_newstate(NULL, "welcome");
	luaL_openlibs(L);

	lua_pushstring(L, "new_my_struct");
	lua_pushcfunction(L, my_struct_func);
	lua_rawset(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "new_my_object");
	lua_pushcfunction(L, my_object_func);
	lua_rawset(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "proto_save");
	lua_pushcfunction(L, my_obj_save);
	lua_rawset(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "proto_load");
	lua_pushcfunction(L, my_obj_load);
	lua_rawset(L, LUA_GLOBALSINDEX);

	lua_pushstring(L,"want_return");
	lua_pushcfunction(L, want_return);
	lua_rawset(L,LUA_GLOBALSINDEX);
	lua_pushstring(L,"debug_break");
	lua_pushcfunction(L, debug_break);
	lua_rawset(L,LUA_GLOBALSINDEX);
	lua_pushstring(L,"debug_msg");
	lua_pushcfunction(L, debug_msg);
	lua_rawset(L,LUA_GLOBALSINDEX);


	for(;;) {
		char line[300];
		int ret;

		printf(">");
		gets(line);
		strtrim(line);
		strltrim(line);

		if(line[0]=='@') {
			ret = luaL_dofile(L, line+1);
		} else if(strcmp(line, "quit")==0) {
			break;
		} else {
			if(line[0]!='\0') {
				ret = luaL_dostring(L, line);
			} else {
				ret = 0;
			}
		}
		if(ret) {
			fprintf(stderr, "LUA_ERROR: %s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
	}

	protocol_lua_closestate(L);
}

int my_struct_func(lua_State* L)
{
	MY_STRUCT* ptr;
	ptr = (MY_STRUCT*)malloc(sizeof(MY_STRUCT));
	memset(ptr, 0, sizeof(*ptr));
	protocol_lua_newstruct(L, &PROTOCOL_NAME(MY_STRUCT), ptr);
	return 1;
}

class CCallMe : public ICallMe {
public:
	virtual os_int callme(os_int i) {
		return i;
	}
};

int my_object_func(lua_State* L)
{
	ICallMe* obj;
	obj = new CCallMe;
	protocol_lua_newobject(L, &PROTOCOL_NAME(ICallMe), obj);
	return 1;
}

int my_obj_save(lua_State* L)
{
	PROTOCOL_LUA_OBJECT* obj;
	const char* filename;
	int ret;

	if(lua_gettop(L)<2 || lua_type(L, 1)!=LUA_TUSERDATA || lua_type(L, 2)!=LUA_TSTRING) {
		luaL_error(L, "invalid parameter type, array.\n");
		return 0;
	}

	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, 1);
	filename = lua_tolstring(L, 2, NULL);

	ret = protocol_file_write(obj->s.type, obj->s.ptr, filename);
	lua_pushinteger(L, ret);
	return 1;
}

int my_obj_load(lua_State* L)
{
	PROTOCOL_LUA_OBJECT* obj;
	const char* filename;
	int ret;

	if(lua_gettop(L)<2 || lua_type(L, 1)!=LUA_TUSERDATA || lua_type(L, 2)!=LUA_TSTRING) {
		luaL_error(L, "invalid parameter type, array.\n");
		return 0;
	}

	obj = (PROTOCOL_LUA_OBJECT*)lua_touserdata(L, 1);
	filename = lua_tolstring(L, 2, NULL);

	ret = protocol_file_read(obj->s.type, filename, obj->s.ptr);
	lua_pushinteger(L, ret);
	return 1;
}
