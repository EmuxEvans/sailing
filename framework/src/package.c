#include <assert.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/rlist.h"
#include "../inc/fdwatch.h"
#include "../inc/sock.h"
#include "../inc/package.h"
#include "../inc/mempool.h"
#include "../inc/threadpool.h"

#define NBUF_SIZE		512

#define RDFLAG_EMPTY		0
#define RDFLAG_READING		1

#define WRFLAG_FULL			0
#define WRFLAG_WRITING		1
#define WRFLAG_WRITABLE		2
#define WRFLAG_WRITE_AGAIN	3

#define SDFLAG_AVAILABLE	0
#define SDFLAG_WAIT			1
#define SDFLAG_CLOSING		2
#define SDFLAG_CLOSED		3

#define EPTYPE_PASSIVE					0
#define EPTYPE_ACTIVE					1

typedef struct ENDPOINT {
	FDWATCH_ITEM	fdwi;
	SOCK_ADDR		sa;
	RLIST_HEAD		conns;
	PACKAGE_NOTIFY	notify;
	SOCK_HANDLE		sock;

	os_mutex_t		ep_mutex;
	os_mutex_t		cn_mutex;

	RLIST_ITEM		remove;
	int				removable;
	int				type;
} ENDPOINT;

typedef struct NBUF {
	RLIST_ITEM		item;
	unsigned int	len;
	char			buf[NBUF_SIZE];
} NBUF;

typedef struct PACKAGE_CONNECTION {
	FDWATCH_ITEM	witem;
	SOCK_HANDLE		sock;
	ENDPOINT*		ep;
	RLIST_ITEM		eitem;
	void*			userptr;
	int				rd_flag;
	int				wr_flag;
	int				sd_flag;

	RLIST_HEAD		dbufs;
	RLIST_HEAD		ubufs;
	unsigned int	recvlen;
	char			recvbuf[1];
} PACKAGE_CONNECTION;

static ENDPOINT			ep_list[100];
static RLIST_HEAD		ep_remove;
static os_thread_t		iot_handle;
static int				iot_quitflag;
static FDWATCH_HANDLE	fdw_handle;
static MEMPOOL_HANDLE	con_pool;
static unsigned int		conn_recvbuf_len;
static MEMPOOL_HANDLE	buf_pool;

static unsigned int ZION_CALLBACK iot_process(void* arg);
static void iot_connect(FDWATCH_ITEM* item, int events);
static void iot_accept(FDWATCH_ITEM* item, int events);
static void iot_client(FDWATCH_ITEM* item, int events);

static void iot_onconnect(void* arg);
static void iot_ondata(void* arg);
static void iot_ondisconnect(void* arg);

int ZION_API package_init(unsigned int recvbuf_len)
{
	int ret;
	memset(&ep_list, 0, sizeof(ep_list));
	conn_recvbuf_len = recvbuf_len;
	rlist_init(&ep_remove);
	ret = fdwatch_create(&fdw_handle);
	if(ret!=0) return ret;
	con_pool = mempool_create(sizeof(PACKAGE_CONNECTION)+conn_recvbuf_len, 0);
	buf_pool = mempool_create(sizeof(NBUF), 0);
	assert(con_pool!=NULL && buf_pool!=NULL);
	if(con_pool==NULL || buf_pool==NULL) return ERR_UNKNOWN;
	ret = os_thread_begin(&iot_handle, iot_process, NULL);
	if(ret!=0) return ret;
	return ERR_NOERROR;
}

void ZION_API package_final()
{
	iot_quitflag = 1;
	os_thread_wait(iot_handle, NULL);
	os_thread_close(iot_handle);
	mempool_destroy(con_pool);
	mempool_destroy(buf_pool);
	fdwatch_destroy(fdw_handle);
}

int ZION_API package_connect(SOCK_ADDR* addr, PACKAGE_NOTIFY notify)
{
	int ret;
	unsigned int index;
	SOCK_ADDR local;

	for(index=0; index<sizeof(ep_list)/sizeof(ep_list[0]); index++) {
		if(ep_list[index].sa.port==0) break;
	}
	if(index==sizeof(ep_list)/sizeof(ep_list[0])) return ERR_FULL;

	ep_list[index].sock = sock_connect(addr, SOCK_REUSEADDR);
	if(ep_list[index].sock==SOCK_INVALID_HANDLE) return ERR_UNKNOWN;
	sock_sockname(ep_list[index].sock, &local);

	fdwatch_set(&ep_list[index].fdwi, ep_list[index].sock, FDWATCH_ONCE | FDWATCH_WRITE, iot_connect, &ep_list[index]);
	memcpy(&ep_list[index].sa, &local, sizeof(local));
	rlist_init(&ep_list[index].conns);
	ep_list[index].type = EPTYPE_ACTIVE;
	ep_list[index].notify = notify;
	os_mutex_init(&ep_list[index].ep_mutex);
	os_mutex_init(&ep_list[index].cn_mutex);
	rlist_clear(&ep_list[index].remove, &ep_list[index]);
	ep_list[index].removable = 0;

	ret = fdwatch_add(fdw_handle, &ep_list[index].fdwi);
	if(ret!=0) {
		sock_unbind(ep_list[index].sock);
		os_mutex_destroy(&ep_list[index].ep_mutex);
		os_mutex_destroy(&ep_list[index].cn_mutex);
		memset(&ep_list[index], 0, sizeof(ep_list[index]));
		return ret;
	}

	return ERR_NOERROR;
}

void ZION_API package_close(PACKAGE_HANDLE handle, int flags)
{
	NBUF *buf;
	PACKAGE_CONNECTION *conn = handle;

	if (conn == NULL) return;
	if (handle->ep->sa.port == 0) return;

	switch (flags) {
		case PACKAGE_CLOSE_NORMAL:
		case PACKAGE_CLOSE_INTHREAD:
			if (conn->sd_flag == SDFLAG_AVAILABLE) {
				conn->ep->notify(conn, PACKAGE_ONDISCONNECT, NULL, 0);
			}
		case PACKAGE_CLOSE_NOTCONNECT:
		default:
			conn->sd_flag = SDFLAG_CLOSING;
	}

	while (conn->rd_flag != RDFLAG_EMPTY) os_thread_switch();
	while (conn->wr_flag != WRFLAG_FULL) os_thread_switch();
	while (conn->sd_flag != SDFLAG_CLOSED) os_thread_switch();

	if ((((RLIST_ITEM*)&conn->witem)->prev != NULL) && (((RLIST_ITEM*)&conn->witem)->next != NULL)) {
		fdwatch_remove(fdw_handle, &conn->witem);
	}

	while (!rlist_empty(&conn->ubufs)) {
		buf = (NBUF*)rlist_front(&conn->ubufs);
		rlist_remove(&conn->ubufs, rlist_front(&conn->ubufs));
		mempool_free(buf_pool, buf);
	}

	while (!rlist_empty(&conn->dbufs)) {
		buf = (NBUF*)rlist_front(&conn->dbufs);
		rlist_remove(&conn->dbufs, rlist_front(&conn->dbufs));
		mempool_free(buf_pool, buf);
	}

	sock_disconnect(conn->sock);
	sock_close(conn->sock);

	rlist_remove(&handle->ep->conns, &handle->eitem);

	if (handle->ep->type) {
		rlist_push_back(&ep_remove, &handle->ep->remove);
		while(!handle->ep->removable) os_thread_switch();
		handle->ep->sa.ip = handle->ep->sa.port = 0;
		os_mutex_destroy(&handle->ep->cn_mutex);
		os_mutex_destroy(&handle->ep->ep_mutex);
	}

	mempool_free(con_pool, conn);
}

int ZION_API package_bind(SOCK_ADDR* addr, PACKAGE_NOTIFY notify)
{
	int ret;
	unsigned int index;

	for(index=0; index<sizeof(ep_list)/sizeof(ep_list[0]); index++) {
		if(ep_list[index].sa.port==0) break;
	}
	if(index==sizeof(ep_list)/sizeof(ep_list[0])) return ERR_FULL;

	ep_list[index].sock = sock_bind(addr, SOCK_REUSEADDR | SOCK_NONBLOCK);
	if(ep_list[index].sock==SOCK_INVALID_HANDLE) return ERR_UNKNOWN;

	fdwatch_set(&ep_list[index].fdwi, ep_list[index].sock, FDWATCH_READ, iot_accept, &ep_list[index]);
	memcpy(&ep_list[index].sa, addr, sizeof(*addr));
	rlist_init(&ep_list[index].conns);
	ep_list[index].type = EPTYPE_PASSIVE;
	ep_list[index].notify = notify;
	os_mutex_init(&ep_list[index].ep_mutex);
	os_mutex_init(&ep_list[index].cn_mutex);
	rlist_clear(&ep_list[index].remove, &ep_list[index]);
	ep_list[index].removable = 0;

	ret = fdwatch_add(fdw_handle, &ep_list[index].fdwi);
	if(ret!=0) {
		sock_unbind(ep_list[index].sock);
		os_mutex_destroy(&ep_list[index].ep_mutex);
		os_mutex_destroy(&ep_list[index].cn_mutex);
		memset(&ep_list[index], 0, sizeof(ep_list[index]));
		return ret;
	}

	return ERR_NOERROR;
}

int ZION_API package_unbind(const SOCK_ADDR* addr)
{
	unsigned int index;

	assert(addr->port!=0);
	if(addr->port==0) return ERR_INVALID_PARAMETER;

	for(index=0; index<sizeof(ep_list)/sizeof(ep_list[0]); index++) {
		if(ep_list[index].sa.ip==addr->ip && ep_list[index].sa.port==addr->port) break;
	}
	if(index==sizeof(ep_list)/sizeof(ep_list[0])) return ERR_NOT_FOUND;

	rlist_push_back(&ep_remove, &ep_list[index].remove);
	while(!ep_list[index].removable) os_thread_switch();

	while (!rlist_empty(&ep_list[index].conns)) {
		package_close(rlist_get_userdata(rlist_front(&ep_list[index].conns)), PACKAGE_CLOSE_NORMAL);
	}

	ep_list[index].sa.ip = ep_list[index].sa.port = 0;

	os_mutex_destroy(&ep_list[index].cn_mutex);
	os_mutex_destroy(&ep_list[index].ep_mutex);

	return ERR_NOERROR;
}

int ZION_API package_send(PACKAGE_HANDLE handle, const void* buf, unsigned int buflen)
{
	unsigned int i;
	NBUF* nbuf;

	if (handle->sd_flag != SDFLAG_AVAILABLE) return ERR_INVALID_HANDLE;

	for (i = 0; i < buflen / NBUF_SIZE + 1; i++) {
		nbuf = (NBUF*)mempool_alloc(buf_pool);
		assert(nbuf != NULL);

		nbuf->len = buflen - i * NBUF_SIZE > NBUF_SIZE ? NBUF_SIZE : buflen - i * NBUF_SIZE;
		memcpy(nbuf->buf, (char*)buf + i * NBUF_SIZE, nbuf->len);

		os_mutex_lock(&handle->ep->cn_mutex);
		rlist_push_back(&handle->ubufs, &nbuf->item);
		os_mutex_unlock(&handle->ep->cn_mutex);
	}

	handle->wr_flag = WRFLAG_WRITABLE;

	return ERR_NOERROR;
}

void ZION_API package_disconnect(PACKAGE_HANDLE handle, int flags)
{
	NBUF *buf;
	PACKAGE_CONNECTION *conn = handle;

	if (conn == NULL) return;
	if (handle->ep->sa.port == 0) return;

	switch (flags) {
		case PACKAGE_DISCONNECT_DIRTY:
			if ((((RLIST_ITEM*)&conn->witem)->prev != NULL) && (((RLIST_ITEM*)&conn->witem)->next != NULL)) {
				fdwatch_remove(fdw_handle, &conn->witem);
			}
			while (!rlist_empty(&conn->ubufs)) {
				buf = (NBUF*)rlist_front(&conn->ubufs);
				rlist_remove(&conn->ubufs, rlist_front(&conn->ubufs));
				mempool_free(buf_pool, buf);
			}
			while (!rlist_empty(&conn->dbufs)) {
				buf = (NBUF*)rlist_front(&conn->dbufs);
				rlist_remove(&conn->dbufs, rlist_front(&conn->dbufs));
				mempool_free(buf_pool, buf);
			}
			conn->sd_flag = SDFLAG_CLOSED;
			sock_disconnect(conn->sock);
			sock_close(conn->sock);
			rlist_remove(&handle->ep->conns, &handle->eitem);
			mempool_free(con_pool, conn);
			break;
		case PACKAGE_DISCONNECT_CLEAN:
			conn->sd_flag = SDFLAG_CLOSING;
			break;
	}
}

void ZION_API package_setdata(PACKAGE_HANDLE handle, void* ptr)
{
	handle->userptr = ptr;
}

void ZION_API *package_getdata(PACKAGE_HANDLE handle)
{
	return handle->userptr;
}

unsigned int ZION_CALLBACK iot_process(void* arg)
{
	int ret;

	while(!iot_quitflag) {
		ret = fdwatch_dispatch(fdw_handle, 100);

		while(!rlist_empty(&ep_remove)) {
			ENDPOINT* ep;
			ep = rlist_get_userdata(rlist_front(&ep_remove));
			rlist_remove(&ep_remove, rlist_front(&ep_remove));
			fdwatch_remove(fdw_handle, &ep->fdwi);
			ep->removable = 1;
		}
	}

	return 0;
}

void iot_connect(FDWATCH_ITEM* item, int events)
{
	int ret;
	SOCK_HANDLE sock;
	PACKAGE_CONNECTION* conn;

	sock = fdwatch_getfd(item);

	conn = mempool_alloc(con_pool);
	if(conn==NULL) {
		sock_disconnect(sock);
		sock_close(sock);
		return;
	}

	fdwatch_set(&conn->witem, sock, FDWATCH_READ|FDWATCH_WRITE, iot_client, conn);
	conn->sock = sock;
	rlist_clear(&conn->eitem, conn);
	conn->ep = (ENDPOINT*)item;
	conn->userptr = NULL;
	conn->rd_flag = RDFLAG_EMPTY;
	conn->wr_flag = WRFLAG_FULL;
	conn->sd_flag = SDFLAG_AVAILABLE;
	rlist_init(&conn->dbufs);
	rlist_init(&conn->ubufs);
	os_mutex_lock(&conn->ep->ep_mutex);
	rlist_push_back(&conn->ep->conns, &conn->eitem);
	os_mutex_unlock(&conn->ep->ep_mutex);

	ret = threadpool_queueitem(iot_onconnect, conn);
	if(ret!=ERR_NOERROR) {
		package_disconnect(conn, PACKAGE_DISCONNECT_DIRTY);
		assert(0);
		return;
	}
}

void iot_accept(FDWATCH_ITEM* item, int events)
{
	int ret;
	SOCK_HANDLE sock;
	PACKAGE_CONNECTION* conn;

	sock = sock_accept(((ENDPOINT*)item)->sock, NULL);
	if(sock==SOCK_INVALID_HANDLE) {
		assert(0);
		return;
	}

	conn = mempool_alloc(con_pool);
	if(conn==NULL) {
		sock_disconnect(sock);
		sock_close(sock);
		assert(0);
		return;
	}

	fdwatch_set(&conn->witem, sock, FDWATCH_READ|FDWATCH_WRITE, iot_client, conn);
	conn->sock = sock;
	rlist_clear(&conn->eitem, conn);
	conn->ep = (ENDPOINT*)item;
	conn->userptr = NULL;
	conn->rd_flag = RDFLAG_EMPTY;
	conn->wr_flag = WRFLAG_FULL;
	conn->sd_flag = SDFLAG_AVAILABLE;
	rlist_init(&conn->dbufs);
	rlist_init(&conn->ubufs);
	os_mutex_lock(&conn->ep->ep_mutex);
	rlist_push_back(&conn->ep->conns, &conn->eitem);
	os_mutex_unlock(&conn->ep->ep_mutex);

	ret = threadpool_queueitem(iot_onconnect, conn);
	if(ret!=ERR_NOERROR) {
		package_disconnect(conn, PACKAGE_DISCONNECT_DIRTY);
		assert(0);
		return;
	}
}

void iot_client(FDWATCH_ITEM* item, int events)
{
	int ret;
	PACKAGE_CONNECTION* conn;

	conn = (PACKAGE_CONNECTION*)item;

	if (conn->sd_flag == SDFLAG_CLOSED) {
		return;
	}

	if(events&FDWATCH_READ) {
		NBUF* buf;
		char *rbuf;
		if (conn->sd_flag != SDFLAG_AVAILABLE) {
			return;
		}
		conn->rd_flag = RDFLAG_READING;
		conn->recvlen = conn_recvbuf_len;
		ret = sock_read(conn->sock, conn->recvbuf, conn->recvlen);
		if(ret<=0) {
			goto ON_ERROR;
		}

		conn->recvlen = ret;
		rbuf = conn->recvbuf;

		while(conn->recvlen > 0) {
			buf = (NBUF*)mempool_alloc(buf_pool);
			if(buf!=NULL) {
				buf->len = conn->recvlen > NBUF_SIZE ? NBUF_SIZE : conn->recvlen;
				memcpy(buf->buf, rbuf, buf->len);
				os_mutex_lock(&conn->ep->cn_mutex);
				rlist_push_back(&conn->dbufs, &buf->item);
				os_mutex_unlock(&conn->ep->cn_mutex);
				conn->recvlen -= buf->len;
				rbuf += buf->len;
			}
		}

		if(conn->recvlen==conn_recvbuf_len) goto ON_ERROR;
		
		conn->rd_flag = RDFLAG_EMPTY;
		threadpool_queueitem(iot_ondata, conn);
	}

	if(events&FDWATCH_WRITE) {
		NBUF* buf;
		char *sendbuf;
		int sendlen;

		if(conn->wr_flag != WRFLAG_FULL) {
			conn->wr_flag = WRFLAG_WRITING;
			do {
				os_mutex_lock(&conn->ep->cn_mutex);
				buf = (NBUF*)rlist_pop_front(&conn->ubufs);
				os_mutex_unlock(&conn->ep->cn_mutex);
				if (buf != NULL) {
					sendbuf = buf->buf;
					sendlen = buf->len;
					ret = 0;
					for (;;) {
						if (conn->sd_flag == SDFLAG_AVAILABLE) ret = sock_write(conn->sock, sendbuf+=ret, sendlen-=ret);
						else { mempool_free(buf_pool, buf); goto ON_ERROR; }
						if (sendlen == 0) break;
						if (ret <= 0) { mempool_free(buf_pool, buf); goto ON_ERROR; }
					}
					mempool_free(buf_pool, buf);
				}
			} while(buf != NULL);
			if (conn->wr_flag == WRFLAG_WRITING) conn->wr_flag = WRFLAG_FULL;
		}
	}

	if (conn->sd_flag == SDFLAG_CLOSING && conn->wr_flag == WRFLAG_FULL) {
		fdwatch_remove(fdw_handle, &conn->witem);
		conn->sd_flag = SDFLAG_CLOSED;
		sock_disconnect(conn->sock);
		sock_close(conn->sock);
		rlist_remove(&conn->ep->conns, &conn->eitem);
		mempool_free(con_pool, conn);
	}

	return;

ON_ERROR:
	fdwatch_remove(fdw_handle, &conn->witem);
	if(conn->rd_flag==RDFLAG_READING || conn->wr_flag==WRFLAG_WRITING) {
		conn->rd_flag = RDFLAG_EMPTY;
		conn->wr_flag = WRFLAG_FULL;
		threadpool_queueitem(iot_ondisconnect, conn);
	}
}

void iot_onconnect(void* arg)
{
	int ret;
	PACKAGE_CONNECTION* conn = (PACKAGE_CONNECTION*)arg;

	conn->ep->notify(conn, PACKAGE_ONCONNECT, NULL, 0);

	ret = fdwatch_add(fdw_handle, &conn->witem);
	if(ret!=ERR_NOERROR) {
		package_disconnect(conn, PACKAGE_DISCONNECT_DIRTY);
		return;
	}
}

void iot_ondata(void* arg)
{
	PACKAGE_CONNECTION* conn = (PACKAGE_CONNECTION*)arg;
	NBUF* buf;

	for(;;) {
		os_mutex_lock(&conn->ep->cn_mutex);
		buf = (NBUF*)rlist_pop_front(&conn->dbufs);
		os_mutex_unlock(&conn->ep->cn_mutex);

		if(buf==NULL) {
			break;
		}

		conn->ep->notify(conn, PACKAGE_ONDATA, buf->buf, buf->len);
		mempool_free(buf_pool, buf);
	}
}

void iot_ondisconnect(void* arg)
{
	PACKAGE_CONNECTION* conn = (PACKAGE_CONNECTION*)arg;
	conn->ep->notify(conn, PACKAGE_ONDISCONNECT, NULL, 0);
	package_disconnect(conn, PACKAGE_DISCONNECT_DIRTY);
}
