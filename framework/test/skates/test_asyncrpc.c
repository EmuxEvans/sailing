#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <skates/skates.h>

#include "async_rpc.h"

static const char svr_edp[] = "127.0.0.1:30000";
static const char clt_edp[] = "127.0.0.1:20000";

static void client_event(void* ptr);
static void server_event(void* ptr);

int main(int argc, char* argv[])
{
	SOCK_ADDR addr;

	if(argc!=2 || (strcmp(argv[1], "S")!=0 && strcmp(argv[1], "C")!=0)) {
		printf("invlid paramter\n");
		return(-1);
	}

	mempool_init();
	sock_init();
	printf("fdwatch_init ret = %d\n",  fdwatch_init());
	printf("threadpool_init ret = %d\n",  threadpool_init(1));
	printf("rpcnet_init ret = %d\n",  rpcnet_init());
	printf("rpcfun_init ret = %d\n",  rpcfun_init());
	
	if(strcmp(argv[1], "S")==0) {
		sock_str2addr(svr_edp, &addr);
	} else {
		sock_str2addr(clt_edp, &addr);
	}
	printf("rpcnet_bind ret = %d\n", rpcnet_bind(&addr));

	rpcfun_register(__async_rpc_desc, 0);

	if(strcmp(argv[1], "S")==0) {
		threadpool_queueitem(server_event, NULL);
	} else {
		threadpool_queueitem(client_event, NULL);
	}

	printf("press enter to exit!\n");
	getchar();

	rpcfun_unregister(__async_rpc_desc, 0);

	printf("rpcnet_unbind ret = %d\n",  rpcnet_unbind());
	printf("rpcnet_shutdown ret = %d\n", rpcnet_shutdown());
	printf("rpcfun_final ret = %d\n",  rpcfun_final());
	printf("rpcnet_final ret = %d\n",  rpcnet_final());
	printf("threadpool_final ret = %d\n",  threadpool_final());
	printf("fdwatch_final = %d\n",  fdwatch_final());
	sock_final();
	mempool_final();

	return 0;
}

void client_event(void* ptr)
{
	printf("do client_event!\n");
}

void server_event(void* ptr)
{
	printf("do server_event!\n");
}

int test_rpc1_impl(RPCNET_GROUP* group, int r1, const int* r2, int* r3, int* r4)
{
	return ERR_NOERROR;
}

int test_rpc2_impl(RPCNET_GROUP* group, int* r1, int* r2, int* r3, int* r4)
{
	return ERR_NOERROR;
}

