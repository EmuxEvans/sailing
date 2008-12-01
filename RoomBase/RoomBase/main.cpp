#include <string.h>
#include <assert.h>

#include <skates/skates.h>

#include "GameUser.h"
#include "GameRoom.h"
#include "GameUser.inl"
#include "GameRoom.inl"

#include "CubeRoom.proto.h"
#include "CubeRoom.h"

lua_State* L;

int main(int argc, char* argv[])
{
	int ret;

	if(argc!=2) {
		printf("invalid parameter\n");
		return -1;
	}

	dymempool_init(30, 1024);
	L = protocol_lua_newstate(NULL, "default");

	ret = luaL_loadfile(L, argv[1]);
	if(ret!=0) {
		fprintf(stderr, "LUA_ERROR: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	printf("lua script \"%s\" loaded.\n", argv[1]);

	lua_close(L);
	dymempool_final();
	return 0;
}
