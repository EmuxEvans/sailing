#include <assert.h>
#include <string.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/sock.h"
#include "../inc/rqueue.h"
#include "../inc/rlist.h"
#include "../inc/mempool.h"
#include "../inc/network.h"
#include "../inc/hashmap.h"

// static config : begin
#define RQUEUE_RECVBUF_SIZE			(5*1024-4)
#define RQUEUE_BUFFER_SIZE			(5*1024*1024)
// static config : end

struct RQUEUE {
	HASHMAP_ITEM	hmitem;
	SOCK_ADDR		queue_addr;
	os_sem_t		queue_sem;

	char			buffer[RQUEUE_BUFFER_SIZE];
	unsigned int	buffer_cur;
	unsigned int	buffer_len;
	os_mutex_t		buffer_mutex;

	NETWORK_HANDLE	clt_list[1000];
};

struct RQUEUE_CLIENT {
	HASHMAP_ITEM	hmitem;
	RQUEUE*			queue;
	NETWORK_HANDLE	clt_handle;
	os_mutex_t		clt_mutex;
};

static void rqueue_accept_event(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname);
static void rqueue_ondata_event(NETWORK_HANDLE handle, void* userptr);
static void rqueue_ondisconnect_event(NETWORK_HANDLE handle, void* userptr);

static HASHMAP_HANDLE	rqueue_map;
static os_mutex_t		rqueue_mutex;
static RQUEUE			rqueue_list[10];
static int				rqueue_count;

static HASHMAP_HANDLE	rqueue_clt_map;
static os_mutex_t		rqueue_clt_mutex;
static RQUEUE_CLIENT	rqueue_clt_list[100];
static int				rqueue_clt_count;

static NETWORK_EVENT	rqueue_network_event;

void rqueue_init()
{
	int i;

	rqueue_map = hashmap_create(10, NULL, 0);
	os_mutex_init(&rqueue_mutex);
	rqueue_count = 0;
	rqueue_clt_map = hashmap_create(10, NULL, 0);
	os_mutex_init(&rqueue_clt_mutex);
	rqueue_clt_count = 0;

	memset(rqueue_list, 0, sizeof(rqueue_list));
	for(i=0; i<sizeof(rqueue_list)/sizeof(rqueue_list[0]); i++) {
		os_sem_init(&rqueue_list[i].queue_sem, 0);
		os_mutex_init(&rqueue_list[i].buffer_mutex);
	}

	memset(&rqueue_clt_list, 0, sizeof(rqueue_clt_list));
	for(i=0; i<sizeof(rqueue_clt_list)/sizeof(rqueue_clt_list[0]); i++) {
		os_mutex_init(&rqueue_clt_list[i].clt_mutex);
	}

	rqueue_network_event.OnConnect = NULL;
	rqueue_network_event.OnData = rqueue_ondata_event;
	rqueue_network_event.OnDisconnect = rqueue_ondisconnect_event;
	rqueue_network_event.recvbuf_pool = mempool_create("RQUEUE_RECVBUF", RQUEUE_RECVBUF_SIZE, 0);
	rqueue_network_event.recvbuf_buf = NULL;
	rqueue_network_event.recvbuf_max = RQUEUE_RECVBUF_SIZE;
}

void rqueue_final()
{
	int i;

	hashmap_destroy(rqueue_map);
	os_mutex_destroy(&rqueue_mutex);
	hashmap_destroy(rqueue_clt_map);
	os_mutex_init(&rqueue_clt_mutex);

	for(i=0; i<sizeof(rqueue_list)/sizeof(rqueue_list[0]); i++) {
		os_sem_destroy(&rqueue_list[i].queue_sem);
		os_mutex_destroy(&rqueue_list[i].buffer_mutex);
	}

	for(i=0; i<sizeof(rqueue_clt_list)/sizeof(rqueue_clt_list[0]); i++) {
		os_mutex_destroy(&rqueue_clt_list[i].clt_mutex);
	}

	mempool_destroy(rqueue_network_event.recvbuf_pool);
}

RQUEUE* rqueue_create(const SOCK_ADDR* sa, int flags, const char* path)
{
	RQUEUE* queue;
	if(rqueue_count==sizeof(rqueue_list)/sizeof(rqueue_list[0])) return NULL;
	queue = &rqueue_list[rqueue_count];
	memcpy(&queue->queue_addr, sa, sizeof(SOCK_ADDR));
	if(network_tcp_register(&queue->queue_addr, rqueue_accept_event, queue)!=ERR_NOERROR) return NULL;

	os_mutex_lock(&rqueue_mutex);
	hashmap_add(rqueue_map, &queue->hmitem, &queue->queue_addr, sizeof(queue->queue_addr));
	os_mutex_unlock(&rqueue_mutex);

	return queue;
}

void rqueue_destroy(RQUEUE* queue)
{
	int l;

	if(network_tcp_unregister(&queue->queue_addr)!=ERR_NOERROR) {
	}

	os_mutex_lock(&queue->buffer_mutex);
	for(l=0; l<sizeof(queue->clt_list)/sizeof(queue->clt_list[0]); l++) {
		if(queue->clt_list[l]==NULL) continue;
		network_disconnect(queue->clt_list[l]);
	}
	os_mutex_lock(&queue->buffer_mutex);

	do {
		os_sleep(50);
		for(l=0; l<sizeof(queue->clt_list)/sizeof(queue->clt_list[0]); l++) {
			if(queue->clt_list[l]!=NULL) break;
		}
	} while(l<sizeof(queue->clt_list)/sizeof(queue->clt_list[0]));

	os_mutex_lock(&rqueue_mutex);
	hashmap_erase(rqueue_map, &queue->hmitem);
	os_mutex_unlock(&rqueue_mutex);
}

int rqueue_read(RQUEUE* queue, void* buf, int size)
{
	return 0;
}

RQUEUE_CLIENT* rqueue_get(const SOCK_ADDR* sa)
{
	return NULL;
}

int rqueue_write(RQUEUE_CLIENT* client, void* buf, int len)
{
	return 0;
}

void rqueue_accept_event(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname)
{
	RQUEUE* queue = (RQUEUE*)userptr;
	int i;

	for(i=0; i<sizeof(queue->clt_list)/sizeof(queue->clt_list[0]); i++) {
		if(queue->clt_list[0]==NULL) break;
	}

	if(i<sizeof(queue->clt_list)/sizeof(queue->clt_list[0])) {
		os_mutex_lock(&queue->buffer_mutex);
		queue->clt_list[i] = network_add(sock, &rqueue_network_event, queue);
		os_mutex_unlock(&queue->buffer_mutex);
		if(queue->clt_list[i]!=NULL) return;
	}

	sock_disconnect(sock);
	sock_close(sock);
}

void rqueue_ondata_event(NETWORK_HANDLE handle, void* userptr)
{
	RQUEUE* queue = (RQUEUE*)userptr;
	unsigned int len; 
	char buf[RQUEUE_RECVBUF_SIZE];

	for(;;) {
		if(network_recvbuf_len(handle)<sizeof(len)) break;
		network_recvbuf_get(handle, &len, 0, sizeof(len));
		if(len>RQUEUE_RECVBUF_SIZE) { network_disconnect(handle); break; }
		if(network_recvbuf_len(handle)<len) break;
		network_recvbuf_get(handle, buf, sizeof(len), len-sizeof(len));

		//
		// put (buf, len-4) to queue
		//

		network_recvbuf_commit(handle, len);
	}
}

void rqueue_ondisconnect_event(NETWORK_HANDLE handle, void* userptr)
{
	int i;
	RQUEUE* queue = (RQUEUE*)userptr;

	os_mutex_lock(&queue->buffer_mutex);
	for(i=0; i<sizeof(queue->clt_list)/sizeof(queue->clt_list[0]); i++) {
		if(queue->clt_list[i]==handle) {
			queue->clt_list[i] = NULL;
			break;
		}
	}
	os_mutex_unlock(&queue->buffer_mutex );

	sock_close(network_get_sock(handle));
	network_del(handle);
}
