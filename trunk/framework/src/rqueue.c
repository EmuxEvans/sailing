#include <assert.h>
#include <string.h>
/*
#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/sock.h"
#include "../inc/rqueue.h"

#include "../inc/rlist.h"
#include "../inc/fdwatch.h"
#include "../inc/mempool.h"
#include "../inc/applog.h"

#define RQUEUE_CLIENT_IOTIME		50
#define RETRIES_TIME				2

typedef struct BUFFER {
	RLIST_ITEM		ritem;
	RQUEUE_CLIENT*	client;
	RQUEUE_BUFFER	buf;
} BUFFER;

typedef struct SUB_CLIENT {
    FDWATCH_ITEM    witem;
	SOCK_HANDLE		sock;
    RQUEUE*         queue;
    int             index;
	BUFFER*			curbuf;
} SUB_CLIENT;

struct RQUEUE {
	SOCK_HANDLE		sock;
	FDWATCH_HANDLE	whandle;
	FDWATCH_ITEM	witem;
	SUB_CLIENT		clts[RQUEUE_MAX_CLIENT];
	RLIST_HEAD		bufs;
};

struct RQUEUE_CLIENT {
	FDWATCH_ITEM	witem;
	SOCK_HANDLE		sock;
	SOCK_ADDR		sa;
	RLIST_HEAD		bufs;
	time_t			ctime;
	int				wrlen;
	int				wrflag;
};

static RQUEUE			que_list[RQUEUE_MAX_COUNT];
static RQUEUE_CLIENT	clt_list[RQUEUE_CLIENT_MAX];
static int				clt_count;
static os_thread_t		clt_thread_id;
static int				clt_thread_flag;
static os_mutex_t		clt_mtx;
static FDWATCH_HANDLE	clt_fdw;
static RLIST_HEAD		clt_buf_list;
static MEMPOOL_HANDLE	buf_pool;

static void listener_event(FDWATCH_ITEM* item, int events);
static void subclient_event(FDWATCH_ITEM* item, int events);
static void client_event(FDWATCH_ITEM* item, int events);

static unsigned int ZION_CALLBACK workthread_proc(void* arg);

int rqueue_init()
{
	int i, ret;

	buf_pool = mempool_create(sizeof(BUFFER), 0);
	assert(buf_pool!=NULL);
	if(buf_pool==NULL) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rqueue_init: buf_pool==%p", buf_pool);
		return ERR_UNKNOWN;
	}

	memset(clt_list, 0, sizeof(clt_list));
	clt_count = 0;
	os_mutex_init(&clt_mtx);
	rlist_init(&clt_buf_list);

	ret = fdwatch_create(&clt_fdw);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rqueue(rqueue_init) : Failed to fdwatch_create(), ret=%d", ret);
		return ret;
	}
	clt_thread_flag = 0;
	ret = os_thread_begin(&clt_thread_id, workthread_proc, NULL);
	if(ret!=0) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rqueue(rqueue_init) : Failed to os_thread_begin(), ret=%d", ret);
		return ret;
	}

	memset(que_list, 0, sizeof(que_list));
	for(i=0; i<sizeof(que_list)/sizeof(que_list[0]); i++) {
		que_list[i].sock = SOCK_INVALID_HANDLE;
	}

	return ERR_UNKNOWN;
}

int rqueue_final()
{
	int i;

	for(i=0; i<sizeof(que_list)/sizeof(que_list[0]); i++) {
		if(que_list[i].sock==SOCK_INVALID_HANDLE) continue;
		DBGLOG(LOG_WARNING, MODULE_NAME, "rqueue_final: queue leak(index=%d)", i);
		rqueue_destroy(&que_list[i]);
	}

	clt_thread_flag = 1;
	os_thread_wait(clt_thread_id, NULL);
	fdwatch_break(clt_fdw);

	for(i=0; i<clt_count; i++) {
		if(clt_list[i].sock==SOCK_INVALID_HANDLE) continue;

		fdwatch_remove(clt_fdw, &clt_list[i].witem);
		sock_disconnect(clt_list[i].sock);
		sock_close(clt_list[i].sock);
		clt_list[i].sock = SOCK_INVALID_HANDLE;
	}

	fdwatch_destroy(clt_fdw);

	os_mutex_destroy(&clt_mtx);
	mempool_destroy(buf_pool);

	return ERR_NOERROR;
}

RQUEUE* rqueue_create(SOCK_ADDR* sa, RQUEUE_EVENT qevent)
{
	int index, ret, i;

	for(index=0; index<sizeof(que_list)/sizeof(que_list[0]); index++) {
		if(que_list[index].sock==SOCK_INVALID_HANDLE) break;
	}
	if(index==sizeof(que_list)/sizeof(que_list[0])) return NULL;

	que_list[index].sock = sock_bind(sa, SOCK_REUSEADDR);
	if(que_list[index].sock==SOCK_INVALID_HANDLE) return NULL;
	ret = fdwatch_create(&que_list[index].whandle);
	if(ret!=ERR_NOERROR) {
		sock_unbind(que_list[index].sock);
		que_list[index].sock = SOCK_INVALID_HANDLE;
		return NULL;
	}
	ret = fdwatch_set(&que_list[index].witem, que_list[index].sock, FDWATCH_READ|FDWATCH_ONCE, listener_event, &que_list[index]);
	if(ret!=ERR_NOERROR) {
		fdwatch_destroy(que_list[index].whandle);
		que_list[index].whandle = NULL;
		sock_unbind(que_list[index].sock);
		que_list[index].sock = SOCK_INVALID_HANDLE;
		return NULL;
	}
	ret = fdwatch_add(que_list[index].whandle, &que_list[index].witem);
	if(ret!=ERR_NOERROR) {
		fdwatch_destroy(que_list[index].whandle);
		que_list[index].whandle = NULL;
		sock_unbind(que_list[index].sock);
		que_list[index].sock = SOCK_INVALID_HANDLE;
		return NULL;
	}
	memset(que_list[index].clts, 0, sizeof(que_list[index].clts));
	for(i=0; i<sizeof(que_list[index].clts)/sizeof(que_list[index].clts[0]); i++) {
		que_list[index].clts[i].sock = SOCK_INVALID_HANDLE;
	}
	rlist_init(&que_list[index].bufs);

	return &que_list[index];
}

void rqueue_destroy(RQUEUE* queue)
{
	int i, ret;

	for(i=0; i<sizeof(queue->clts)/sizeof(queue->clts[0]); i++) {
		if(queue->clts[i].sock==SOCK_INVALID_HANDLE) continue;
		ret = fdwatch_remove(queue->whandle, &queue->clts[i].witem);
		if(ret!=ERR_NOERROR) {
			DBGLOG(LOG_ERROR, MODULE_NAME, "rqueue_destroy : Failed to fdwatch_remove(client), ret=%d", ret);
		}
		sock_disconnect(queue->clts[i].sock);
		sock_close(queue->clts[i].sock);
		queue->clts[i].sock = SOCK_INVALID_HANDLE;
	}

	ret = fdwatch_remove(queue->whandle, &queue->witem);
	if(ret!=ERR_NOERROR) {
		DBGLOG(LOG_ERROR, MODULE_NAME, "rqueue_destroy : Failed to fdwatch_remove(server), ret=%d", ret);
	}

	sock_unbind(queue->sock);
	fdwatch_destroy(queue->whandle);
	queue->whandle = NULL;

	queue->sock = SOCK_INVALID_HANDLE;
}

RQUEUE_CLIENT* rqueue_get(const SOCK_ADDR* sa)
{
	int i;
	RQUEUE_CLIENT* client = NULL;

	os_mutex_lock(&clt_mtx);
	for(i=0; i<clt_count; i++) {
		if(memcmp(&clt_list[i].sa, sa, sizeof(SOCK_ADDR))==0) {
			client = &clt_list[i];
			os_mutex_unlock(&clt_mtx);
			return client;
		}
	}

	if(clt_count<sizeof(clt_list)/sizeof(clt_list[0])) {
		client = &clt_list[clt_count];
		client->sock = SOCK_INVALID_HANDLE;
		memcpy(&client->sa, sa, sizeof(SOCK_ADDR));
		rlist_init(&client->bufs);
		client->ctime = 0;
		clt_count++;
	}

	os_mutex_unlock(&clt_mtx);
	return client;
}

RQUEUE_BUFFER* rqueue_read(RQUEUE* queue)
{
	RLIST_ITEM* item;
	item = rlist_pop_front(&queue->bufs);
	if(item==NULL) return NULL;
	return &(((BUFFER*)item)->buf);
}

int rqueue_write(RQUEUE_CLIENT* client, RQUEUE_BUFFER* buffer)
{
	BUFFER* buf;

	assert(client!=NULL);
	if(client==NULL) return ERR_INVALID_PARAMETER;

	buf = (BUFFER*)((char*)buffer - (unsigned int)(&((BUFFER*)NULL)->buf));
	os_mutex_lock(&clt_mtx);
	rlist_push_back(&clt_buf_list, &buf->ritem);
	buf->client = client;
	os_mutex_unlock(&clt_mtx);
	return ERR_NOERROR;
}

void rqueue_free(RQUEUE_BUFFER* buffer)
{
	BUFFER* buf;
	buf = (BUFFER*)((char*)buffer - (unsigned int)(&((BUFFER*)NULL)->buf));
	mempool_free(buf_pool, buf);
}

RQUEUE_BUFFER* rqueue_alloc(const void* buf, unsigned short len)
{
	BUFFER* buffer;
	if(buf!=NULL && len>sizeof(buffer->buf.data)) return NULL;
	buffer = (BUFFER*)mempool_alloc(buf_pool);
	if(buffer==NULL) return NULL;
	if(buf!=NULL) {
		memcpy(buffer->buf.data, buf, len);
		buffer->buf.len = len;
	} else {
		buffer->buf.len = 0;
	}
	return &buffer->buf;
}

int rqueue_process(RQUEUE* queue, int time)
{
	return fdwatch_dispatch(queue->whandle, time);
}

void listener_event(FDWATCH_ITEM* item, int events)
{
	SOCK_HANDLE sock;
	RQUEUE* queue;
	SUB_CLIENT* client;
	int index, ret;

	if((events&FDWATCH_READ)==0) return;

	sock = sock_accept(fdwatch_getfd(item), NULL);
	if(sock==SOCK_INVALID_HANDLE) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "ruqueue(listener_event) : Failed to sock_accept(), ret=%d", os_last_error());
		return;
	}

	queue = (RQUEUE*)fdwatch_getptr(item);
	for(index=0; index<sizeof(queue->clts)/sizeof(queue->clts[0]); index++) {
		if(queue->clts[index].sock==SOCK_INVALID_HANDLE) break;
	}
	if(index<sizeof(queue->clts)/sizeof(queue->clts[0])) {
		client = &queue->clts[index];
		ret = fdwatch_set(&client->witem, sock, FDWATCH_READ|FDWATCH_WRITE, subclient_event, client);
		if(ret==ERR_NOERROR) {
			ret = fdwatch_add(queue->whandle, &client->witem);
			if(ret==ERR_NOERROR) {
				client->sock = sock;
				client->queue = queue;
				client->index = index;
				client->curbuf = (BUFFER*)mempool_alloc(buf_pool);
				if(client->curbuf!=NULL) {
					client->curbuf->buf.len = 0;
					return;
				}
				fdwatch_remove(queue->whandle, &client->witem);
			}
		}
	}
	sock_disconnect(sock);
	sock_close(sock);
}

void subclient_event(FDWATCH_ITEM* item, int events)
{
	SUB_CLIENT* client;
	int ret;
	unsigned short len;

	if((events&FDWATCH_READ)==0) return;

	client = (SUB_CLIENT*)fdwatch_getptr(item);

	ret = sock_readbuf(fdwatch_getfd(item), &len, sizeof(len));
	if(ret==ERR_NOERROR) {
		ret = sock_readbuf(fdwatch_getfd(item), client->curbuf->buf.data, len);
		if(ret==ERR_NOERROR) {
			client->curbuf->buf.len = len;
			rlist_push_back(&client->queue->bufs, &client->curbuf->ritem);
			client->curbuf = (BUFFER*)mempool_alloc(buf_pool);
			if(client->curbuf!=NULL) {
				return;
			}
			SYSLOG(LOG_ERROR, MODULE_NAME, "rqueue(client_event) : Failed to mempool_alloc()");
		}
	}

	fdwatch_remove(client->queue->whandle, &client->witem);
	sock_disconnect(fdwatch_getfd(item));
	sock_close(fdwatch_getfd(item));

	if(client->curbuf!=NULL) mempool_free(buf_pool, client->curbuf);
	client->sock = SOCK_INVALID_HANDLE;
}

void client_event(FDWATCH_ITEM* item, int events)
{
	RQUEUE_CLIENT* client;

	client = (RQUEUE_CLIENT*)fdwatch_getptr(item);

	if((events&FDWATCH_READ)!=0) {
		fdwatch_remove(clt_fdw, item);
		sock_disconnect(client->sock);
		sock_close(client->sock);
		client->sock = SOCK_INVALID_HANDLE;
		return;
	}

	if((events&FDWATCH_WRITE)==0) {
		client->wrflag = 1;
	}
}

unsigned int ZION_CALLBACK workthread_proc(void* arg)
{
/// *
	int i, ret;
	BUFFER* buf;
	time_t curtime;

	while(clt_thread_flag==0) {

		os_mutex_lock(&clt_mtx);
		while(!rlist_empty(&clt_buf_list)) {
			buf = (BUFFER*)rlist_pop_front(&clt_buf_list);
			assert(buf!=NULL && buf->client!=NULL);

			rlist_push_back(&buf->client->bufs, &buf->ritem);
		}
		os_mutex_unlock(&clt_mtx);

		fdwatch_dispatch(clt_fdw, RQUEUE_CLIENT_IOTIME);

		curtime = time(NULL);
		for(i=0; i<sizeof(clt_list)/sizeof(clt_list[0]); i++) {
			if(rlist_empty(&clt_list[i].bufs)) continue;
			if(clt_list[i].sock!=SOCK_INVALID_HANDLE && !clt_list[i].wrflag) continue;

			if(clt_list[i].sock==SOCK_INVALID_HANDLE) {
				while(!rlist_empty(&clt_list[i].bufs)) {
					mempool_free(buf_pool, rlist_pop_front(&clt_list[i].bufs));
				}
				if(clt_list[i].ctime>curtime) continue;

				clt_list[i].sock = sock_connect(&clt_list[i].sa, SOCK_NONBLOCK);
				if(clt_list[i].sock==SOCK_INVALID_HANDLE) continue;

				ret = fdwatch_set(&clt_list[i].witem, clt_list[i].sock, FDWATCH_READ|FDWATCH_WRITE, client_event, &clt_list[i]);
				if(ret==ERR_NOERROR) {
					ret = fdwatch_add(clt_fdw, &clt_list[i].witem);
					if(ret==ERR_NOERROR) {
						clt_list[i].wrlen = 0;
						clt_list[i].wrflag = 1;
						continue;
					} else {
						SYSLOG(LOG_ERROR, MODULE_NAME, "rqueue(workthread_proc) : fdwatch_add(), ret = %d", ret);
					}
				} else {
					SYSLOG(LOG_ERROR, MODULE_NAME, "rqueue(workthread_proc) : fdwatch_set(), ret = %d", ret);
				}

				sock_disconnect(clt_list[i].sock);
				sock_close(clt_list[i].sock);
				clt_list[i].sock = SOCK_INVALID_HANDLE;
				clt_list[i].ctime = curtime += RETRIES_TIME;
				continue;
			}

			while(!rlist_empty(&clt_list[i].bufs)) {
				buf = (BUFFER*)rlist_front(&clt_list[i].bufs);
				ret = sock_write(clt_list[i].sock, buf->buf.data + clt_list[i].wrlen, buf->buf.len - clt_list[i].wrlen);
				if(ret==ERR_UNKNOWN) {
					fdwatch_remove(clt_fdw, &clt_list[i].witem);
					sock_disconnect(clt_list[i].sock);
					sock_close(clt_list[i].sock);
					clt_list[i].sock = SOCK_INVALID_HANDLE;
					continue;
				}
				if(ret==0) break;
				clt_list[i].wrlen += ret;
				if(clt_list[i].wrlen==buf->buf.len) {
					rlist_pop_front(&clt_list[i].bufs);
					mempool_free(buf_pool, buf);
					clt_list[i].wrlen = 0;
				}
			}
		}
	}
* /
	return 0;
}

*/