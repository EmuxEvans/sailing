
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

void room_set_singer(SVR_USER_CTX* user_ctx)
{
	CUBE_ROOM* room;
	room = user_ctx->conn->room;
	if(room==NULL) return;
	if(room->state!=CUBE_ROOM_STATE_ACTIVE) return;
}

void room_load_complete(SVR_USER_CTX* user_ctx)
{
	CUBE_ROOM* room;
	room = user_ctx->conn->room;
	if(room==NULL) return;
	if(room->state!=CUBE_ROOM_STATE_LOADING) return;
}

void room_set_ready(SVR_USER_CTX* user_ctx)
{
	CUBE_ROOM* room;
	room = user_ctx->conn->room;
	if(room==NULL) return;
	if(room->state!=CUBE_ROOM_STATE_ACTIVE) return;
}

int SVR_Newstream(SVR_USER_CTX* ctx, STREAM** ptr)
{
	memstream_init(&ctx->stream, ctx->buf, sizeof(ctx->buf), 0);
	*ptr = (STREAM*)&ctx->stream;
	return ERR_NOERROR;
}

int SVR_Send(SVR_USER_CTX* ctx, STREAM* stream)
{
	return 0;
}

int SVR_Alloc(SVR_USER_CTX* ctx, STREAM* stream, void** ptr, int size)
{
	return 0;
}

void SVR_Free(SVR_USER_CTX* ctx, void* ptr)
{
}
