#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "../inc/skates.h"

#define SUBSYS_A				0
#define SUBSYS_B				1
#define SUBSYS_CALLBACK			100

static void client_event(void* arg);
static void server_event(void* arg);

static void subsys_a_ondata(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* conn, STREAM* stream);
static void subsys_b_ondata(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* conn, STREAM* stream);

static void subsys_a_proc(void* arg);
static void subsys_b_proc(void* arg);

static void send_subsys_a(RPCNET_GROUP* group);
static void send_subsys_b(RPCNET_GROUP* group);

static void subsys_a_event(int etype, RPCNET_EVENTDATA* data);
static void subsys_b_event(int etype, RPCNET_EVENTDATA* data);

static const char svr_edp[] = "127.0.0.1:30000";
static const char clt_edp[] = "127.0.0.1:20000";

void test_ep2idx()
{
	SOCK_ADDR addr;
	sock_addr(&addr, 1000, 0);

	printf("index = %d\n", rpcnet_ep2idx(&addr));
	printf("index = %d\n", rpcnet_ep2idx(&addr));
	printf("index = %d\n", rpcnet_ep2idx(&addr));
	printf("index = %d\n", rpcnet_ep2idx(&addr));
	printf("index = %d\n", rpcnet_ep2idx(&addr));
	printf("index = %d\n", rpcnet_ep2idx(&addr));
}

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
	
	if(strcmp(argv[1], "S")==0) {
		sock_str2addr(svr_edp, &addr);
	} else {
		sock_str2addr(clt_edp, &addr);
	}
	printf("rpcnet_bind ret = %d\n", rpcnet_bind(&addr));
	
	rpcnet_register_subsys(SUBSYS_A, subsys_a_event);
	rpcnet_register_subsys(SUBSYS_B, subsys_b_event);

	if(strcmp(argv[1], "S")==0) {
		threadpool_queueitem(server_event, NULL);
	} else {
		threadpool_queueitem(client_event, NULL);
	}

	test_ep2idx();	
	printf("press enter to exit!\n");
	getchar();

	printf("rpcnet_unbind ret = %d\n",  rpcnet_unbind());
	printf("rpcnet_shutdown ret = %d\n", rpcnet_shutdown());
	printf("rpcnet_final ret = %d\n",  rpcnet_final());
	printf("threadpool_final ret = %d\n",  threadpool_final());
	printf("fdwatch_final = %d\n",  fdwatch_final());
	sock_final();
	mempool_final();

	return 0;
}

void client_event(void* ptr)
{
	SOCK_ADDR addr;
	RPCNET_GROUP*	group;
	printf("do client_event!\n");
	if(sock_str2addr(svr_edp, &addr)==NULL) {
		printf("error: %d\n", __LINE__);
		return;
	}
	group = rpcnet_getgroup(&addr);
	if(group==NULL) {
		printf("error: %d\n", __LINE__);
		return;
	}
	send_subsys_a(group);
	send_subsys_b(group);
	send_subsys_a(group);
	send_subsys_b(group);
	send_subsys_a(group);
	send_subsys_b(group);
	send_subsys_a(group);
	send_subsys_b(group);
	send_subsys_a(group);
	send_subsys_b(group);
}

void server_event(void* ptr)
{
	printf("do server_event!\n");
}

void subsys_a_ondata(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* conn, STREAM* stream)
{
	int ret;
	int len;
	char str[1000];
	char addr[100];

	ret = stream_get(stream, &len, sizeof(len));
	if(ret!=ERR_NOERROR) printf("error: %s:%d %d\n", __FILE__, __LINE__, ret);
	ret = stream_get(stream, &str, len);
	if(ret!=ERR_NOERROR) printf("error: %s:%d %d\n", __FILE__, __LINE__, ret);

	sock_addr2str(rpcnet_group_get_endpoint(group, NULL), addr);
	printf("subsys_a_ondata from %s : %d\n", addr, len);
	
	assert(group!=NULL);
	threadpool_queueitem(subsys_b_proc, group);
}

void subsys_b_ondata(RPCNET_THREAD_CONTEXT* ctx, RPCNET_GROUP* group, RPCNET_CONNECTION* conn, STREAM* stream)
{
	int ret;
	int len;
	char str[1000];
	char addr[100];

	ret = stream_get(stream, &len, sizeof(len));
	if(ret!=ERR_NOERROR) printf("error: %s:%d %d\n", __FILE__, __LINE__, ret);
	ret = stream_get(stream, &str, len);
	if(ret!=ERR_NOERROR) printf("error: %s:%d %d\n", __FILE__, __LINE__, ret);

	sock_addr2str(rpcnet_group_get_endpoint(group, NULL), addr);
	printf("subsys_b_ondata from %s : %d\n", addr, len);

	assert(group!=NULL);
	threadpool_queueitem(subsys_a_proc, group);
}

void subsys_a_proc(void* arg)
{
	RPCNET_GROUP* group;
	group = (RPCNET_GROUP*)arg;
	printf("do send subsys_a %s:%d\n", __FILE__, __LINE__);
	send_subsys_a(group);
}

void subsys_b_proc(void* arg)
{
	RPCNET_GROUP* group;
	group = (RPCNET_GROUP*)arg;
	printf("do send subsys_b %s:%d\n", __FILE__, __LINE__);
	send_subsys_b(group);
}

void send_subsys_a(RPCNET_GROUP* group)
{
	int ret;
	int len;
	char data[] = "send_subsys_a call dagtaajfkjfk";
	RPCNET_THREAD_CONTEXT*	ctx;
	RPCNET_CONNECTION*		conn;
	STREAM*						stream;

	ctx = rpcnet_context_get();
	if(ctx==NULL) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		return;
	}
	conn = rpcnet_context_getconn(ctx, group);
	if(conn==NULL) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		rpcnet_context_free(ctx);	
		return;
	}
	stream = rpcnet_context_getstream(ctx);

	len = (int)strlen(data) + 1;
	ret = stream_put(stream, &len, sizeof(len));
	if(ret!=ERR_NOERROR) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		rpcnet_context_free(ctx);	
		return;
	}
	ret = stream_put(stream, data, len);
	if(ret!=ERR_NOERROR) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		rpcnet_context_free(ctx);	
		return;
	}
	
	ret = rpcnet_conn_write(conn, SUBSYS_A, stream);
	if(ret!=ERR_NOERROR) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		rpcnet_context_free(ctx);	
		return;
	}

	rpcnet_context_free(ctx);
}

void send_subsys_b(RPCNET_GROUP* group)
{
	int ret;
	int len;
	char data[] = "send_subsys_b call happy fucking new year";
	RPCNET_THREAD_CONTEXT*	ctx;
	RPCNET_CONNECTION*		conn;
	STREAM*						stream;

	ctx = rpcnet_context_get();
	if(ctx==NULL) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		return;
	}
	conn = rpcnet_context_getconn(ctx, group);
	if(conn==NULL) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		rpcnet_context_free(ctx);	
		return;
	}
	stream = rpcnet_context_getstream(ctx);

	len = (int)strlen(data) + 1;
	ret = stream_put(stream, &len, sizeof(len));
	if(ret!=ERR_NOERROR) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		rpcnet_context_free(ctx);	
		return;
	}
	ret = stream_put(stream, data, len);
	if(ret!=ERR_NOERROR) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		rpcnet_context_free(ctx);	
		return;
	}
	
	ret = rpcnet_conn_write(conn, SUBSYS_B, stream);
	if(ret!=ERR_NOERROR) {
		printf("error: %s:%d\n", __FILE__, __LINE__);
		rpcnet_context_free(ctx);	
		return;
	}

	rpcnet_context_free(ctx);
}

void subsys_a_event(int etype, RPCNET_EVENTDATA* data)
{
	char str[100];
	SOCK_ADDR addr;
	switch(etype) {
	case RPCNET_EVENTTYPE_DATA:
		subsys_a_ondata(data->DATA.context, data->DATA.group, data->DATA.conn, data->DATA.stream);
		break;
	case RPCNET_EVENTTYPE_RESET:
		rpcnet_group_get_endpoint(data->RESET.group, &addr);
		sock_addr2str(&addr, str);
		printf("subsys_a_event: RPCNET_EVENTTYPE_GROUPRESET %s\n", str);
		break;
	case RPCNET_EVENTTYPE_INIT:
		rpcnet_group_get_endpoint(data->INIT.group, &addr);
		sock_addr2str(&addr, str);
		printf("subsys_a_event: RPCNET_EVENTTYPE_INIT %s\n", str);
		break;
	}
}

void subsys_b_event(int etype, RPCNET_EVENTDATA* data)
{
	char str[100];
	SOCK_ADDR addr;
	switch(etype) {
	case RPCNET_EVENTTYPE_DATA:
		subsys_b_ondata(data->DATA.context, data->DATA.group, data->DATA.conn, data->DATA.stream);
		break;
	case RPCNET_EVENTTYPE_RESET:
		rpcnet_group_get_endpoint(data->RESET.group, &addr);
		sock_addr2str(&addr, str);
		printf("subsys_b_event: RPCNET_EVENTTYPE_GROUPRESET %s\n", str);
		break;
	case RPCNET_EVENTTYPE_INIT:
		rpcnet_group_get_endpoint(data->INIT.group, &addr);
		sock_addr2str(&addr, str);
		printf("subsys_b_event: RPCNET_EVENTTYPE_INIT %s\n", str);
		break;
	}
}
