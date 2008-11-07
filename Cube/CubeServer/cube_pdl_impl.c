
#include <stdlib.h>

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

	if(user_ctx->conn->uid>=0) return;

	if(sscanf(token, "%s %s", username, password)!=2) {
		login_login_callback(user_ctx, ERR_UNKNOWN);
		return;
	}

	handle = dbapi_connect(cube_dbstr);
	if(handle==NULL) {
		login_login_callback(user_ctx, ERR_UNKNOWN);
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to dbapi_connect(\"%s\")", cube_dbstr);
		return;
	}

	sprintf(sql, "select uid, nick, ri, wh, equ from account where username='%s' and password='%s'", username, password);
	ret = dbapi_query(handle, sql, &rs, 1);
	if(ret!=ERR_NOERROR) {
		login_login_callback(user_ctx, ERR_UNKNOWN);
		dbapi_release(handle);
		return;
	}
	user_ctx->conn->uid = atoi(dbapi_recordset_get(rs, 0, 0));
	strcpy(user_ctx->conn->nick, dbapi_recordset_get(rs, 0, 1));
	strcpy(user_ctx->conn->ri, dbapi_recordset_get(rs, 0, 2));
	strcpy(user_ctx->conn->wh, dbapi_recordset_get(rs, 0, 3));
	strcpy(user_ctx->conn->equ, dbapi_recordset_get(rs, 0, 4));
	dbapi_recordset_free(rs);

	dbapi_release(handle);

	if(user_ctx->conn->nick[0]=='\0') {
		login_login_callback(user_ctx, ERR_NOT_FOUND);
	} else {
		login_login_callback(user_ctx, ERR_NOERROR);
	}
}

void login_create_player(SVR_USER_CTX* user_ctx, const char* nick, const char* role)
{
	int ret;
	DBAPI_HANDLE handle;
	char sql[100];

	if(user_ctx->conn->uid==0) return;
	if(user_ctx->conn->nick[0]!='\0') return;
	if(nick[0]=='\0') return;

	handle = dbapi_connect(cube_dbstr);
	if(handle==NULL) {
		login_login_callback(user_ctx, ERR_UNKNOWN);
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to dbapi_connect(\"%s\")", cube_dbstr);
		return;
	}

	sprintf(sql, "update account set nick='%s', ri='%s' where uid=%d", nick, role, user_ctx->conn->uid);
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
		if(room_list[idx]->singer>=0) {
			lobby_roomlist_callback(user_ctx, idx, room_list[idx]->name, room_list[idx]->conns[room_list[idx]->singer]->nick, room_list[idx]->map, room_list[idx]->state);
		} else {
			lobby_roomlist_callback(user_ctx, idx, room_list[idx]->name, "", room_list[idx]->map, room_list[idx]->state);
		}
	}
	lobby_roomlist_end(user_ctx);
}

void lobby_room_create(SVR_USER_CTX* user_ctx, const char* name, const char* map)
{
	CUBE_ROOM* room;
	if(user_ctx->conn->room!=NULL) return;

	room = cube_room_create(user_ctx->conn, name, map);
	if(room==NULL) {
		lobby_room_callback(user_ctx, ERR_UNKNOWN, "");
		return;
	}

	cube_room_onjoin(room, user_ctx->conn);
}

void lobby_room_join(SVR_USER_CTX* user_ctx, int index)
{
	int idx;
	CUBE_ROOM* room;

	if(index<0 || index>=sizeof(room_list)/sizeof(room_list[0]) || room_list[index]==NULL) {
		lobby_room_callback(user_ctx, ERR_UNKNOWN, "");
		return;
	}
	room = room_list[index];
	if(room->state!=CUBE_ROOM_STATE_ACTIVE) {
	}
	for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
		if(room->conns[idx]==NULL) break;
	}
	if(idx==sizeof(room->conns)/sizeof(room->conns[0])) {
	}

	room->conns[idx] = user_ctx->conn;

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
	sprintf(sql, "update account set ri='%s' where uid=%d", value, user_ctx->conn->uid);
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
	sprintf(sql, "update account set wh='%s' where uid=%d", value, user_ctx->conn->uid);
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
	sprintf(sql, "update account set equ='%s' where uid=%d", value, user_ctx->conn->uid);
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

void room_info_set(SVR_USER_CTX* user_ctx, const char* singer, const char* map)
{
	int idx, sidx;
	CUBE_ROOM* room;
	SVR_USER_CTX ctx;

	if(user_ctx->conn->room==NULL) return;
	room = user_ctx->conn->room;

	if(room->singer>=0 && room->conns[room->singer]!=user_ctx->conn) return;
	if(room->state!=CUBE_ROOM_STATE_ACTIVE) return;

	sidx = -1;
	for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
		if(room->conns[idx]==NULL) continue;
		if(strcmp(room->conns[idx]->nick, singer)!=0) continue;
		sidx = idx;
	}
	room->singer = sidx;
	if(map[0]!='\0') strcpy(room->map, map);

	for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
		if(room->conns[idx]==NULL) continue;
		ctx.conn = room->conns[idx];
		if(room->singer>=0) {
			room_info_callback(&ctx, room->name, room->conns[room->singer]->nick, room->map);
		} else {
			room_info_callback(&ctx, room->name, "", room->map);
		}
	}
}

void room_leave(SVR_USER_CTX* user_ctx)
{
	if(user_ctx->conn->room==NULL) return;
	cube_room_leave(user_ctx->conn->room, user_ctx->conn);
}

void room_chat(SVR_USER_CTX* user_ctx, const char* what)
{
	int idx;
	CUBE_ROOM* room;
	SVR_USER_CTX ctx;

	if(user_ctx->conn->room==NULL) return;
	room = user_ctx->conn->room;

	for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
		if(room->conns[idx]==NULL) continue;
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

	for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
		if(room->conns[idx]==NULL) continue;
		if(strcmp(room->conns[idx]->nick, who)==0) break;
	}
	if(idx==sizeof(room->conns)/sizeof(room->conns[0])) return;

	if(loud) {
		for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
			if(room->conns[idx]==NULL) continue;
			ctx.conn = room->conns[idx];
			room_xuxu_callback(&ctx, loud, user_ctx->conn->nick, who, what);
		}
	} else {
		ctx.conn = room->conns[idx];
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

	for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
		if(room->conns[idx]==NULL) continue;
		room_walk_callback(&ctx, user_ctx->conn->nick, pos);
	}
}

void room_load_complete(SVR_USER_CTX* user_ctx)
{
	CUBE_ROOM* room;
	room = user_ctx->conn->room;
	if(room==NULL) return;
	if(room->state!=CUBE_ROOM_STATE_LOADING) return;
	room->loaded[user_ctx->conn->room_idx] = 1;
}

void room_set_ready(SVR_USER_CTX* user_ctx, int flag)
{
	int idx;
	CUBE_ROOM* room;
	SVR_USER_CTX ctx;
	room = user_ctx->conn->room;
	if(room==NULL) return;
	if(room->state!=CUBE_ROOM_STATE_ACTIVE) return;
	room->readys[user_ctx->conn->room_idx] = flag;
	for(idx=0; idx<sizeof(room->conns)/sizeof(room->conns[0]); idx++) {
		if(room->conns[idx]==NULL) continue;
		ctx.conn = room->conns[idx];
		room_notify_ready(&ctx, user_ctx->conn->nick, flag);
	}
}

void room_terminate(SVR_USER_CTX* user_ctx)
{
	CUBE_ROOM* room;
	room = user_ctx->conn->room;
	if(room==NULL) return;
	if(room->state!=CUBE_ROOM_STATE_ACTIVE) return;
	if(room->conns[room->singer]!=user_ctx->conn) return;
	room->state = CUBE_ROOM_STATE_ACTIVE;
	memset(room->readys, 0, sizeof(room->readys));
	cube_room_terminate(room);
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

	*((unsigned short*)ctx->buf) = (unsigned short)ctx->stream.len;
	count = network_downbufs_alloc(downbufs, sizeof(downbufs)/sizeof(downbufs[0]), ctx->stream.len+2);
	network_downbufs_fill(downbufs, count, 0, ctx->buf, ctx->stream.len+2);

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
