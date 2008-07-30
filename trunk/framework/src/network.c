#include <assert.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/sock.h"
#include "../inc/mempool.h"
#include "../inc/network.h"

#include "../inc/rlist.h"
#include "../inc/fdwatch.h"

// static config : begin
#define NETWORK_QUEUE_ITEM_MAX			10000
#define NETWORK_FDWAIT_TIME				200
// static config : end

#define QUEUE_ITEM_QUIT				0
#define QUEUE_ITEM_TCP_ACCEPT		1
#define QUEUE_ITEM_TCP_READ			2
#define QUEUE_ITEM_TCP_WRITE		3
#define QUEUE_ITEM_TCP_DISCONNECT	4

typedef struct TCP_ENDPOINT {
	FDWATCH_ITEM fdwitem;

	SOCK_HANDLE sock;
	SOCK_ADDR sa;
	NETWORK_ONACCEPT OnAccept;
	void* userptr;

	RLIST_ITEM	shutdown_item;
	int			ref_count;
} TCP_ENDPOINT;

typedef struct NETWORK_CONNECTION {
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
} NETWORK_CONNECTION;

typedef struct QUEUE_ITEM{
	int		type;
	union {
		TCP_ENDPOINT*		tcp_ep;
		NETWORK_CONNECTION*	tcp_conn;
		void*				data;
	};
} QUEUE_ITEM;

static unsigned int downbuf_size;
static MEMPOOL_HANDLE downbuf_pool;
static os_mutex_t downbuf_mutex;

static TCP_ENDPOINT tcp_eps[100];
static RLIST_HEAD tcp_shutdown_eps;
static os_mutex_t tcp_shutdown_mutex;

static FDWATCH_HANDLE network_fdw;

static QUEUE_ITEM event_queue[NETWORK_QUEUE_ITEM_MAX];
static unsigned int event_queue_cur;
static unsigned int event_queue_len;
static os_mutex_t event_queue_mutex;
static os_condition_t event_queue_cond;
static os_thread_t event_thread;

static int notify_thread_quit;
static os_thread_t notify_thread;

static unsigned int ZION_CALLBACK notify_proc(void* arg);
static unsigned int ZION_CALLBACK event_proc(void* arg);

static void event_post(int type, void* data);

static void accept_event(FDWATCH_ITEM* item, int events);
static void connection_event(FDWATCH_ITEM* item, int events);

static void event_opt_accept(TCP_ENDPOINT* ep);
static void event_opt_read(NETWORK_CONNECTION* conn);
static void event_opt_write(NETWORK_CONNECTION* conn);
static void event_opt_close(NETWORK_CONNECTION* conn);

void network_init(unsigned int size)
{
	int l;

	downbuf_size = size;
	downbuf_pool = mempool_create(sizeof(NETWORK_DOWNBUF)+size, 0);
	os_mutex_init(&downbuf_mutex);

	memset(&tcp_eps, 0, sizeof(tcp_eps));
	for(l=0; l<sizeof(tcp_eps)/sizeof(tcp_eps[0]); l++) {
		tcp_eps[l].sock = SOCK_INVALID_HANDLE;
	}
	rlist_init(&tcp_shutdown_eps);
	os_mutex_init(&tcp_shutdown_mutex);

	network_fdw = fdwatch_create();

	event_queue_cur = 0;
	event_queue_len = 0;
	os_mutex_init(&event_queue_mutex);
	os_condition_init(&event_queue_cond);
	os_thread_begin(&event_thread, event_proc, NULL);

	notify_thread_quit = 0;
	os_thread_begin(&notify_thread, notify_proc, NULL);	
}

void network_final()
{
	event_post(QUEUE_ITEM_QUIT, NULL);
	os_thread_wait(event_thread, NULL);

	notify_thread_quit = 1;
	os_thread_wait(event_thread, NULL);
	os_condition_destroy(&event_queue_cond);
	os_mutex_destroy(&event_queue_mutex);

	os_mutex_destroy(&tcp_shutdown_mutex);

	fdwatch_destroy(network_fdw);

	os_mutex_destroy(&downbuf_mutex);
	mempool_destroy(downbuf_pool);
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
	tcp_eps[index].ref_count = 1;

	ret = fdwatch_set(&tcp_eps[index].fdwitem, tcp_eps[index].sock, FDWATCH_READ, accept_event, &tcp_eps[index]);
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

	rlist_clear(&tcp_eps[index].shutdown_item, &tcp_eps[index]);
	rlist_push_back(&tcp_shutdown_eps, &tcp_eps[index].shutdown_item);
	while(tcp_eps[index].ref_count!=0) os_sleep(1000);

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

unsigned int ZION_CALLBACK notify_proc(void* arg)
{
	while(!notify_thread_quit) {
		os_mutex_lock(&tcp_shutdown_mutex);
		while(!rlist_empty(&tcp_shutdown_eps)) {
			TCP_ENDPOINT* ep;
			ep = (TCP_ENDPOINT*)rlist_get_userdata(rlist_front(&tcp_shutdown_eps));
			fdwatch_remove(network_fdw, &ep->fdwitem);
			atom_dec(&ep->ref_count);
			rlist_pop_front(&tcp_shutdown_eps);
		}
		os_mutex_unlock(&tcp_shutdown_mutex);

		fdwatch_dispatch(network_fdw, NETWORK_FDWAIT_TIME);
	}
	return 0;
}

unsigned int ZION_CALLBACK event_proc(void* arg)
{
	QUEUE_ITEM temp[sizeof(event_queue)/sizeof(event_queue[0])];
	unsigned int temp_len, l;
	int quit_flag;

	quit_flag = 0;
	os_mutex_lock(&event_queue_mutex);

	while(!quit_flag) {
		os_condition_wait(&event_queue_cond, &event_queue_mutex);

		if(event_queue_cur+event_queue_len>sizeof(event_queue)/sizeof(event_queue[0])) {
			unsigned int t;
			t = sizeof(event_queue)/sizeof(event_queue[0]) - event_queue_cur;
			memcpy(&temp[0], &event_queue[event_queue_cur], sizeof(event_queue[0])*t);
			memcpy(&temp[t], &event_queue[0], sizeof(event_queue[0])*(event_queue_len-t));
		} else {
			memcpy(&temp[0], &event_queue[event_queue_cur], sizeof(event_queue[0])*event_queue_len);
		}
		temp_len = event_queue_len;
		event_queue_cur = 0;
		event_queue_len = 0;

		os_mutex_unlock(&event_queue_mutex);

		for(l=0; l<temp_len; l++) {
			switch(temp[l].type) {
			case QUEUE_ITEM_QUIT:
				quit_flag = 1;
				break;
			case QUEUE_ITEM_TCP_ACCEPT:
				break;
			case QUEUE_ITEM_TCP_READ:
				break;
			case QUEUE_ITEM_TCP_WRITE:
				break;
			case QUEUE_ITEM_TCP_DISCONNECT:
				break;
			}
		}

		os_mutex_lock(&event_queue_mutex);
	}

	os_mutex_unlock(&event_queue_mutex);

	return 0;
}

void event_post(int type, void* data)
{
	int done = 0;
	do {
	os_mutex_lock(&event_queue_mutex);
	if(event_queue_len<sizeof(event_queue)/sizeof(event_queue[0])) {
		unsigned index;
		index = (event_queue_cur+event_queue_len)%(sizeof(event_queue)/sizeof(event_queue[0]));
		event_queue[index].type = type;
		event_queue[index].data = data;
		event_queue_len++;
		done = 1;
	}
	os_mutex_unlock(&event_queue_mutex);
	} while(!done);
}

void accept_event(FDWATCH_ITEM* item, int events)
{
	TCP_ENDPOINT* ep;
	ep = (TCP_ENDPOINT*)item;
	atom_inc(&ep->ref_count);
	event_post(QUEUE_ITEM_TCP_ACCEPT, ep);
}

void connection_event(FDWATCH_ITEM* item, int events)
{
	NETWORK_CONNECTION* conn;
	conn = (NETWORK_CONNECTION*)item;

	if(events&FDWATCH_WRITE) {
		event_post(QUEUE_ITEM_TCP_WRITE, conn);
	}
	if(events&FDWATCH_READ) {
	}
}

void event_opt_accept(TCP_ENDPOINT* ep)
{
	SOCK_ADDR pname;
	SOCK_HANDLE clt;

	for(;;) {
		clt = sock_accept(ep->sock, &pname);
		if(clt==SOCK_INVALID_HANDLE) break;

		ep->OnAccept(ep->userptr, clt, &pname);
	}

	atom_dec(&ep->ref_count);
}

void event_opt_read(NETWORK_CONNECTION* conn)
{
}

void event_opt_write(NETWORK_CONNECTION* conn)
{
}

void event_opt_close(NETWORK_CONNECTION* conn)
{
}
