#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <skates.h>

#include "sync_rpc.h"

static const char svr_edp[] = "127.0.0.1:30000";
static const char clt_edp[] = "127.0.0.1:20000";

static void client_event(void* ptr);
static void server_event(void* ptr);

static void a_event(void* ptr);
static void b_event(void* ptr);

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
	printf("threadpool_init ret = %d\n",  threadpool_init(10));
	printf("rpcnet_init ret = %d\n",  rpcnet_init());
	printf("rpcfun_init ret = %d\n",  rpcfun_init());
	
	if(strcmp(argv[1], "S")==0) {
		sock_str2addr(svr_edp, &addr);
	} else {
		sock_str2addr(clt_edp, &addr);
	}
	printf("rpcnet_bind ret = %d\n", rpcnet_bind(&addr));

	rpcfun_register(__sync_rpc_desc, 0);

	if(strcmp(argv[1], "S")==0) {
		threadpool_queueitem(server_event, NULL);
	} else {
		threadpool_queueitem(client_event, NULL);
	}

	printf("press enter to exit!\n");
	getchar();

	rpcfun_unregister(__sync_rpc_desc, 0);

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
	int ret, a1=__LINE__, a2[1], a3[1], a4[2];
    SOCK_ADDR     addr;
    RPCNET_GROUP*   group;

    if(sock_str2addr(svr_edp, &addr)==NULL) {
        printf("error: %s:%d\n", __FILE__, __LINE__);
        return;
    }
    group = rpcnet_getgroup(&addr);
    if(group==NULL) {
        printf("error: %s:%d\n", __FILE__, __LINE__);
        return;
    }

	printf("do client_event!\n");

	ret = test_rpc1(group, a1, a2, a3, a4);
	printf("call test_rpc1, ret = %d\n", ret);
}

void server_event(void* ptr)
{
	printf("do server_event!\n");
}

void a_event(void* ptr)
{
	int ret, a1[1], a2[2], a3[2], a4[3];

	ret = test_rpc2((RPCNET_GROUP*)ptr, a1, a2, a3, a4);
	printf("call test_rpc1, ret = %d\n", ret);
}

void b_event(void* ptr)
{
	int ret, a1=__LINE__, a2[1], a3[1], a4[2];

	ret = test_rpc1((RPCNET_GROUP*)ptr, a1, a2, a3, a4);
	printf("call test_rpc1, ret = %d\n", ret);
}

int test_rpc1_impl(RPCNET_GROUP* group, int r1, const int* r2, int* r3, int* r4)
{
	printf("test_rpc1_impl\n");

	printf("post a_event\n");
	threadpool_queueitem(a_event, group);

	return ERR_NOERROR;
}

int test_rpc2_impl(RPCNET_GROUP* group, int* r1, int* r2, int* r3, int* r4)
{
	printf("test_rpc2_impl\n");

	printf("post b_event\n");
	threadpool_queueitem(b_event, group);

	return ERR_NOERROR;
}

