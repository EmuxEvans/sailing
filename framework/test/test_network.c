#include <stdio.h>

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
int recvbuf_size = 10000;

int main(int argc, char* argv[])
{
	SOCK_ADDR sa;

	sock_init();
	fdwatch_init();
	mempool_init();
	network_init(20000);

	recvbuf_pool = mempool_create("RECVBUF", recvbuf_size, 0);
	network_tcp_register(sock_str2addr("0.0.0.0:1980", &sa), onaccept_proc, NULL);

	getchar();

	network_tcp_unregister(&sa);
	mempool_destroy(recvbuf_pool);

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
}

void ondata_proc(NETWORK_HANDLE handle, void* userptr)
{
}

void ondisconnect_proc(NETWORK_HANDLE handle, void* userptr)
{
}
