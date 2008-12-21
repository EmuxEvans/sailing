#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/sock.h>
#include <skates/mempool.h>
#include <skates/rlist.h>
#include <skates/network.h>

#include <skates/rlist.h>
#include <skates/fdwatch.h>
#include <skates/threadpool.h>

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
	if(argc!=3) { print_usage(); exit(0); }
	if(sock_str2addr(argv[2], &sa)==NULL) { print_usage(); exit(0); }

	if(strcmp(argv[1], "client")!=0 && strcmp(argv[1], "server")!=0) { print_usage(); exit(0); }

	sock_init();
	fdwatch_init();
	mempool_init();
	threadpool_init(1);
	network_init(20000);

	if(strcmp(argv[1], "client")==0) {
		client_do();
	} else {
		server_do();
	}

	network_final();
	threadpool_final();
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

	if(!network_add(sock, &event, NULL)) {
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
	unsigned int count;
	NETWORK_DOWNBUF* bufs[10];

	char addr[50];
	sock_addr2str(network_get_peername(handle), addr);
	printf("%p ondata(%d).\n", handle, network_recvbuf_len(handle));

	count = network_downbufs_alloc(bufs, sizeof(bufs)/sizeof(bufs[0]), network_recvbuf_len(handle));
	if(count>0) {
		const void* buf;
		buf = network_recvbuf_ptr(handle, 0, network_recvbuf_len(handle));
		if(buf) {
			network_downbufs_fill(bufs, count, 0, buf, network_recvbuf_len(handle));
			network_send(handle, bufs, count);
		}
	}

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
	if(network_tcp_register(&sa, onaccept_proc, NULL)!=ERR_NOERROR) {
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
	char str_out[100];
	char str_in[100];
	int k = 0, len;

	while(!quit_flag) {
		os_sleep(10);

		if(sock==SOCK_INVALID_HANDLE) {
			sock = sock_connect(&sa, 0);
		}
		if(sock==SOCK_INVALID_HANDLE) continue;

		if((rand()%100)<95) {
			sprintf(str_out+4, "%p %d ", &sock, k++);
			len = strlen(str_out+4);
			memset(str_out+4+len, 'a'+k%26, sizeof(str_out)-4-len);
			str_out[sizeof(str_out)/sizeof(str_out[0])-1] = '\0';
			*((int*)str_out) = sizeof(str_out)-4;
			if(sock_writebuf(sock, str_out, sizeof(str_out))==ERR_NOERROR) {
				if(sock_readbuf(sock, str_in, sizeof(str_in))==ERR_NOERROR) {
					if(memcmp(str_in, str_out, sizeof(str_in))==0) {
						continue;
					} else {
						printf("failed to memcmp\n");
					}
				} else {
					printf("failed to sock_readbuf\n");
				}
			} else {
				printf("failed to sock_writebuf\n");
			}
		}

		sock_disconnect(sock);
		sock_close(sock);
		sock = SOCK_INVALID_HANDLE;
	}
	if(sock!=SOCK_INVALID_HANDLE) return 0;

	sprintf(str_out+4, "quit");
	len = strlen(str_out+4)+1;
	*((int*)str_out) = len;
	if(sock_writebuf(sock, str_out, 4+len)==ERR_NOERROR) {
		if(sock_readbuf(sock, str_in, 4+len)==ERR_NOERROR) {
			if(memcmp(str_in, str_out, 4+len)==0) {
				printf("very well\n");
			} else {
				printf("E failed to memcmp\n");
			}
		} else {
			printf("E failed to sock_readbuf\n");
		}
	} else {
		printf("E failed to sock_writebuf\n");
	}


	return 0;
}

