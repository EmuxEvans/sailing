#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/sock.h"
#include "../inc/rqueue.h"
#include "../inc/rlist.h"
#include "../inc/mempool.h"
#include "../inc/network.h"
#include "../inc/hashmap.h"
#include "../inc/applog.h"
#include "../inc/misc.h"

// static config : begin
#define RQUEUE_RECVBUF_SIZE			(5*1024)
#define RQUEUE_BUFFER_SIZE			(5*1024*1024)
#define RQUEUE_TIMEOUT_CONNECT		(1*1000)
// static config : end

struct RQUEUE {
	HASHMAP_ITEM	hmitem;
	int				queue_enable;
	SOCK_ADDR		queue_addr;
	unsigned int	queue_flags;
	os_condition_t	queue_cond;

	char*			buffer;
	unsigned int	buffer_cur;
	unsigned int	buffer_len;
	unsigned int	buffer_max;
	os_mutex_t		buffer_mutex;
	os_condition_t	buffer_cond;

	NETWORK_HANDLE	clt_list[1000];
};

struct RQUEUE_CLIENT {
	HASHMAP_ITEM	hmitem;
	RQUEUE*			queue;
	NETWORK_HANDLE	clt_handle;
	os_mutex_t		clt_mutex;
	SOCK_ADDR		clt_sa;
	os_time_t		clt_timeout;

	char			recvbuf[10];
};

static int rqueue_internal_write(RQUEUE* queue, const void* buf, unsigned int size);
static int rqueue_internal_connect(RQUEUE_CLIENT* client, const SOCK_ADDR* sa);

static void rqueue_buffer_read(RQUEUE* queue, void* buf, unsigned int size);
static void rqueue_buffer_write(RQUEUE* queue, const void* buf, unsigned int size);

static void rqueue_accept_event(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname);
static void rqueue_ondata_event(NETWORK_HANDLE handle, void* userptr);
static void rqueue_ondisconnect_event(NETWORK_HANDLE handle, void* userptr);

static void rqueue_clt_ondata_event(NETWORK_HANDLE handle, void* userptr);
static void rqueue_clt_ondisconnect_event(NETWORK_HANDLE handle, void* userptr);

static unsigned int ZION_CALLBACK rqueue_thread_proc(void* arg);

static HASHMAP_HANDLE	rqueue_map;
static os_mutex_t		rqueue_mutex;
static RQUEUE			rqueue_list[10];
static int				rqueue_count;

static HASHMAP_HANDLE	rqueue_clt_map;
static RQUEUE_CLIENT	rqueue_clt_list[100];
static int				rqueue_clt_count;

static NETWORK_EVENT	rqueue_network_event;

static int				rqueue_thread_flag;
static os_thread_t		rqueue_thread_tid;

void rqueue_init()
{
	int i;

	rqueue_map = hashmap_create(10, NULL, 0);
	os_mutex_init(&rqueue_mutex);
	rqueue_count = 0;
	rqueue_clt_map = hashmap_create(10, NULL, 0);
	rqueue_clt_count = 0;

	memset(rqueue_list, 0, sizeof(rqueue_list));
	for(i=0; i<sizeof(rqueue_list)/sizeof(rqueue_list[0]); i++) {
		os_condition_init(&rqueue_list[i].queue_cond);
		os_mutex_init(&rqueue_list[i].buffer_mutex);
		os_condition_init(&rqueue_list[i].buffer_cond);
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

	rqueue_thread_flag = 1;
	os_thread_begin(&rqueue_thread_tid, rqueue_thread_proc, NULL);
}

void rqueue_final()
{
	int i, count;

	rqueue_thread_flag = 0;
	os_thread_wait(rqueue_thread_tid, NULL);

	for(i=0; i<rqueue_count; i++) {
		char tmp[100];
		if(!rqueue_list[i].queue_enable) continue;
		sock_addr2str(&rqueue_list[i].queue_addr, tmp);
		SYSLOG(LOG_ERROR, MODULE_NAME, "rqueue_final: %s not destroy", tmp);
		rqueue_destroy(&rqueue_list[i]);
	}

	do {
		count = 0;

		os_mutex_lock(&rqueue_mutex);
		for(i=0; i<rqueue_clt_count; i++) {
			if(rqueue_clt_list[i].clt_handle==NULL) continue;
			network_disconnect(rqueue_clt_list[i].clt_handle);
			count++;
		}
		os_mutex_unlock(&rqueue_mutex);
		if(count>0) os_sleep(50);

	} while(count>0);

	hashmap_destroy(rqueue_map);
	os_mutex_destroy(&rqueue_mutex);
	hashmap_destroy(rqueue_clt_map);

	for(i=0; i<sizeof(rqueue_list)/sizeof(rqueue_list[0]); i++) {
		if(rqueue_list[i].buffer) free(rqueue_list[i].buffer);
		os_condition_destroy(&rqueue_list[i].queue_cond);
		os_mutex_destroy(&rqueue_list[i].buffer_mutex);
		os_condition_destroy(&rqueue_list[i].buffer_cond);
	}

	for(i=0; i<sizeof(rqueue_clt_list)/sizeof(rqueue_clt_list[0]); i++) {
		os_mutex_destroy(&rqueue_clt_list[i].clt_mutex);
	}

	mempool_destroy(rqueue_network_event.recvbuf_pool);
}

RQUEUE* rqueue_create(const SOCK_ADDR* sa, unsigned int bufsize, unsigned int flags, const char* path)
{
	RQUEUE* queue = NULL;
	RQUEUE_CLIENT* client;
	char* buffer;

	if(bufsize==0) bufsize = RQUEUE_BUFFER_SIZE;
	buffer = malloc(bufsize);
	if(buffer==NULL) return NULL;

	os_mutex_lock(&rqueue_mutex);

	if(rqueue_count<sizeof(rqueue_list)/sizeof(rqueue_list[0]) && rqueue_clt_count<sizeof(rqueue_clt_list)/sizeof(rqueue_clt_list[0])) {
		memcpy(&rqueue_list[rqueue_count].queue_addr, sa, sizeof(SOCK_ADDR));
		rqueue_list[rqueue_count].queue_flags = flags;
		rqueue_list[rqueue_count].buffer = buffer;
		rqueue_list[rqueue_count].buffer_max = bufsize;

		os_mutex_lock(&rqueue_list[rqueue_count].buffer_mutex);
		if(network_tcp_register(&rqueue_list[rqueue_count].queue_addr, rqueue_accept_event, &rqueue_list[rqueue_count])!=ERR_NOERROR) {
			queue = &rqueue_list[rqueue_count];
			client = &rqueue_clt_list[rqueue_clt_count];

			client->queue = queue;
			client->clt_handle = NULL;
			memcpy(&client->clt_sa, sa, sizeof(*sa));

			queue->queue_enable = 1;
			hashmap_add(rqueue_map, &queue->hmitem, &queue->queue_addr, sizeof(queue->queue_addr));
			hashmap_add(rqueue_clt_map, &client->hmitem, &client->clt_sa, sizeof(client->clt_sa));
			rqueue_count++;
			rqueue_clt_count++;
		}
		os_mutex_unlock(&rqueue_list[rqueue_count].buffer_mutex);
	}

	os_mutex_unlock(&rqueue_mutex);

	if(queue==NULL) free(buffer);
	return queue;
}

void rqueue_destroy(RQUEUE* queue)
{
	int l;

	os_mutex_lock(&queue->buffer_mutex);
	queue->queue_enable = 0;
	os_mutex_unlock(&queue->buffer_mutex);

	network_tcp_unregister(&queue->queue_addr);

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

int rqueue_read(RQUEUE* queue, void* buf, unsigned int* size)
{
	int ret;
	unsigned int r_size;

	os_mutex_lock(&queue->buffer_mutex);

	for(;;) {
		if(!queue->queue_enable) {
			ret = ERR_INVALID_HANDLE;
			break;
		}

		if(queue->buffer_len>sizeof(r_size)) {
			rqueue_buffer_read(queue, &r_size, sizeof(r_size));
			if(r_size>*size) {
				ret = ERR_NOT_ENOUGH;
				break;
			}
			if(queue->buffer_len<sizeof(r_size)+r_size) {
				ret = ERR_INVALID_DATA;
				break;
			}
			queue->buffer_cur = (queue->buffer_cur+sizeof(r_size)) % queue->buffer_max;
			rqueue_buffer_read(queue, buf, r_size);
			queue->buffer_cur = (queue->buffer_cur+r_size) % queue->buffer_max;
			ret = ERR_NOERROR;
			break;
		}
		if(queue->queue_flags&RQUEUE_DATA_UNSAFE) {
			ret = ERR_FULL;
			break;
		}

		os_condition_wait(&queue->buffer_cond, &queue->buffer_mutex);
	}

	os_mutex_unlock(&queue->buffer_mutex);

	if(ret==ERR_NOERROR) *size = r_size;
	return ret;
}

RQUEUE_CLIENT* rqueue_get(const SOCK_ADDR* sa)
{
	RQUEUE_CLIENT* client;

	os_mutex_lock(&rqueue_mutex);
	client = (RQUEUE_CLIENT*)hashmap_get(rqueue_clt_map, sa, sizeof(*sa));
	if(client==NULL) {
		if(rqueue_internal_connect(&rqueue_clt_list[rqueue_clt_count], sa)==ERR_NOERROR) {
			client = &rqueue_clt_list[rqueue_clt_count++];
			memcpy(&client->clt_sa, sa, sizeof(*sa));
			hashmap_add(rqueue_clt_map, &client->hmitem, &client->clt_sa, sizeof(client->clt_sa));
		}
	}
	os_mutex_unlock(&rqueue_mutex);

	return client;
}

int rqueue_write(RQUEUE_CLIENT* client, const void* buf, unsigned int len)
{
	int ret;
	unsigned int count, slen;
	NETWORK_DOWNBUF* downbufs[10];

	if(len<=0 && len+4>RQUEUE_RECVBUF_SIZE) {
		return ERR_INVALID_PARAMETER;
	}

	if(client->queue) {
		return rqueue_internal_write(client->queue, buf, len);
	}

	slen = len + sizeof(slen);
	count = network_downbufs_alloc(downbufs, sizeof(downbufs)/sizeof(downbufs[0]), slen);
	if(count==0) return ERR_UNKNOWN;
	network_downbufs_fill(downbufs, count, 0, &slen, sizeof(slen));
	network_downbufs_fill(downbufs, count, sizeof(slen), buf, len);

	os_mutex_lock(&client->clt_mutex);
	if(client->clt_handle) {
		ret = network_send(client->clt_handle, downbufs, count);
	} else {
		ret = ERR_UNKNOWN;
	}
	os_mutex_unlock(&client->clt_mutex);

	if(ret!=ERR_NOERROR) {
		network_downbufs_free(downbufs, count);
	}

	return ret;
}

int rqueue_internal_write(RQUEUE* queue, const void* buf, unsigned int size)
{
	int ret;

	os_mutex_lock(&queue->buffer_mutex);

	for(;;) {
		if(!queue->queue_enable) {
			ret = ERR_INVALID_HANDLE;
			break;
		}

		if(queue->buffer_len+sizeof(size)+size<=queue->buffer_max) {
			rqueue_buffer_write(queue, &size, sizeof(size));
			queue->buffer_len += sizeof(size);
			rqueue_buffer_write(queue, buf, size);
			queue->buffer_len += size;
			ret = ERR_NOERROR;
			break;
		}
		if(queue->queue_flags&RQUEUE_DATA_UNSAFE) {
			ret = ERR_FULL;
			break;
		}

		os_condition_wait(&queue->buffer_cond, &queue->buffer_mutex);
	}

	os_mutex_unlock(&queue->buffer_mutex);

	if(ret==ERR_NOERROR) os_condition_broadcast(&queue->queue_cond);

	return ret;
}

int rqueue_internal_connect(RQUEUE_CLIENT* client, const SOCK_ADDR* sa)
{
	SOCK_HANDLE sock;
	NETWORK_HANDLE handle;
	NETWORK_EVENT event;

	sock = sock_connect(sa, 0);
	if(sock==SOCK_INVALID_HANDLE) return ERR_UNKNOWN;

	event.OnConnect = NULL;
	event.OnData = rqueue_clt_ondata_event;
	event.OnDisconnect = rqueue_clt_ondisconnect_event;
	event.recvbuf_pool = NULL;
	event.recvbuf_buf = client->recvbuf;
	event.recvbuf_max = sizeof(client->recvbuf);
	handle = network_add(sock, &event, client);
	if(handle==NULL) {
		sock_disconnect(sock);
		sock_close(sock);
		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

void rqueue_buffer_read(RQUEUE* queue, void* buf, unsigned int size)
{
	unsigned int k = queue->buffer_max - queue->buffer_cur;
	if(k<size) {
		memcpy(buf, queue->buffer+queue->buffer_cur, k);
		memcpy((char*)buf+k, queue->buffer, size-k);
	} else {
		memcpy(buf, queue->buffer+queue->buffer_cur, size);
	}
}

void rqueue_buffer_write(RQUEUE* queue, const void* buf, unsigned int size)
{
	unsigned int k = queue->buffer_max - (queue->buffer_cur+queue->buffer_len);
	if(k<size) {
		memcpy(queue->buffer+queue->buffer_cur, buf, k);
		memcpy(queue->buffer, (char*)buf+k, size-k);
	} else {
		memcpy(queue->buffer+queue->buffer_cur, buf, size);
	}
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
		rqueue_internal_write(queue, buf, len);
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

void rqueue_clt_ondata_event(NETWORK_HANDLE handle, void* userptr)
{
	SYSLOG(LOG_ERROR, MODULE_NAME, "rqueue_clt_ondata_event: impossible, receive invalid data");
	network_disconnect(handle);
}

void rqueue_clt_ondisconnect_event(NETWORK_HANDLE handle, void* userptr)
{
	RQUEUE_CLIENT* client;

	client = (RQUEUE_CLIENT*)userptr;

	os_time_get(&client->clt_timeout);
	client->clt_timeout += RQUEUE_TIMEOUT_CONNECT;

	os_mutex_lock(&rqueue_mutex);
	client->clt_handle = NULL;
	os_mutex_unlock(&rqueue_mutex);	

	sock_close(network_get_sock(handle));
	network_del(handle);
}

static RQUEUE_CLIENT clt_list[sizeof(rqueue_clt_list)/sizeof(rqueue_clt_list[0])];

unsigned int ZION_CALLBACK rqueue_thread_proc(void* arg)
{
	int i, clt_count;
	os_time_t curtime;

	while(rqueue_thread_flag) {
		os_sleep(100);

		os_mutex_lock(&rqueue_mutex);
		memcpy(clt_list, rqueue_clt_list, sizeof(rqueue_clt_list));
		clt_count = rqueue_clt_count;
		os_mutex_unlock(&rqueue_mutex);

		os_time_get(&curtime);

		for(i=0; i<clt_count; i++) {
			if(clt_list[i].clt_handle!=NULL) continue;
			if(clt_list[i].queue!=NULL) continue;
			if(clt_list[i].clt_timeout>curtime) continue;

			rqueue_clt_list[i].clt_timeout = curtime + RQUEUE_TIMEOUT_CONNECT;
			rqueue_internal_connect(&rqueue_clt_list[i], &rqueue_clt_list[i].clt_sa);
		}
	}

	return 0;
}

