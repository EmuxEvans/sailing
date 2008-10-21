#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/misc.h>
#include <skates/protocol_def.h>
#include <skates/protocol_lua.h>
#include <skates/protocol.h>

#include "sample_p.proto.h"
#include "sample_p.proto.lua.h"

static lua_State* L;
static int my_struct_func(lua_State* L);
static int my_object_func(lua_State* L);
static int my_lua_func(lua_State* L);
static int my_obj_save(lua_State* L);
static int my_obj_load(lua_State* L);

int main(int argc, char* argv[])
{
	L = luaL_newstate();
	luaL_openlibs(L);
	protocol_lua_init(L);

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

	for(;;) {
		char line[300];
		int ret;

		printf(">");
		gets(line);
		strtrim(line);
		strltrim(line);

		if(line[0]=='@') {
			ret = luaL_dofile(L, line+1);
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

	return 0;
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
