#include <string.h>
#include <assert.h>

#include <skates/skates.h>

#include "CubeServer.h"

//lua_State* L;
//	L = protocol_lua_newstate(NULL, "default");
//
//	ret = luaL_loadfile(L, argv[1]);
//	if(ret!=0) {
//		fprintf(stderr, "LUA_ERROR: %s\n", lua_tostring(L, -1));
//		lua_pop(L, 1);
//	}
//	printf("lua script \"%s\" loaded.\n", argv[1]);
//
//	lua_close(L);

int main(int argc, char* argv[])
{
	SOCK_ADDR sa;

	sock_init();
	fdwatch_init();
	mempool_init();
	threadpool_init(10);
	timer_init(100);
	network_init(1024);
	rpcnet_init();
	rpcfun_init();
	sock_str2addr("127.0.0.1:0", &sa);
	rpcnet_bind(&sa);
	dymempool_init(30, 1024);
	protocol_lua_init();

	cubeserver_init();

	getchar();

	cubeserver_final();

	protocol_lua_final();
	dymempool_final();
	rpcnet_unbind();
	rpcfun_final();
	rpcnet_final();
	network_final();
	timer_final();
	threadpool_final();
	mempool_final();
	fdwatch_final();
	sock_final();

	return 0;
}
