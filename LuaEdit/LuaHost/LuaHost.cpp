#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <skates\errcode.h>
#include <skates\os.h>
#include <skates\rlist.h>
#include <skates\mempool.h>
#include <skates\threadpool.h>
#include <skates\fdwatch.h>
#include <skates\sock.h>
#include <skates\hashmap.h>
#include <skates\stream.h>
#include <skates\rpc_net.h>
#include <skates\rpc_fun.h>
#include <skates\misc.h>

#include "LuaDebugClientRpc.h"
#include "LuaDebugHostRpc.h"

extern "C" {
#include <skates\lua\lua.h>
#include <skates\lua\lauxlib.h>
#include <skates\lua\lualib.h>
}

static RPCNET_GROUP* client_grp = NULL;
static lua_State* L = NULL;

static int want_return(lua_State* L)
{
	getchar();
	return 0;
}
static int debug_break(lua_State* L)
{
	if(!client_grp)
		return 0;

	threadpool_s();
	LuaDebugClientRpc_BreakPoint(client_grp);
	threadpool_e();
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

	if(!client_grp)
		return 0;

	threadpool_s();
	LuaDebugClientRpc_DebugMsg(client_grp, type, msg);
	threadpool_e();
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

	ret = rpcnet_bind(&sa);
	if(ret==ERR_NOERROR) {
		rpcfun_register(__LuaDebugHostRpc_desc, 0);

		threadpool_s();
		lua_do();
		threadpool_e();

		rpcfun_register(__LuaDebugHostRpc_desc, 0);
		rpcnet_unbind();
	} else {
		printf("error in rpcnet_bind(), return %d\n", ret);
	}

	rpcfun_final();
	rpcnet_final();
	threadpool_final();
	fdwatch_final();
	sock_final();
	mempool_final();
	return 0;
}

int LuaDebugHostRpc_Attach_impl(RPCNET_GROUP* group)
{
	int ret;
	if(client_grp!=NULL && client_grp!=group) {
		ret = LuaDebugClientRpc_Detach(client_grp);
		if(ret!=ERR_NOERROR) {
			printf("@@LuaDebugClientRpc_Detach return %d\n", ret);
		}
	}
	client_grp = group;
	ret = LuaDebugClientRpc_Attach(client_grp);
	if(ret!=ERR_NOERROR) {
		printf("@@LuaDebugClientRpc_Attach return %d\n", ret);
	}
	client_grp = group;
	return ret;
}

int LuaDebugHostRpc_Detach_impl(RPCNET_GROUP* group)
{
	int ret;
	if(client_grp==NULL) {
		printf("@@LuaDebugHostRpc_Detach_impl(): client_grp==NULL\n");
		return ERR_NOERROR;
	}
	ret = LuaDebugClientRpc_Detach(client_grp);
	if(ret!=ERR_NOERROR) {
		printf("@@LuaDebugClientRpc_Detach return %d\n", ret);
	}
	client_grp = NULL;
	return ret;
}


int LuaDebugHostRpc_RunCmd_impl(RPCNET_GROUP* group, const char* Cmd)
{
	luaL_dostring(L, Cmd);
	return ERR_NOERROR;
}

int LuaDebugHostRpc_GetCallStack_impl(RPCNET_GROUP* group, LUADEBUG_CALLSTACK* stacks, int* depth)
{
	int i;
	lua_Debug ar;

	for(i=1; i<*depth; i++) {
		memset(&ar, 0, sizeof(ar));
		if(!lua_getstack(L, i, &ar)) break;
		if(!lua_getinfo(L, "flnSu", &ar)) {
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
