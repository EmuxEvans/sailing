#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/sock.h"
#include "../inc/mempool.h"
#include "../inc/rlist.h"
#include "../inc/network.h"

#include "../inc/rlist.h"
#include "../inc/fdwatch.h"

static void onaccept_proc(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname);
static void onconnect_proc(NETWORK_HANDLE handle, void* userptr);
static void ondata_proc(NETWORK_HANDLE handle, void* userptr);
static void ondisconnect_proc(NETWORK_HANDLE handle, void* userptr);

static MEMPOOL_HANDLE recvbuf_pool;
static int recvbuf_size = 10000;
static SOCK_ADDR sa;

static void print_usage();
static void server_do();
static void client_do();

int main(int argc, char* argv[])
{
	srandom((int)time(NULL));

	if(argc!=3) { print_usage(); exit(0); }
	if(sock_str2addr(argv[2], &sa)==NULL) { print_usage(); exit(0); }

	if(strcmp(argv[1], "client")!=0 && strcmp(argv[1], "server")!=0) { print_usage(); exit(0); }

	sock_init();
	fdwatch_init();
	mempool_init();
	network_init(20000);

	if(strcmp(argv[1], "client")==0) {
		client_do();
	} else {
		server_do();
	}

	network_final();
	mempool_final();
	fdwatch_final();
	sock_final();

	return 0;
}

void onaccept_proc(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname)
{
	char addr[50];
	NETWORK_EVENT event;

	printf("accept from %s\n", sock_addr2str(pname, addr));
	sock_nonblock(sock);

	event.OnConnect = onconnect_proc;
	event.OnData = ondata_proc;
	event.OnDisconnect = ondisconnect_proc;
	event.recvbuf_pool = recvbuf_pool;
	event.recvbuf_buf = NULL;
	event.recvbuf_max = recvbuf_size;

	if(network_add(sock, &event, NULL)==NULL) {
		sock_disconnect(sock);
		sock_close(sock);
	}
}

void onconnect_proc(NETWORK_HANDLE handle, void* userptr)
{
	char addr[50];
	sock_addr2str(network_get_peername(handle), addr);
	printf("%p connected. from %s\n", handle, addr);
}

void ondata_proc(NETWORK_HANDLE handle, void* userptr)
{
	char addr[50];
	sock_addr2str(network_get_peername(handle), addr);
	printf("%p ondata(%d).\n", handle, network_recvbuf_len(handle));
	network_recvbuf_commit(handle, network_recvbuf_len(handle));
}

void ondisconnect_proc(NETWORK_HANDLE handle, void* userptr)
{
	char addr[50];
	sock_addr2str(network_get_peername(handle), addr);
	printf("%p disconnect. from %s\n", handle, addr);
}

void print_usage()
{
}

void server_do()
{
	recvbuf_pool = mempool_create("RECVBUF", recvbuf_size, 0);
	if(network_tcp_register(sock_str2addr("0.0.0.0:1980", &sa), onaccept_proc, NULL)!=ERR_NOERROR) {
		printf("Failed to network_tcp_register.\n");
		exit(0);
	}
	getchar();
	network_tcp_unregister(&sa);
	mempool_destroy(recvbuf_pool);
}

static int quit_flag = 0;
static unsigned int ZION_CALLBACK client_thread_proc(void* arg);
static os_thread_t tids[10];

void client_do()
{
	int i;

	for(i=0; i<sizeof(tids)/sizeof(tids[0]); i++) {
		os_thread_begin(&tids[i], client_thread_proc, NULL);
	}

	getchar();
	quit_flag = 1;

	for(i=0; i<sizeof(tids)/sizeof(tids[0]); i++) {
		os_thread_wait(tids[i], NULL);
	}
}

unsigned int ZION_CALLBACK client_thread_proc(void* arg)
{
	SOCK_HANDLE sock = SOCK_INVALID_HANDLE;
	char str[100];

	memset(str, 'a', sizeof(str));
	str[97] = '\0';

	while(!quit_flag) {
		os_sleep(10);

		if(sock==SOCK_INVALID_HANDLE) {
			sock = sock_connect(&sa, 0);
		}
		if(sock==SOCK_INVALID_HANDLE) continue;

		if((rand()%100)<95) {
			if(sock_writebuf(sock, str, strlen(str)+1)==ERR_NOERROR) {
				printf("%p send\n", str);
				continue;
			}
			printf("failed to sock_writebuf\n");
		}

		sock_disconnect(sock);
		sock_close(sock);
		sock = SOCK_INVALID_HANDLE;
	}
	return 0;
}

