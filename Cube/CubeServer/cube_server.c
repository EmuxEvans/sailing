#include <string.h>
#include <assert.h>

#include <skates\skates.h>

#include "cube_pdl.CubeServer.h"
#include "cube_server.h"

SOCK_ADDR cube_sa;
char cube_dbstr[100] = "provider=sqlite;dbname=..\\cube.db";
CUBE_CONNECTION* conn_list[1000];
CUBE_ROOM* room_list[1000];

static MEMPOOL_HANDLE conn_pool;
static MEMPOOL_HANDLE room_pool;

static void onconnect(NETWORK_HANDLE handle, void* userptr)
{
	CUBE_CONNECTION* conn;
	conn = (CUBE_CONNECTION*)userptr;
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

	if(conn->room!=NULL) {
		cube_room_leave(conn->room, conn);
		cube_room_check(conn->room);
		conn->room = NULL;
	}

	network_del(handle);
	mempool_free(conn_pool, conn);
}

static void onaccept(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname)
{
	int idx;
	CUBE_CONNECTION* conn;
	NETWORK_HANDLE handle;
	NETWORK_EVENT event;

	for(idx=0; idx<sizeof(conn_list)/sizeof(conn_list[0]); idx++) {
		if(conn_list[idx]==NULL) break;
	}
	if(idx==sizeof(conn_list)/sizeof(conn_list[0])) {
		sock_close(sock);
		return;
	}

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
	memset(conn, 0, sizeof(*conn));
	conn->index = idx;

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
	memset(conn_list, 0, sizeof(conn_list));
	conn_pool = mempool_create("CUBE_CONN_POOL", sizeof(CUBE_CONNECTION), 0);
	return network_tcp_register(&cube_sa, onaccept, NULL);
}

static int cube_final()
{
	int ret;

	ret = network_tcp_unregister(&cube_sa);
	if(ret!=ERR_NOERROR) return ret;

	while(1) {
		os_sleep(10);

		for(ret=0; ret<sizeof(conn_list)/sizeof(conn_list[0]); ret++) {
			if(conn_list[ret]!=NULL) break;
		}
		if(ret==sizeof(conn_list)/sizeof(conn_list[0])) {
			break;
		}
	}

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

CUBE_ROOM* cube_room_create(CUBE_CONNECTION* conn, const char* name, const char* map)
{
	CUBE_ROOM* room;
	room = (CUBE_ROOM*)mempool_alloc(room_pool);
	if(room==NULL) return NULL;

	memset(room, 0, sizeof(*room));
	strcpy(room->name, name);
	strcpy(room->map, map);
	room->singer = 0;
	room->conns[0] = conn;
	conn->room = room;
	conn->room_idx = 0;

	return room;
}

void cube_room_leave(CUBE_ROOM* room, CUBE_CONNECTION* conn)
{
	int idx;
	SVR_USER_CTX ctx;

	for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
		if(room->conns[idx]==conn) break;
	}
	assert(idx<sizeof(room->conns)/sizeof(room->conns[0]));

	for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
		if(room->conns[idx]==NULL) continue;
		ctx.conn = room->conns[idx];
		room_notify_leave(&ctx, conn->nick);
	}

	room->conns[idx] = NULL;
	room->readys[idx] = 0;
	conn->room = NULL;
}

void cube_room_check(CUBE_ROOM* room)
{
	int idx, count, ready, loaded;

	count = ready = loaded = 0;
	for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
		if(room->conns[idx]==NULL) continue;
		if(room->readys[idx]) ready++;
		if(room->loaded[idx]) loaded++;
		count++;
	}
	if(count==0) {
		mempool_free(room_pool, room);
		return;
	}
	if(room->singer>=0 && room->conns[room->singer]==NULL) {
		room->singer = -1;
	}
	if(room->singer==-1 && room->state!=CUBE_ROOM_STATE_ACTIVE) {
		for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
			SVR_USER_CTX ctx;
			if(room->conns[idx]==NULL) continue;
			ctx.conn = room->conns[idx];
			room_notify_terminate(&ctx);
		}
		memset(room->readys, 0, sizeof(room->readys));
		return;
	}
	if(count==ready && room->singer>=0 && room->state==CUBE_ROOM_STATE_ACTIVE) {
		room->state = CUBE_ROOM_STATE_LOADING;
		for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
			SVR_USER_CTX ctx;
			if(room->conns[idx]==NULL) continue;
			ctx.conn = room->conns[idx];
			room_notify_load(&ctx);
		}
		return;
	}
	if(count==loaded && room->state==CUBE_ROOM_STATE_LOADING) {
		room->state = CUBE_ROOM_STATE_GAMING;
		for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
			SVR_USER_CTX ctx;
			if(room->conns[idx]==NULL) continue;
			ctx.conn = room->conns[idx];
			room_notify_start(&ctx);
		}
		return;
	}
}

void cube_room_onjoin(CUBE_ROOM* room, CUBE_CONNECTION* conn)
{
	int idx;
	SVR_USER_CTX ctx;

	for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
		if(room->conns[idx]==NULL) continue;

		ctx.conn = room->conns[idx];
		room_notify_join(&ctx, conn->nick, conn->equ);

		if(room->conns[idx]!=conn) continue;

		ctx.conn = conn;
		room_notify_join(&ctx, room->conns[idx]->nick, room->conns[idx]->equ);
	}

	ctx.conn = conn;
	if(room->singer>=0) {
		room_info_callback(&ctx, room->name, room->conns[room->singer]->nick, room->map);
	} else {
		room_info_callback(&ctx, room->name, "", room->map);
	}
}

void cube_room_terminate(CUBE_ROOM* room)
{
	int i, j;
	SVR_USER_CTX ctx;

	for(i=0; i<sizeof(room->conns)/sizeof(room->conns[0]); i++) {
		if(room->conns[i]==NULL) continue;
		ctx.conn = room->conns[i];

		room_notify_terminate(&ctx);
		for(j=0; j<sizeof(room->conns)/sizeof(room->conns[0]); j++) {
			if(room->conns[j]==NULL) continue;
			room_notify_join(&ctx, room->conns[j]->nick, room->conns[j]->equ);
		}
		if(room->singer>=0) {
			room_info_callback(&ctx, room->name, room->conns[room->singer]->nick, room->map);
		} else {
			room_info_callback(&ctx, room->name, "", room->map);
		}
	}
}

int cube_can_change_equipment(CUBE_CONNECTION* conn)
{
	if(conn->room==NULL) return 1;
	if(conn->room->state!=CUBE_ROOM_STATE_ACTIVE) return 0;
	if(conn->room->readys[conn->room_idx]) return 0;
	return 1;
}
