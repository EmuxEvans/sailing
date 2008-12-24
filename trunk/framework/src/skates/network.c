#include <assert.h>
#include <string.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/sock.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/mempool.h"
#include "../../inc/skates/network.h"

#include "../../inc/skates/threadpool.h"
#include "../../inc/skates/fdwatch.h"

typedef struct TCP_ENDPOINT {
	FDWATCH_ITEM			fdwitem;
	RLIST_ITEM				shutdown_item;
	int						ref_count;

	SOCK_HANDLE				sock;
	SOCK_ADDR				sa;
	NETWORK_ONACCEPT		OnAccept;
	void*					userptr;
} TCP_ENDPOINT;

typedef struct NETWORK_CONNECTION {
	FDWATCH_ITEM			fdwitem;
	int						ref_count;
	int						rdcount, wrcount;
	int						is_connect, alive;
	RLIST_ITEM				shutdown_item;

	SOCK_HANDLE				sock;
	SOCK_ADDR				peername;
	void*					userptr;
	NETWORK_ONCONNECT		OnConnect;
	NETWORK_ONDATA			OnData;
	NETWORK_ONDISCONNECT	OnDisconnect;

	MEMPOOL_HANDLE			recvbuf_pool;
	char*					recvbuf_buf;
	unsigned int			recvbuf_cur;
	unsigned int			recvbuf_len;
	unsigned int			recvbuf_max;

	RLIST_HEAD				downbufs;
	unsigned int			downbuf_cur;
} NETWORK_CONNECTION;

static unsigned int downbuf_size;
static MEMPOOL_HANDLE downbuf_pool;
static os_mutex_t downbuf_mutex;
static MEMPOOL_HANDLE conn_pool;

static TCP_ENDPOINT tcp_eps[100];

static RLIST_HEAD shutdown_eps;
static RLIST_HEAD shutdown_con;
static os_mutex_t shutdown_mutex;

static FDWATCH_HANDLE network_fdw;
static void accept_fdevent(FDWATCH_ITEM* item, int events);
static void tcpcon_fdevent(FDWATCH_ITEM* item, int events);

static void accept_event(void* data);
static void read_event(void* data);
static void write_event(void* data);
static void close_event(void* data);

static void opt_accept(TCP_ENDPOINT* ep);
static int opt_read(NETWORK_CONNECTION* conn);
static void opt_write(NETWORK_CONNECTION* conn);
static void opt_close(NETWORK_CONNECTION* conn);

static unsigned int ZION_CALLBACK notify_proc(void* arg);
static os_thread_t notify_tid;
static volatile int notify_quit;

int network_init(unsigned int _downbuf_size)
{
	int ret;

	downbuf_size = _downbuf_size;

	downbuf_pool = mempool_create("NETWORK.DOWNBUF", sizeof(NETWORK_DOWNBUF)+downbuf_size-1, 0);
	conn_pool = mempool_create("NETWORK.CONN", sizeof(NETWORK_CONNECTION), 0);
	assert(downbuf_pool && conn_pool);

	os_mutex_init(&downbuf_mutex);
	os_mutex_init(&shutdown_mutex);

	memset(tcp_eps, 0, sizeof(tcp_eps));
	rlist_init(&shutdown_eps);
	rlist_init(&shutdown_con);

	network_fdw = fdwatch_create();
	if(network_fdw) {
		notify_quit = 1;
		ret = os_thread_begin(&notify_tid, notify_proc, NULL);
		if(ret==0) {
			while(notify_quit) os_sleep(10);
			return ERR_NOERROR;
		}
		fdwatch_destroy(network_fdw);
	} else {
		ret = ERR_UNKNOWN;
	}

	mempool_destroy(downbuf_pool);
	mempool_destroy(conn_pool);
	os_mutex_destroy(&downbuf_mutex);
	os_mutex_destroy(&shutdown_mutex);
	return ERR_UNKNOWN;
}

int network_final()
{
	notify_quit = 1;
	os_thread_wait(notify_tid, NULL);

	fdwatch_destroy(network_fdw);
	mempool_destroy(downbuf_pool);
	mempool_destroy(conn_pool);
	os_mutex_destroy(&downbuf_mutex);
	os_mutex_destroy(&shutdown_mutex);

	return ERR_NOERROR;
}

int network_tcp_register(SOCK_ADDR* sa, NETWORK_ONACCEPT OnAccept, void* userptr)
{
	int index, ret;

	for(index=0; index<sizeof(tcp_eps)/sizeof(tcp_eps[0]); index++) {
		if(tcp_eps[index].ref_count==0) break;
	}
	if(index==sizeof(tcp_eps)/sizeof(tcp_eps[0])) return ERR_FULL;

	tcp_eps[index].sock = sock_bind(sa, SOCK_REUSEADDR|SOCK_NONBLOCK);
	if(tcp_eps[index].sock==SOCK_INVALID_HANDLE) {
		return ERR_UNKNOWN;
	}
	if(sock_nonblock(tcp_eps[index].sock)!=0) {
		sock_unbind(tcp_eps[index].sock);
		tcp_eps[index].sock = SOCK_INVALID_HANDLE;
		return ERR_UNKNOWN;
	}

	rlist_clear(&tcp_eps[index].shutdown_item, &tcp_eps[index]);
	tcp_eps[index].ref_count = 1;

	memcpy(&tcp_eps[index].sa, sa, sizeof(*sa));
	tcp_eps[index].OnAccept = OnAccept;
	tcp_eps[index].userptr = userptr;

	ret = fdwatch_set(&tcp_eps[index].fdwitem, tcp_eps[index].sock, FDWATCH_READ, accept_fdevent, &tcp_eps[index]);
	if(ret!=ERR_NOERROR) {
		sock_unbind(tcp_eps[index].sock);
		tcp_eps[index].sock = SOCK_INVALID_HANDLE;
		return ret;
	}

	ret = fdwatch_add(network_fdw, &tcp_eps[index].fdwitem);
	if(ret!=ERR_NOERROR) {
		sock_unbind(tcp_eps[index].sock);
		memset(&tcp_eps[index], 0, sizeof(tcp_eps[index]));
		return ret;
	}

	return ERR_NOERROR;
}

int network_tcp_unregister(const SOCK_ADDR* sa)
{
	int index;

	for(index=0; index<sizeof(tcp_eps)/sizeof(tcp_eps[0]); index++) {
		if(tcp_eps[index].ref_count==0) continue;
		if(memcmp(&tcp_eps[index].sa, sa, sizeof(*sa))==0) break;
	}
	if(index==sizeof(tcp_eps)/sizeof(tcp_eps[0])) return ERR_NOT_FOUND;

	assert(tcp_eps[index].shutdown_item.next==NULL);
	assert(tcp_eps[index].shutdown_item.prev==NULL);
	assert(tcp_eps[index].shutdown_item.ptr==&tcp_eps[index]);

	rlist_push_back(&shutdown_eps, &tcp_eps[index].shutdown_item);
	while(tcp_eps[index].ref_count!=0) os_sleep(10);

	sock_unbind(tcp_eps[index].sock);
	memset(&tcp_eps[index], 0, sizeof(tcp_eps[index]));

	return ERR_NOERROR;
}

NETWORK_HANDLE network_add(SOCK_HANDLE sock, NETWORK_EVENT* event, void* userptr)
{
	NETWORK_CONNECTION* conn;

	if(event->recvbuf_pool==NULL && event->recvbuf_buf==NULL) {
		return NULL;
	}

	conn = mempool_alloc(conn_pool);
	if(conn==NULL) return NULL;

	fdwatch_set(&conn->fdwitem, sock, FDWATCH_READ|FDWATCH_WRITE, tcpcon_fdevent, conn);
	conn->ref_count = 1;
	conn->rdcount = 0;
	conn->wrcount = 0;
	conn->is_connect = 0;
	conn->alive = 1;
	rlist_clear(&conn->shutdown_item, conn);
	conn->sock			= sock;
	sock_peername(sock, &conn->peername);
	conn->userptr		= userptr;
	conn->OnConnect		= event->OnConnect;
	conn->OnData		= event->OnData;
	conn->OnDisconnect	= event->OnDisconnect;

	if(event->recvbuf_buf) {
		conn->recvbuf_pool = NULL;
		conn->recvbuf_buf = event->recvbuf_buf;
	} else {
		conn->recvbuf_pool = event->recvbuf_pool;
		conn->recvbuf_buf = (char*)mempool_alloc(event->recvbuf_pool);
		if(conn->recvbuf_buf==NULL) {
			mempool_free(conn_pool, conn);
			return NULL;
		}
	}
	conn->recvbuf_cur = 0;
	conn->recvbuf_len = 0;
	conn->recvbuf_max = event->recvbuf_max;

	rlist_init(&conn->downbufs);
	conn->downbuf_cur = 0;

	if(fdwatch_add(network_fdw, &conn->fdwitem)!=ERR_NOERROR) {
		if(conn->recvbuf_pool) mempool_free(conn->recvbuf_pool, conn->recvbuf_buf);
		mempool_free(conn_pool, conn);
		return NULL;
	}

	return conn;
}

int network_del(NETWORK_HANDLE handle)
{
	while(!rlist_empty(&handle->downbufs)) {
		mempool_free(downbuf_pool, rlist_pop_front(&handle->downbufs));
	}

	if(handle->recvbuf_pool) mempool_free(handle->recvbuf_pool, handle->recvbuf_buf);
	mempool_free(conn_pool, handle);
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

unsigned int network_downbufs_alloc(NETWORK_DOWNBUF* downbufs[], unsigned int count, unsigned int size)
{
	unsigned int n, l;
	n = (size-1) / downbuf_size + 1;
	if(n>count) return 0;

	for(l=0; l<n; l++) {
		downbufs[l] = network_downbuf_alloc();
		if(downbufs[l]==NULL) break;
		downbufs[l]->len = downbuf_size;
	}
	if(l<n) {
		for(n=0; n<l; n++) {
			network_downbuf_free(downbufs[n]);
		}
		return 0;
	}
	downbufs[n-1]->len = (size-1) % downbuf_size + 1;
	return n;
}

int network_downbufs_fill(NETWORK_DOWNBUF* downbufs[], unsigned int count, unsigned int start, const void* data, unsigned int data_len)
{
	unsigned int bs, be;

	if(start+data_len>=downbuf_size*count) return ERR_INVALID_PARAMETER;

	bs = start / downbuf_size;
	be = (start+data_len-1) / downbuf_size;

	if(bs==be) {
		memcpy(downbufs[bs]->buf+start%downbuf_size, data, data_len);
	} else {
		unsigned int bs_s, bs_l, be_l, l;
		bs_s = start % downbuf_size;
		bs_l = downbuf_size - bs_s;
		be_l = (start+downbuf_size-1)%downbuf_size + 1;
		memcpy(downbufs[bs]->buf+bs_s, data, bs_l);
		for(l=bs+1; l<be; l++) {
		memcpy(downbufs[l]->buf, (char*)data + bs_l + downbuf_size*(l-bs), downbuf_size);
		}
		memcpy(downbufs[be]->buf, (char*)data + bs_l + downbuf_size*(be-bs-1), be_l);
	}

	return ERR_NOERROR;
}

void network_downbufs_free(NETWORK_DOWNBUF* downbufs[], unsigned int count)
{
	unsigned int l;
	for(l=0; l<count; l++) {
		network_downbuf_free(downbufs[l]);
	}
}

unsigned int network_recvbuf_len(NETWORK_HANDLE handle)
{
	return handle->recvbuf_len;
}

const void* network_recvbuf_ptr(NETWORK_HANDLE handle, unsigned int start, unsigned int len)
{
	unsigned int t;
	if(start+len<=handle->recvbuf_len) {
		start = (handle->recvbuf_cur + start) % handle->recvbuf_max;
		t = handle->recvbuf_max - start;
		if(t>len) {
			return &handle->recvbuf_buf[start];
		}
	}
	return NULL;
}

int network_recvbuf_get(NETWORK_HANDLE handle, void* buf, unsigned int start, unsigned int len)
{
	unsigned int t;

	if(start+len>handle->recvbuf_len)
		return ERR_UNKNOWN;

	start = (handle->recvbuf_cur + start) % handle->recvbuf_max;
	t = handle->recvbuf_max - start;
	if(t>len) {
		memcpy(buf, &handle->recvbuf_buf[start], len);
	} else {
		memcpy(buf, &handle->recvbuf_buf[start], t);
		memcpy((char*)buf+t, &handle->recvbuf_buf[0], len-t);
	}

	return ERR_NOERROR;
}

int network_recvbuf_commit(NETWORK_HANDLE handle, unsigned int len)
{
	if(len>handle->recvbuf_len)
		return ERR_INVALID_PARAMETER;

	if(handle->recvbuf_len==len) {
		handle->recvbuf_cur = 0;
		handle->recvbuf_len = 0;
	} else {
		handle->recvbuf_cur += len;
		handle->recvbuf_len -= len;
	}

	return ERR_NOERROR;
}

int network_send(NETWORK_HANDLE handle, NETWORK_DOWNBUF* downbufs[], unsigned int count)
{
	unsigned int l;

	os_mutex_lock(&downbuf_mutex);
	for(l=0; l<count; l++) {
		rlist_clear(&downbufs[l]->item, downbufs[l]);
		rlist_push_back(&handle->downbufs, &downbufs[l]->item);
	}
	os_mutex_unlock(&downbuf_mutex);

	if(atom_inc(&handle->wrcount)==1) {
		do {
			if(handle->alive) {
				opt_write(handle);
			}
		} while(atom_dec(&handle->wrcount)>0);
	}

	return ERR_NOERROR;
}

int network_disconnect(NETWORK_HANDLE handle)
{
	sock_disconnect(handle->sock);
	return ERR_NOERROR;
}

void network_set_userptr(NETWORK_HANDLE handle, void* userptr)
{
	handle->userptr = userptr;
}

void* network_get_userptr(NETWORK_HANDLE handle)
{
	return handle->userptr;
}

SOCK_HANDLE network_get_sock(NETWORK_HANDLE handle)
{
	return handle->sock;
}

const SOCK_ADDR* network_get_peername(NETWORK_HANDLE handle)
{
	return &handle->peername;
}

void accept_fdevent(FDWATCH_ITEM* item, int events)
{
	TCP_ENDPOINT* ep;
	ep = (TCP_ENDPOINT*)item;
	atom_inc(&ep->ref_count);
	threadpool_queueitem(accept_event, ep);
}

void tcpcon_fdevent(FDWATCH_ITEM* item, int events)
{
	NETWORK_CONNECTION* conn;
	conn = (NETWORK_CONNECTION*)item;
	if(events&FDWATCH_WRITE) {
		atom_inc(&conn->ref_count);
		threadpool_queueitem(write_event, conn);
	}
	if((events&FDWATCH_READ) || !conn->is_connect) {
		conn->is_connect = 1;
		atom_inc(&conn->ref_count);
		threadpool_queueitem(read_event, conn);
	}
}

void accept_event(void* data)
{
	TCP_ENDPOINT* ep;
	ep = (TCP_ENDPOINT*)data;
	opt_accept(ep);
	atom_dec(&ep->ref_count);
}

void read_event(void* data)
{
	NETWORK_CONNECTION* conn = (NETWORK_CONNECTION*)data;

	if(atom_inc(&conn->rdcount)==1) {
		if(conn->OnConnect) {
			conn->OnConnect(conn, conn->userptr);
			conn->OnConnect = NULL;
		}
		do {
			if(conn->alive) {
				if(!opt_read(conn)) {
					conn->alive = 0;
					assert(conn->shutdown_item.next==NULL);
					assert(conn->shutdown_item.prev==NULL);
					assert(conn->shutdown_item.ptr==conn);
					os_mutex_lock(&shutdown_mutex);
					rlist_push_back(&shutdown_con, &conn->shutdown_item);
					os_mutex_unlock(&shutdown_mutex);
				}
			}
		} while(atom_dec(&conn->rdcount)>0);
	}

	if(atom_dec(&conn->ref_count)==0)
		opt_close(conn);
}

void write_event(void* data)
{
	NETWORK_CONNECTION* conn = (NETWORK_CONNECTION*)data;

	if(atom_inc(&conn->wrcount)==1) {
		do {
			if(conn->alive) {
				opt_write(conn);
			}
		} while(atom_dec(&conn->wrcount)>0);
	}

	if(atom_dec(&conn->ref_count)==0)
		opt_close(conn);
}

void close_event(void* data)
{
	NETWORK_CONNECTION* conn = (NETWORK_CONNECTION*)data;
	if(atom_dec(&conn->ref_count)==0) opt_close(conn);
}

unsigned int ZION_CALLBACK notify_proc(void* arg)
{
	TCP_ENDPOINT* sd_eps[10];
	NETWORK_CONNECTION* sd_con[100];
	int sd_eps_count, sd_con_count;

	notify_quit = 0;
	while(!notify_quit) {

		sd_eps_count = sd_con_count = 0;
		os_mutex_lock(&shutdown_mutex);
		for(;;) {
			if(sd_eps_count==sizeof(sd_eps)/sizeof(sd_eps[0])) break;
			if(rlist_empty(&shutdown_eps)) break;
			sd_eps[sd_eps_count++] = (TCP_ENDPOINT*)rlist_get_userdata(rlist_front(&shutdown_eps));
			rlist_pop_front(&shutdown_eps);
		}
		for(;;) {
			if(sd_con_count==sizeof(sd_con)/sizeof(sd_con[0])) break;
			if(rlist_empty(&shutdown_con)) break;
			sd_con[sd_con_count++] = (NETWORK_CONNECTION*)rlist_get_userdata(rlist_front(&shutdown_con));
			rlist_pop_front(&shutdown_con);
		}
		os_mutex_unlock(&shutdown_mutex);
		for(; sd_eps_count>0; sd_eps_count--) {
			fdwatch_remove(network_fdw, &sd_eps[sd_eps_count-1]->fdwitem);
			atom_dec(&sd_eps[sd_eps_count-1]->ref_count);
		}
		for(; sd_con_count>0; sd_con_count--) {
			fdwatch_remove(network_fdw, &sd_con[sd_con_count-1]->fdwitem);
			threadpool_queueitem(close_event, sd_con[sd_con_count-1]);
		}

		fdwatch_dispatch(network_fdw, 10);
	}
	return 0;
}

void opt_accept(TCP_ENDPOINT* ep)
{
	SOCK_ADDR pname;
	SOCK_HANDLE clt;
	for(;;) {
		clt = sock_accept(ep->sock, &pname);
		if(clt==SOCK_INVALID_HANDLE) break;

		ep->OnAccept(ep->userptr, clt, &pname);
	}
}

int opt_read(NETWORK_CONNECTION* conn)
{
	int ret;
	while(1) {
		unsigned int start, len;

		start = (conn->recvbuf_cur+conn->recvbuf_len)%conn->recvbuf_max;
		if(start<conn->recvbuf_cur) {
			len = conn->recvbuf_cur - start;
		} else {
			len = conn->recvbuf_max - start;
		}

		ret = sock_read(conn->sock, &conn->recvbuf_buf[start], len);
		if(ret<=0) break;
		conn->recvbuf_len += ret;

		conn->OnData(conn, conn->userptr);

		if(conn->recvbuf_len==conn->recvbuf_max) {
			sock_disconnect(conn->sock);
			return 0;
		}
	}

	return ret==0?1:0;
}

void opt_write(NETWORK_CONNECTION* conn)
{
	NETWORK_DOWNBUF* downbuf;
	downbuf = (NETWORK_DOWNBUF*)rlist_front(&conn->downbufs);
	while(!rlist_is_head(&conn->downbufs, &downbuf->item)) {
		int ret;
		ret = sock_write(conn->sock, &downbuf->buf[conn->downbuf_cur], downbuf->len-conn->downbuf_cur);
		if(ret<=0) break;

		conn->downbuf_cur += ret;
		if(conn->downbuf_cur<downbuf->len) continue;

		os_mutex_lock(&downbuf_mutex);
		rlist_pop_front(&conn->downbufs);
		conn->downbuf_cur = 0;
		os_mutex_unlock(&downbuf_mutex);
		network_downbuf_free(downbuf);

		downbuf = (NETWORK_DOWNBUF*)rlist_front(&conn->downbufs);
	}
}

void opt_close(NETWORK_CONNECTION* conn)
{
	conn->OnDisconnect(conn, conn->userptr);
}
