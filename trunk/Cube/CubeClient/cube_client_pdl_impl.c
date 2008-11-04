
#include "cube_pdl.CubeClient.h"

void login_login_callback(CLT_USER_CTX* user_ctx, int ret)
{
}

void login_create_player_callback(CLT_USER_CTX* user_ctx, int ret)
{
}

void lobby_roomlist_callback(CLT_USER_CTX* user_ctx)
{
}

void lobby_chat_callback(CLT_USER_CTX* user_ctx)
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

void room_join_callback(CLT_USER_CTX* user_ctx)
{
}

void room_leave(CLT_USER_CTX* user_ctx)
{
}

void room_leave_callback(CLT_USER_CTX* user_ctx)
{
}

void room_chat_callback(CLT_USER_CTX* user_ctx, const char* nick, const char* what)
{
}

void room_walk_callback(CLT_USER_CTX* user_ctx)
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
