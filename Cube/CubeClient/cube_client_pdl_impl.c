
#include "cube_pdl.CubeClient.h"

void login_login_callback(CLT_USER_CTX* user_ctx, int ret)
{
}

void login_create_player_callback(CLT_USER_CTX* user_ctx, int ret)
{
}

void lobby_roomlist_callback(CLT_USER_CTX* user_ctx, int index, const char* name, const char* map, const char* singer, int state)
{
}

void lobby_roomlist_end(CLT_USER_CTX* user_ctx)
{
}

void lobby_room_callback(CLT_USER_CTX* user_ctx, int ret, const char* name)
{
}

void lobby_chat_callback(CLT_USER_CTX* user_ctx, const char* nick, const char* what)
{
}

void lobby_roleinfo_callback(CLT_USER_CTX* user_ctx, const char* value)
{
}

void lobby_warehouse_callback(CLT_USER_CTX* user_ctx, const char* value)
{
}

void lobby_equipment_callback(CLT_USER_CTX* user_ctx, const char* value)
{
}

void room_info_callback(CLT_USER_CTX* user_ctx, const char* name, const char* singer, const char* map)
{
}

void room_notify_join(CLT_USER_CTX* user_ctx, const char* nick, const char* equ)
{
}

void room_notify_leave(CLT_USER_CTX* user_ctx, const char* nick)
{
}

void room_notify_load(CLT_USER_CTX* user_ctx)
{
}

void room_notify_start(CLT_USER_CTX* user_ctx)
{
}

void room_notify_terminate(CLT_USER_CTX* user_ctx)
{
}

void room_notify_ready(CLT_USER_CTX* user_ctx, const char* nick, int flag)
{
}

void room_chat_callback(CLT_USER_CTX* user_ctx, const char* nick, const char* what)
{
}

void room_xuxu_callback(CLT_USER_CTX* user_ctx, int loud, const char* from, const char* who, const char* what)
{
}

void room_walk_callback(CLT_USER_CTX* user_ctx, const char* nick, const char* pos)
{
}

void room_status_callback(CLT_USER_CTX* user_ctx)
{
}

int CLT_Newstream(CLT_USER_CTX* ctx, STREAM** ptr)
{
	return 0;
}

int CLT_Send(CLT_USER_CTX* ctx, STREAM* stream)
{
	return 0;
}

int CLT_Alloc(CLT_USER_CTX* ctx, STREAM* stream, void** ptr, int size)
{
	return 0;
}

void CLT_Free(CLT_USER_CTX* ctx, void* ptr){
}
