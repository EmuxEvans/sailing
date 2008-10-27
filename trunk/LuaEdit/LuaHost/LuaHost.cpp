#include <stdio.h>

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

#include "LuaDebugClientRpc.h"
#include "LuaDebugHostRpc.h"

#include <skates\lua\lua.h>
#include <skates\lua\lauxlib.h>

static int run_flag = 1;
static RPCNET_GROUP* client_grp = NULL;
static void do_proc(void*);

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
		printf("press enter to continue\n");
		getchar();
		threadpool_queueitem(do_proc, NULL);
		printf("begin\n");
		while(run_flag) {
			os_sleep(100);
		}
		printf("end\n");
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

void do_proc(void*)
{
	run_flag = 0;
}

int LuaDebugHostRpc_Attach_impl(RPCNET_GROUP* group)
{
	int ret;
	if(client_grp!=NULL) {
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
	ret = LuaDebugClientRpc_Attach(client_grp);
	if(ret!=ERR_NOERROR) {
		printf("@@LuaDebugClientRpc_Attach return %d\n", ret);
	}
	client_grp = NULL;
	return ret;
}

int LuaDebugHostRpc_RunCmd_impl(RPCNET_GROUP* group, const char* Cmd)
{
	printf("@@LuaDebugHostRpc_RunCmd_impl : [[[%s\n]]]", Cmd);
	return ERR_NOERROR;
}
