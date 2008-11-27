#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <skates\skates.h>

static RPCNET_GROUP* client_grp = NULL;
static lua_State* L = NULL;

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
static void lua_do()
{
	L = luaL_newstate();
	luaL_openlibs(L);

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
}

int main(int argc, char* argv[])
{
	int ret;
	SOCK_ADDR sa;

	if(argc<2 || sock_str2addr(argv[1], &sa)==NULL) {
		printf("invalid parameter\n");
		return 0;
	}

	mempool_init();
	sock_init();
	fdwatch_init();
	threadpool_init(10);
	rpcnet_init();
	rpcfun_init();
	dymempool_init(56, 2048);
	protocol_lua_init();

	ret = rpcnet_bind(&sa);
	if(ret==ERR_NOERROR) {
		threadpool_s();
		lua_do();
		threadpool_e();

		rpcnet_unbind();
	} else {
		printf("error in rpcnet_bind(), return %d\n", ret);
	}

	protocol_lua_final();
	dymempool_final();
	rpcfun_final();
	rpcnet_final();
	threadpool_final();
	fdwatch_final();
	sock_final();
	mempool_final();
	return 0;
}
