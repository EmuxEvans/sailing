#include <string.h>
#include <assert.h>

#include <skates/skates.h>

#include "cube_pdl.CubeServer.h"
#include "cube_server.h"

SOCK_ADDR cube_sa;
char cube_dbstr[100] = "provider=sqlite;dbname=..\\cube.db";
CUBE_CONNECTION* conn_list[1000];
CUBE_ROOM* room_list[1000];
static TIMER room_timer;
static unsigned int cur_time;
static MEMPOOL_HANDLE conn_pool;
static MEMPOOL_HANDLE room_pool;
static os_mutex_t room_mtx;

static void room_timer_proc(TIMER handle, void* key)
{
	int idx;
	os_mutex_lock(&room_mtx);
	cur_time ++;
	for(idx=0; idx<sizeof(room_list)/sizeof(room_list[0]); idx++) {
		if(room_list[idx]==NULL) continue;
		cube_room_tick(room_list[idx]);
	}
	os_mutex_unlock(&room_mtx);
}

static void onconnect(NETWORK_HANDLE handle, void* userptr)
{
	CUBE_CONNECTION* conn;

	conn = (CUBE_CONNECTION*)userptr;
	assert(handle);
}

static void ondata(NETWORK_HANDLE handle, void* userptr)
{
	CUBE_CONNECTION* conn;
	SVR_USER_CTX ctx;
	conn = (CUBE_CONNECTION*)userptr;
	ctx.conn = conn;

	os_mutex_lock(&room_mtx);

	while(1) {
		int ret;
		unsigned short len;
		char pkg_buf[sizeof(conn->recv_buf)];
		MEM_STREAM stream;

		if(network_recvbuf_len(handle)<2) break;
		network_recvbuf_get(handle, &len, 0, sizeof(len));
		if(network_recvbuf_len(handle)<len) break;
		network_recvbuf_get(handle, pkg_buf, sizeof(len), len-sizeof(len));

		if(conn->nick[0]=='\0' && *((unsigned short*)pkg_buf)!=LOGIN_FILTER_ID) {
			network_disconnect(handle);
			break;
		}

		memstream_init(&stream, pkg_buf, len, len);
		ret = SVR_Dispatcher(&ctx, (STREAM*)&stream);
		if(ret!=ERR_NOERROR) {
			network_disconnect(handle);
			break;
		}

		network_recvbuf_commit(handle, len);
	}

	os_mutex_unlock(&room_mtx);
}

static void ondisconnect(NETWORK_HANDLE handle, void* userptr)
{
	CUBE_CONNECTION* conn;
	CUBE_ROOM* room;
	conn = (CUBE_CONNECTION*)userptr;

	os_mutex_lock(&room_mtx);
	if(conn->room!=NULL) {
		room = conn->room;
		cube_room_leave(room, conn);
	}
	os_mutex_unlock(&room_mtx);

	network_del(handle);
	mempool_free(conn_pool, conn);
}

static void onaccept(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname)
{
	int idx, ret;
	CUBE_CONNECTION* conn;
	NETWORK_EVENT event;

	os_mutex_lock(&room_mtx);

	for(idx=0; idx<sizeof(conn_list)/sizeof(conn_list[0]); idx++) {
		if(conn_list[idx]==NULL) break;
	}
	if(idx==sizeof(conn_list)/sizeof(conn_list[0])) {
		sock_close(sock);
		os_mutex_unlock(&room_mtx);
		return;
	}

	conn = (CUBE_CONNECTION*)mempool_alloc(conn_pool);
	if(conn==NULL) {
		sock_close(sock);
		os_mutex_unlock(&room_mtx);
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
	conn_list[idx] = conn;

	ret = network_add(sock, &event, conn, &conn->handle);
	if(ret!=ERR_NOERROR) {
		mempool_free(conn_pool, conn);
		sock_close(sock);
		os_mutex_unlock(&room_mtx);
		return;
	}

	os_mutex_unlock(&room_mtx);
}

static int cube_loadconfig()
{
	APPBOX_SETTING_BEGIN(_setting)
		APPBOX_SETTING_ENDPOINT("tcp_ep", cube_sa)
		APPBOX_SETTING_STRING("dbstr", cube_dbstr, sizeof(cube_dbstr))
	APPBOX_SETTING_END(_setting)
	return appbox_config_get(MODULE_NAME, _setting);
}

static int cube_init()
{
	os_mutex_init(&room_mtx);

	memset(conn_list, 0, sizeof(conn_list));
	conn_pool = mempool_create("CUBE_CONN_POOL", sizeof(CUBE_CONNECTION), 0);
	room_pool = mempool_create("CUBE_ROOM_POOL", sizeof(CUBE_ROOM), 0);

	cur_time = 0;
	room_timer = timer_add(CUBE_ROOM_TIMER, CUBE_ROOM_TIMER, room_timer_proc, NULL);

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

	timer_force_remove(room_timer);
	mempool_destroy(room_pool);
	mempool_destroy(conn_pool);
	os_mutex_destroy(&room_mtx);
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

CUBE_ROOM* cube_room_create(CUBE_CONNECTION* conn, const char* name, const char* owner)
{
	int idx;
	CUBE_ROOM* room;

	for(idx=0; idx<sizeof(room_list)/sizeof(room_list[0]); idx++) {
		if(room_list[idx]==NULL) break;
	}
	if(idx==sizeof(room_list)/sizeof(room_list[0])) return NULL;

	room = (CUBE_ROOM*)mempool_alloc(room_pool);
	if(room==NULL) return NULL;
	room_list[idx] = room;

	memset(room, 0, sizeof(*room));
	strcpy(room->name, name);
	strcpy(room->owner, owner);

	return room;
}

void cube_room_leave(CUBE_ROOM* room, CUBE_CONNECTION* conn)
{
	int idx;
	SVR_USER_CTX ctx;

	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==conn) break;
	}
	assert(idx<sizeof(room->members)/sizeof(room->members[0]));
	assert(idx==conn->room_idx);

	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) continue;
		ctx.conn = room->members[idx].conn;
		room_notify_leave(&ctx, conn->nick);
	}

	conn->room = NULL;
	room->members[conn->room_idx].conn = NULL;
	room->members[conn->room_idx].ready = 0;
	room->members[conn->room_idx].loaded = 0;

	cube_room_giveup(room, conn->nick);
	cube_room_check(room);
}

void cube_room_check(CUBE_ROOM* room)
{
	int idx, count, ready, loaded, singer, p2p, terminated;
	unsigned int p2pmask;

	singer = -1;
	count = ready = loaded = p2p = terminated = 0;
	p2pmask = cube_room_p2pmask(room, -1);
	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) continue;
		if(room->members[idx].ready) ready++;
		if(room->members[idx].loaded) loaded++;
		if(room->members[idx].terminated) terminated++;
		if((room->members[idx].p2p_status|(1<<idx))==p2pmask) p2p++;
		if(strcmp(room->members[idx].conn->nick, room->microphone[0].nick)==0) singer = idx;
		count++;
	}
	if(count==0) {
		for(idx=0; idx<sizeof(room_list)/sizeof(room_list[0]); idx++) {
			if(room_list[idx]==room) break;
		}
		assert(idx<sizeof(room_list)/sizeof(room_list[0]));
		room_list[idx] = NULL;
		mempool_free(room_pool, room);
		return;
	}

	if(singer==-1 && room->state!=CUBE_ROOM_STATE_ACTIVE) {
		cube_room_terminate(room);
		return;
	}
	if(count==terminated && room->state!=CUBE_ROOM_STATE_ACTIVE) {
		cube_room_terminate(room);
		return;
	}
	if(count==ready && singer>=0 && room->state==CUBE_ROOM_STATE_ACTIVE) {
		room->state = CUBE_ROOM_STATE_LOADING;
		room->start_time = cube_room_curtime(room) + CUBE_ROOM_TIMEOUT/CUBE_ROOM_TIMER;
		strcpy(room->singer, room->members[singer].conn->nick);
		for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
			SVR_USER_CTX ctx;
			if(room->members[idx].conn==NULL) continue;
			ctx.conn = room->members[idx].conn;
			room_notify_load(&ctx);
			room->members[idx].loaded = 0;
			room->members[idx].terminated = 0;
			room->members[idx].p2p_status = 0;
		}
		return;
	}
	if(count==loaded && count==p2p && room->state==CUBE_ROOM_STATE_LOADING) {
		room->state = CUBE_ROOM_STATE_GAMING;
		for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
			SVR_USER_CTX ctx;
			if(room->members[idx].conn==NULL) continue;
			ctx.conn = room->members[idx].conn;
			room_notify_start(&ctx);
		}
		return;
	}
}

int cube_room_member_count(CUBE_ROOM* room)
{
	int idx, count;
	for(count=idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn) count++;
	}
	return count;
}

int cube_room_member_index(CUBE_ROOM* room, const char* nick)
{
	int idx;
	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) continue;
		if(strcmp(nick, room->members[idx].conn->nick)==0) return idx;
	}
	return -1;
}

int cube_room_get_singer(CUBE_ROOM* room)
{
	int idx;
	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) continue;
		if(strcmp(room->members[idx].conn->nick, room->microphone[0].nick)==0) return idx;
	}
	return -1;
}

int cube_room_joinable(CUBE_ROOM* room, CUBE_CONNECTION* conn)
{
	int idx;

	if(conn->room && conn->room->state!=CUBE_ROOM_STATE_ACTIVE) {
		return ERR_UNKNOWN;
	}
	if(room==NULL) return ERR_NOERROR;

	if(room->state!=CUBE_ROOM_STATE_ACTIVE) {
		return ERR_UNKNOWN;
	}

	if(strcmp(room->owner, conn->nick)==0) {
		assert(room->members[0].conn==conn || room->members[0].conn==NULL);
		return room->members[0].conn?ERR_EXISTED:ERR_NOERROR;
	}

	idx = (strcmp(room->owner, "")==0?1:0);
	for(; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) return ERR_NOERROR;
	}

	return ERR_FULL;
}

void cube_room_tick(CUBE_ROOM* room)
{
	int singer, idx;

	if(room->state!=CUBE_ROOM_STATE_LOADING) return;
	if(room->start_time>cube_room_curtime(room)) return;

	singer = cube_room_get_singer(room);
	if(singer<0) return;

	if(!room->members[singer].loaded) {
		cube_room_leave(room, room->members[singer].conn);
		return;
	}

	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) continue;
		if(singer==idx) continue;
		if(room->members[idx].p2p_status&(1<<singer)) continue;
		cube_room_leave(room, room->members[idx].conn);
	}
}

unsigned int cube_room_curtime(CUBE_ROOM* room)
{
	return cur_time;
}

unsigned int cube_room_p2pmask(CUBE_ROOM* room, int midx)
{
	int idx;
	unsigned int ret;
	if(midx>=0) return (1<<(midx));
	ret = 0;
	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) continue;
		ret |= (1<<idx);
	}
	return ret;
}

void cube_room_onjoin(CUBE_ROOM* room, CUBE_CONNECTION* conn)
{
	int idx;
	SVR_USER_CTX ctx;

	strcpy(room->members[conn->room_idx].pos, "default");

	ctx.conn = conn;
	lobby_room_callback(&ctx, ERR_NOERROR, room->owner, room->name);

	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) continue;

		ctx.conn = conn;
		room_notify_join(&ctx, room->members[idx].conn->index, room->members[idx].conn->nick, room->members[idx].conn->ri, room->members[idx].conn->equ);
		room_walk_callback(&ctx, room->members[idx].conn->nick, room->members[idx].pos);
		if(room->members[idx].ready) room_notify_ready(&ctx, room->members[idx].conn->nick, room->members[idx].ready);

		if(room->members[idx].conn==conn) continue;

		ctx.conn = room->members[idx].conn;
		room_notify_join(&ctx, conn->index, conn->nick, room->members[conn->room_idx].conn->ri, room->members[conn->room_idx].conn->equ);
		room_walk_callback(&ctx, conn->nick, room->members[conn->room_idx].pos);
	}

	cube_room_sync(room, conn);

	ctx.conn = conn;
	room_info_callback(&ctx, room->name, room->owner);
}

void cube_room_terminate(CUBE_ROOM* room)
{
	int i, j;
	SVR_USER_CTX ctx;

	cube_room_giveup(room, room->singer);
	strcpy(room->singer, "");

	for(i=0; i<sizeof(room->members)/sizeof(room->members[0]); i++) {
		if(room->members[i].conn==NULL) continue;
		ctx.conn = room->members[i].conn;
		room->members[i].ready = 0;

		room_notify_terminate(&ctx);

		for(j=0; j<sizeof(room->members)/sizeof(room->members[0]); j++) {
			if(room->members[j].conn==NULL) continue;
			room_notify_join(&ctx, room->members[j].conn->index, room->members[j].conn->nick, room->members[j].conn->ri, room->members[j].conn->equ);
			room_walk_callback(&ctx, room->members[j].conn->nick, room->members[j].pos);
		}

		cube_room_sync(room, room->members[i].conn);
		room_info_callback(&ctx, room->name, room->owner);
	}

	room->state = CUBE_ROOM_STATE_ACTIVE;
}

void cube_room_sync(CUBE_ROOM* room, CUBE_CONNECTION* conn)
{
	int idx, i;
	SVR_USER_CTX ctx;

	for(i=0; i<sizeof(room->members)/sizeof(room->members[0]); i++) {
		if(room->members[i].conn==NULL) continue;
		if(conn!=NULL && conn!=room->members[i].conn) continue;
		ctx.conn = room->members[i].conn;

		for(idx=0; idx<sizeof(room->microphone)/sizeof(room->microphone[0]); idx++) {
			if(strcmp(room->microphone[idx].nick, "")==0) continue;
			room_microphone_callback(&ctx, room->microphone[idx].nick, room->microphone[idx].song);
		}
		room_microphone_end(&ctx);
	}
}

void cube_room_acquire(CUBE_ROOM* room, const char* nick, const char* song)
{
	int idx;

	for(idx=0; idx<sizeof(room->microphone)/sizeof(room->microphone[0]); idx++) {
		if(strcmp(nick, room->microphone[idx].nick)==0) return;
	}

	for(idx=0; idx<sizeof(room->microphone)/sizeof(room->microphone[0]); idx++) {
		if(strcmp(room->microphone[idx].nick, "")==0) break;
	}
	assert(idx<sizeof(room->microphone)/sizeof(room->microphone[0]));
	if(idx==sizeof(room->microphone)/sizeof(room->microphone[0])) return;

	strcpy(room->microphone[idx].nick, nick);
	strcpy(room->microphone[idx].song, song);

	cube_room_sync(room, NULL);
}

void cube_room_giveup(CUBE_ROOM* room, const char* nick)
{
	int idx, max;

	if(strcmp(nick, "")==0) return;

	for(idx=0; idx<sizeof(room->microphone)/sizeof(room->microphone[0]); idx++) {
		if(strcmp(nick, room->microphone[idx].nick)==0) break;
	}
	if(idx==sizeof(room->microphone)/sizeof(room->microphone[0])) return;

	max = sizeof(room->microphone)/sizeof(room->microphone[0]);
	memmove(&room->microphone[idx], &room->microphone[idx+1], (max-idx-1)*sizeof(room->microphone[0]));
	memset(&room->microphone[max-1], 0, sizeof(room->microphone[0]));

	cube_room_sync(room, NULL);
}

int cube_can_change_equipment(CUBE_CONNECTION* conn)
{
	if(conn->room==NULL) return 1;
	if(conn->room->state!=CUBE_ROOM_STATE_ACTIVE) return 0;
	if(conn->room->members[conn->room_idx].ready) return 0;
	return 1;
}
