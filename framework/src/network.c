#include <assert.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/sock.h"
#include "../inc/mempool.h"
#include "../inc/network.h"

#include "../inc/rlist.h"
#include "../inc/fdwatch.h"

typedef struct TCP_ENDPOINT {
	FDWATCH_ITEM fdwitem;

	SOCK_HANDLE sock;
	SOCK_ADDR sa;
	NETWORK_ONACCEPT OnAccept;
	void* userptr;

} TCP_ENDPOINT;

struct NETWORK_CONNECTION {
	FDWATCH_ITEM fdwitem;
	SOCK_HANDLE sock;
	void* userptr;

	NETWORK_ONCONNECT		OnConnect;
	NETWORK_ONDATA			OnData;
	NETWORK_ONDISCONNECT	OnDisconnect;

	MEMPOOL_HANDLE			recvbuf_pool;
	unsigned char*			recvbuf_buf;
	unsigned int			recvbuf_len;

	NETWORK_DOWNBUF*		downbuf_begin;
	NETWORK_DOWNBUF*		downbuf_end;
	unsigned int			downbuf_cur;
};

static unsigned int downbuf_size;
static MEMPOOL_HANDLE downbuf_pool;
static os_mutex_t downbuf_mutex;

static TCP_ENDPOINT tcp_eps[100];
static int network_quit_flag, network_ep_shutdown;
static os_condition_t network_write_cond;
static os_thread_t network_read_handle;
static os_thread_t network_write_handle;
static unsigned int ZION_CALLBACK network_read_proc(void* arg);
static unsigned int ZION_CALLBACK network_write_proc(void* arg);

static FDWATCH_HANDLE network_fdw;
static void network_accept_event(FDWATCH_ITEM* item, int events);
//static void network_connection_event(FDWATCH_ITEM* item, int events);

void network_init(unsigned int size)
{
	int l;

	downbuf_size = size;
	downbuf_pool = mempool_create(sizeof(NETWORK_DOWNBUF)+size, 0);
	os_mutex_init(&downbuf_mutex);

	network_ep_shutdown = -1;

	network_fdw = fdwatch_create();
	memset(&tcp_eps, 0, sizeof(tcp_eps));
	for(l=0; l<sizeof(tcp_eps)/sizeof(tcp_eps[0]); l++) {
		tcp_eps[l].sock = SOCK_INVALID_HANDLE;
	}
	network_quit_flag = 0;
	os_condition_init(&network_write_cond);
	os_thread_begin(&network_read_handle, network_read_proc, NULL);
	os_thread_begin(&network_write_handle, network_write_proc, NULL);
}

void network_final()
{
	network_quit_flag = 1;
	os_condition_signal(&network_write_cond);
	os_thread_wait(network_read_proc, NULL);
	os_thread_wait(network_write_handle, NULL);
	os_condition_destroy(&network_write_cond);
	fdwatch_destroy(network_fdw);

	mempool_destroy(downbuf_pool);
	os_mutex_destroy(&downbuf_mutex);
}

int network_tcp_register(SOCK_ADDR* sa, NETWORK_ONACCEPT OnAccept, void* userptr)
{
	int index, ret;

	for(index=0; index<sizeof(tcp_eps)/sizeof(tcp_eps[0]); index++) {
		if(tcp_eps[index].sock==SOCK_INVALID_HANDLE) break;
	}
	if(index==sizeof(tcp_eps)/sizeof(tcp_eps[0])) return ERR_FULL;

	tcp_eps[index].sock = sock_bind(sa, SOCK_REUSEADDR);
	if(tcp_eps[index].sock==SOCK_INVALID_HANDLE) {
		return ERR_UNKNOWN;
	}
	if(sock_nonblock(tcp_eps[index].sock)!=0) {
		sock_unbind(tcp_eps[index].sock);
		tcp_eps[index].sock = SOCK_INVALID_HANDLE;
		return ERR_UNKNOWN;
	}

	memcpy(&tcp_eps[index].sa, sa, sizeof(*sa));
	tcp_eps[index].OnAccept = OnAccept;
	tcp_eps[index].userptr = userptr;

	ret = fdwatch_set(&tcp_eps[index].fdwitem, tcp_eps[index].sock, FDWATCH_READ, network_accept_event, &tcp_eps[index]);
	if(ret!=ERR_NOERROR) {
		sock_unbind(tcp_eps[index].sock);
		tcp_eps[index].sock = SOCK_INVALID_HANDLE;
		return ret;
	}

	ret = fdwatch_add(network_fdw, &tcp_eps[index].fdwitem);
	if(ret!=ERR_NOERROR) {
		sock_unbind(tcp_eps[index].sock);
		tcp_eps[index].sock = SOCK_INVALID_HANDLE;
		return ret;
	}

	return ERR_NOERROR;
}

int network_tcp_unregister(const SOCK_ADDR* sa)
{
	int index;

	for(index=0; index<sizeof(tcp_eps)/sizeof(tcp_eps[0]); index++) {
		if(tcp_eps[index].sock==SOCK_INVALID_HANDLE) continue;
		if(memcmp(&tcp_eps[index].sa, sa, sizeof(*sa))!=0) continue;
		break;
	}
	if(index==sizeof(tcp_eps)/sizeof(tcp_eps[0])) return ERR_NOT_FOUND;

	assert(network_ep_shutdown==-1);
	network_ep_shutdown = index;
	while(tcp_eps[index].OnAccept!=0) os_sleep(100);

	sock_unbind(tcp_eps[index].sock);

	tcp_eps[index].sock = SOCK_INVALID_HANDLE;
	tcp_eps[index].OnAccept = NULL;
	tcp_eps[index].userptr = NULL;

	return ERR_NOERROR;
}

NETWORK_HANDLE network_add(SOCK_HANDLE handle, NETWORK_EVENT* event, void* userptr)
{
	return NULL;
}

int network_del(NETWORK_HANDLE handle)
{
	return ERR_NOERROR;
}

unsigned int network_downbuf_size()
{
	return downbuf_size;
}

NETWORK_DOWNBUF* network_downbuf_alloc()
{
	return (NETWORK_DOWNBUF*)mempool_alloc(downbuf_pool);
}

void network_downbuf_free(NETWORK_DOWNBUF* downbuf)
{
	mempool_free(downbuf_pool, downbuf);
}

int network_send(NETWORK_HANDLE handle, NETWORK_DOWNBUF* downbufs[], unsigned int count)
{
	unsigned int l;

	os_mutex_lock(&downbuf_mutex);
	if(handle->downbuf_begin==NULL) {
		handle->downbuf_begin = downbufs[0];
		handle->downbuf_end = downbufs[0];
		handle->downbuf_end = NULL;
		handle->downbuf_cur = 0;
		l = 1;
	} else l = 0;

	for(; l<count; l++) {
		handle->downbuf_end->next = downbufs[l];
		handle->downbuf_end = downbufs[l];
	}
	os_mutex_unlock(&downbuf_mutex);

	return ERR_NOERROR;
}

int network_disconnect(NETWORK_HANDLE handle)
{
	return ERR_NOERROR;
}

unsigned int ZION_CALLBACK network_read_proc(void* arg)
{
	while(!network_quit_flag) {
		if(network_ep_shutdown>=0 && network_ep_shutdown<sizeof(tcp_eps)/sizeof(tcp_eps[0])) {
			int index;
			index = network_ep_shutdown;
			network_ep_shutdown = -1;

			fdwatch_remove(network_fdw, &tcp_eps[index].fdwitem);
			tcp_eps[index].OnAccept = NULL;
		}

		fdwatch_dispatch(network_fdw, 200);
	}
	return 0;
}

unsigned int ZION_CALLBACK network_write_proc(void* arg)
{
	while(!network_quit_flag) {
	}

	return 0;
}

void network_accept_event(FDWATCH_ITEM* item, int events)
{
	TCP_ENDPOINT* tcp_ep;
	SOCK_ADDR pname;
	SOCK_HANDLE clt;

	tcp_ep = (TCP_ENDPOINT*)item;

	clt = sock_accept(tcp_ep->sock, &pname);
	if(clt==SOCK_INVALID_HANDLE) return;

	tcp_ep->OnAccept(tcp_ep->userptr, clt, &pname);
}
