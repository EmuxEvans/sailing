
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cube_pdl.CubeServer.h"
#include "cube_server.h"

void login_login(SVR_USER_CTX* user_ctx, const char* token)
{
	int ret;
	DBAPI_HANDLE handle;
	char username[100];
	char password[100];
	char sql[100];
	DBAPI_RECORDSET* rs;
	int idx;

	if(user_ctx->conn->uuid>0) return;

	if(sscanf(token, "%s %s", username, password)!=2) {
		login_login_callback(user_ctx, ERR_UNKNOWN, "");
		return;
	}

	handle = dbapi_connect(cube_dbstr);
	if(handle==NULL) {
		login_login_callback(user_ctx, ERR_UNKNOWN, "");
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to dbapi_connect(\"%s\")", cube_dbstr);
		return;
	}

	sprintf(sql, "select uuid, nick, ri, wh, equ from account where username='%s' and password='%s'", username, password);
	ret = dbapi_query(handle, sql, &rs, 1);
	if(ret!=ERR_NOERROR) {
		login_login_callback(user_ctx, ERR_UNKNOWN, "");
		dbapi_release(handle);
		return;
	}
	user_ctx->conn->uuid = atoi(dbapi_recordset_get(rs, 0, 0));
	strcpy(user_ctx->conn->nick, dbapi_recordset_get(rs, 0, 1));
	strcpy(user_ctx->conn->ri, dbapi_recordset_get(rs, 0, 2));
	strcpy(user_ctx->conn->wh, dbapi_recordset_get(rs, 0, 3));
	strcpy(user_ctx->conn->equ, dbapi_recordset_get(rs, 0, 4));
	dbapi_recordset_free(rs);

	dbapi_release(handle);

	for(idx=0; idx<sizeof(conn_list)/sizeof(conn_list[0]); idx++) {
		if(conn_list[idx]==NULL || conn_list[idx]==user_ctx->conn) continue;
		if(conn_list[idx]->uuid==user_ctx->conn->uuid) {
			network_disconnect(conn_list[idx]->handle);
			login_login_callback(user_ctx, ERR_EXISTED, "");
			memset(user_ctx->conn->nick, 0, sizeof(user_ctx->conn->nick));
			memset(user_ctx->conn->ri, 0, sizeof(user_ctx->conn->ri));
			memset(user_ctx->conn->wh, 0, sizeof(user_ctx->conn->wh));
			memset(user_ctx->conn->equ, 0, sizeof(user_ctx->conn->equ));
			user_ctx->conn->uuid = 0;
			return;
		}
	}

	if(user_ctx->conn->nick[0]=='\0') {
		login_login_callback(user_ctx, ERR_NOT_FOUND, "");
	} else {
		login_login_callback(user_ctx, ERR_NOERROR, user_ctx->conn->nick);
	}
}

void login_create_player(SVR_USER_CTX* user_ctx, const char* nick, const char* role)
{
	int ret;
	DBAPI_HANDLE handle;
	char sql[100];

	if(user_ctx->conn->uuid==0) return;
	if(user_ctx->conn->nick[0]!='\0') return;
	if(nick[0]=='\0') return;

	handle = dbapi_connect(cube_dbstr);
	if(handle==NULL) {
		login_create_player_callback(user_ctx, ERR_UNKNOWN);
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to dbapi_connect(\"%s\")", cube_dbstr);
		return;
	}

	sprintf(sql, "update account set nick='%s', ri='%s' where uuid=%d", nick, role, user_ctx->conn->uuid);
	ret = dbapi_execute(handle, sql);
	if(ret!=ERR_NOERROR) {
		login_create_player_callback(user_ctx, ERR_UNKNOWN);
		dbapi_release(handle);
		return;
	}

	dbapi_release(handle);

	strcpy(user_ctx->conn->ri, role);
	login_create_player_callback(user_ctx, ERR_NOERROR);
}

void lobby_roomlist_get(SVR_USER_CTX* user_ctx)
{
	int idx;
	for(idx=0; idx<sizeof(room_list)/sizeof(room_list[0]); idx++) {
		if(room_list[idx]==NULL) continue;
		if(room_list[idx]->owner[0]!='\0') continue;
		lobby_roomlist_callback(user_ctx, idx, room_list[idx]->name, cube_room_member_count(room_list[idx]), room_list[idx]->state);
	}
	lobby_roomlist_end(user_ctx);
}

void lobby_room_create(SVR_USER_CTX* user_ctx, const char* name)
{
	CUBE_ROOM* room;
	if(user_ctx->conn->room!=NULL) return;

	room = cube_room_create(user_ctx->conn, name, "");
	if(room==NULL) {
		lobby_room_callback(user_ctx, ERR_UNKNOWN, "", "");
		return;
	}

	room->members[0].conn = user_ctx->conn;
	user_ctx->conn->room = room;
	user_ctx->conn->room_idx = 0;

	cube_room_onjoin(room, user_ctx->conn);
}

void lobby_room_join(SVR_USER_CTX* user_ctx, int index)
{
	int idx;
	CUBE_ROOM* room;

	if(index<0 || index>=sizeof(room_list)/sizeof(room_list[0]) || room_list[index]==NULL) {
		lobby_room_callback(user_ctx, ERR_UNKNOWN, "", "");
		return;
	}
	room = room_list[index];
	if(room->state!=CUBE_ROOM_STATE_ACTIVE) {
	}
	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) break;
	}
	if(idx==sizeof(room->members)/sizeof(room->members[0])) {
	}

	room->members[idx].conn = user_ctx->conn;
	user_ctx->conn->room = room;
	user_ctx->conn->room_idx = idx;

	cube_room_onjoin(room, user_ctx->conn);
}

void lobby_room_join_owner(SVR_USER_CTX* user_ctx, const char* nick)
{
	CUBE_ROOM* room;
	int idx, selfhome;

	if(user_ctx->conn->room!=NULL) return;
	selfhome = (strcmp(user_ctx->conn->nick, nick)==0);

	for(idx=0; idx<sizeof(room_list)/sizeof(room_list[0]); idx++) {
		if(room_list[idx]==NULL) continue;
		if(strcmp(room_list[idx]->owner, nick)==0) break;
	}
	if(idx<sizeof(room_list)/sizeof(room_list[0])) {
		room = room_list[idx];
		if(!selfhome) {
			for(idx=1; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
				if(room->members[idx].conn==NULL) break;
			}
			if(idx==sizeof(room->members)/sizeof(room->members[0])) {
				lobby_room_callback(user_ctx, ERR_UNKNOWN, "", "");
				return;
			}
		} else {
			idx = 0;
		}
	}

	if(room==NULL) {
		room = cube_room_create(user_ctx->conn, "", nick);
		if(room==NULL) {
			lobby_room_callback(user_ctx, ERR_UNKNOWN, "", "");
			return;
		}
		idx = (selfhome?0:1);
	}

	assert(room->members[idx].conn==NULL);
	if(room->members[idx].conn) {
		cube_room_check(room);
		return;
	}

	room->members[idx].conn = user_ctx->conn;
	user_ctx->conn->room = room;
	user_ctx->conn->room_idx = idx;

	cube_room_onjoin(room, user_ctx->conn);
}

void lobby_chat(SVR_USER_CTX* user_ctx, const char* what)
{
	int idx;
	SVR_USER_CTX ctx;

	for(idx=0; idx<sizeof(conn_list)/sizeof(conn_list[0]); idx++) {
		if(conn_list[idx]==NULL) continue;
		ctx.conn = conn_list[idx];
		lobby_chat_callback(&ctx, user_ctx->conn->nick, what);
	}
}

void lobby_roleinfo_set(SVR_USER_CTX* user_ctx, const char* value)
{
	int ret;
	DBAPI_HANDLE handle;
	char sql[100*1024];

	handle = dbapi_connect(cube_dbstr);
	if(handle==NULL) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to dbapi_connect(\"%s\")", cube_dbstr);
		return;
	}
	sprintf(sql, "update account set ri='%s' where uuid=%d", value, user_ctx->conn->uuid);
	ret = dbapi_execute(handle, sql);
	if(ret!=ERR_NOERROR) {
		dbapi_release(handle);
		return;
	}
	dbapi_release(handle);

	strcpy(user_ctx->conn->ri, value);
}

void lobby_roleinfo_get(SVR_USER_CTX* user_ctx)
{
	lobby_roleinfo_callback(user_ctx, user_ctx->conn->ri);
}

void lobby_warehouse_set(SVR_USER_CTX* user_ctx, const char* value)
{
	int ret;
	DBAPI_HANDLE handle;
	char sql[100*1024];

	handle = dbapi_connect(cube_dbstr);
	if(handle==NULL) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to dbapi_connect(\"%s\")", cube_dbstr);
		return;
	}
	sprintf(sql, "update account set wh='%s' where uuid=%d", value, user_ctx->conn->uuid);
	ret = dbapi_execute(handle, sql);
	if(ret!=ERR_NOERROR) {
		dbapi_release(handle);
		return;
	}
	dbapi_release(handle);

	strcpy(user_ctx->conn->wh, value);
}

void lobby_warehouse_get(SVR_USER_CTX* user_ctx)
{
	lobby_warehouse_callback(user_ctx, user_ctx->conn->wh);
}

void lobby_equipment_set(SVR_USER_CTX* user_ctx, const char* value)
{
	int ret;
	DBAPI_HANDLE handle;
	char sql[100*1024];

	if(!cube_can_change_equipment(user_ctx->conn)) return;

	handle = dbapi_connect(cube_dbstr);
	if(handle==NULL) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to dbapi_connect(\"%s\")", cube_dbstr);
		return;
	}
	sprintf(sql, "update account set equ='%s' where uuid=%d", value, user_ctx->conn->uuid);
	ret = dbapi_execute(handle, sql);
	if(ret!=ERR_NOERROR) {
		dbapi_release(handle);
		return;
	}
	dbapi_release(handle);

	strcpy(user_ctx->conn->equ, value);
}

void lobby_equipment_get(SVR_USER_CTX* user_ctx)
{
	lobby_equipment_callback(user_ctx, user_ctx->conn->equ);
}

void room_leave(SVR_USER_CTX* user_ctx)
{
	if(user_ctx->conn->room==NULL) return;
	cube_room_leave(user_ctx->conn->room, user_ctx->conn);
}

void room_set_ready(SVR_USER_CTX* user_ctx, int flag)
{
	int idx;
	CUBE_ROOM* room;
	SVR_USER_CTX ctx;
	room = user_ctx->conn->room;
	if(room==NULL) return;
	if(room->state!=CUBE_ROOM_STATE_ACTIVE) return;
	room->members[user_ctx->conn->room_idx].ready = flag;
	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) continue;
		ctx.conn = room->members[idx].conn;
		room_notify_ready(&ctx, user_ctx->conn->nick, flag);
	}

	cube_room_check(room);
}

void room_loaded(SVR_USER_CTX* user_ctx)
{
	CUBE_ROOM* room;
	room = user_ctx->conn->room;
	if(room==NULL) return;
	room->members[user_ctx->conn->room_idx].loaded = 1;

	cube_room_check(room);
}

void room_p2p_reged(SVR_USER_CTX* user_ctx)
{
}

void room_p2p_connected(SVR_USER_CTX* user_ctx, const char* nick)
{
	CUBE_ROOM* room;
	int midx;
	room = user_ctx->conn->room;
	if(room==NULL) return;
	midx = cube_room_member_index(room, nick);
	assert(midx>=0);
	if(midx<0) return;
	room->members[user_ctx->conn->room_idx].p2p_status |= (1<<midx);

	cube_room_check(room);
}

void room_terminate(SVR_USER_CTX* user_ctx)
{
	CUBE_ROOM* room;
	room = user_ctx->conn->room;
	if(room==NULL) return;
	if(room->state!=CUBE_ROOM_STATE_ACTIVE) return;
	room->state = CUBE_ROOM_STATE_ACTIVE;
	cube_room_terminate(room);
}

void room_chat(SVR_USER_CTX* user_ctx, const char* what)
{
	int idx;
	CUBE_ROOM* room;
	SVR_USER_CTX ctx;

	if(user_ctx->conn->room==NULL) return;
	room = user_ctx->conn->room;

	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) continue;
		room_chat_callback(&ctx, user_ctx->conn->nick, what);
	}
}

void room_xuxu(SVR_USER_CTX* user_ctx, int loud, const char* who, const char* what)
{
	CUBE_ROOM* room;
	int idx;
	SVR_USER_CTX ctx;

	if(user_ctx->conn->room==NULL) return;
	room = user_ctx->conn->room;

	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) continue;
		if(strcmp(room->members[idx].conn->nick, who)==0) break;
	}
	if(idx==sizeof(room->members)/sizeof(room->members[0])) return;

	if(loud) {
		for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
			if(room->members[idx].conn==NULL) continue;
			ctx.conn = room->members[idx].conn;
			room_xuxu_callback(&ctx, loud, user_ctx->conn->nick, who, what);
		}
	} else {
		ctx.conn = room->members[idx].conn;
		room_xuxu_callback(&ctx, loud, user_ctx->conn->nick, who, what);
	}
}

void room_walk(SVR_USER_CTX* user_ctx, const char* pos)
{
	int idx;
	CUBE_ROOM* room;
	SVR_USER_CTX ctx;

	if(user_ctx->conn->room==NULL) return;
	room = user_ctx->conn->room;

	for(idx=0; idx<sizeof(room->members)/sizeof(room->members[0]); idx++) {
		if(room->members[idx].conn==NULL) continue;
		ctx.conn = room->members[idx].conn;
		room_walk_callback(&ctx, user_ctx->conn->nick, pos);
	}

	strcpy(room->members[user_ctx->conn->room_idx].pos, pos);
}

void room_microphone_acquire(SVR_USER_CTX* user_ctx, const char* song)
{
	CUBE_ROOM* room;
	room = user_ctx->conn->room;
	if(room==NULL) return;
	cube_room_acquire(room, user_ctx->conn->nick, song);
}

void room_microphone_giveup(SVR_USER_CTX* user_ctx)
{
	CUBE_ROOM* room;
	room = user_ctx->conn->room;
	if(room==NULL) return;
	cube_room_giveup(room, user_ctx->conn->nick);
}

int SVR_Newstream(SVR_USER_CTX* ctx, STREAM** ptr)
{
	memstream_init(&ctx->stream, ctx->buf+2, sizeof(ctx->buf)-2, 0);
	*ptr = (STREAM*)&ctx->stream;
	return ERR_NOERROR;
}

int SVR_Send(SVR_USER_CTX* ctx, STREAM* stream)
{
	NETWORK_DOWNBUF* downbufs[100];
	unsigned int count;
	int ret;

	*((unsigned short*)ctx->buf) = sizeof(unsigned short) + (unsigned short)ctx->stream.len;
	count = network_downbufs_alloc(downbufs, sizeof(downbufs)/sizeof(downbufs[0]), sizeof(unsigned short) + ctx->stream.len);
	network_downbufs_fill(downbufs, count, 0, ctx->buf, sizeof(unsigned short) + ctx->stream.len);

	ret = network_send(ctx->conn->handle, downbufs, count);
	if(ret!=ERR_NOERROR) {
		network_downbufs_free(downbufs, count);
		return ret;
	}

	return ERR_NOERROR;
}

int SVR_Alloc(SVR_USER_CTX* ctx, STREAM* stream, void** ptr, int size)
{
	*ptr = memstream_get_position((MEM_STREAM*)stream);
	return 0;
}

void SVR_Free(SVR_USER_CTX* ctx, void* ptr)
{
}
