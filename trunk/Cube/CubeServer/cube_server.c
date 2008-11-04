
#include <skates\skates.h>

#include "cube_pdl.CubeServer.h"

typedef struct CUBE_CONNECTION {
	char uid[CUBE_UID_LEN+1];
	char nick[CUBE_UID_LEN+1];

	char recv_buf[10*1024];
} CUBE_CONNECTION;

static SOCK_ADDR cube_sa;
static MEMPOOL_HANDLE conn_pool;

static void onconnect(NETWORK_HANDLE handle, void* userptr)
{
	CUBE_CONNECTION* conn;
	conn = (CUBE_CONNECTION*)userptr;
	memset(conn, 0, sizeof(*conn));
}

static void ondata(NETWORK_HANDLE handle, void* userptr)
{
	CUBE_CONNECTION* conn;
	conn = (CUBE_CONNECTION*)userptr;

	while(1) {
		int ret;
		unsigned short len;
		char pkg_buf[sizeof(conn->recv_buf)];
		MEM_STREAM stream;

		if(network_recvbuf_len(handle)<2) break;
		network_recvbuf_get(handle, &len, 0, sizeof(len));
		if(network_recvbuf_len(handle)<sizeof(len)+len) break;
		network_recvbuf_get(handle, pkg_buf, sizeof(len), len);

		if(conn->nick[0]=='\0' && *((unsigned short*)pkg_buf)!=LOGIN_FILTER_ID) {
			network_disconnect(handle);
			break;
		}

		memstream_init(&stream, pkg_buf, len, len);
		ret = SVR_Dispatcher(NULL, (STREAM*)&stream);
		if(ret!=ERR_NOERROR) {
			network_disconnect(handle);
			break;
		}

		network_recvbuf_commit(handle, sizeof(len)+len);
	}
}

static void ondisconnect(NETWORK_HANDLE handle, void* userptr)
{
	CUBE_CONNECTION* conn;
	conn = (CUBE_CONNECTION*)userptr;
	network_del(handle);
	mempool_free(conn_pool, conn);
}

static void onaccept(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname)
{
	CUBE_CONNECTION* conn;
	NETWORK_HANDLE handle;
	NETWORK_EVENT event;

	conn = (CUBE_CONNECTION*)mempool_alloc(conn_pool);
	if(conn==NULL) {
		sock_close(sock);
		return;
	}

	event.OnConnect = onconnect;
	event.OnData = ondata;
	event.OnDisconnect = ondisconnect;
	event.recvbuf_buf = conn->recv_buf;
	event.recvbuf_max = sizeof(conn->recv_buf);
	event.recvbuf_pool = NULL;

	handle = network_add(sock, &event, conn);
	if(handle==NULL) {
		mempool_free(conn_pool, conn);
		sock_close(sock);
		return;
	}
}

static int cube_loadconfig()
{
	sock_str2addr("0.0.0.0:2008", &cube_sa);
	return ERR_NOERROR;
}

static int cube_init()
{
	conn_pool = mempool_create("CUBE_CONN_POOL", sizeof(CUBE_CONNECTION), 0);
	return network_tcp_register(&cube_sa, onaccept, NULL);
}

static int cube_final()
{
	int ret;

	ret = network_tcp_unregister(&cube_sa);
	if(ret!=ERR_NOERROR) return ret;

	mempool_destroy(conn_pool);
	return ERR_NOERROR;
}

ZION_EXPORT int module_entry(int reason)
{
	int ret;

	switch(reason) {
	case ENTRY_REASON_ATTACH:
		ret = cube_loadconfig();
		if(ret!=ERR_NOERROR) return ret;
		ret = cube_init();
		if(ret!=ERR_NOERROR) return ret;
		break;
	case ENTRY_REASON_DETACH:
		ret = cube_final();
		if(ret!=ERR_NOERROR) return ret;
		break;
	}

	return ERR_NOERROR;
}
