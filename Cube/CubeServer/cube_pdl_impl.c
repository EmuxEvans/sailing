
#include "cube_pdl.CubeServer.h"

void login_login(SVR_USER_CTX* user_ctx, const char* token)
{
}

void login_create_player(SVR_USER_CTX* user_ctx, const char* nick, const char* role)
{
}

void lobby_roomlist_get(SVR_USER_CTX* user_ctx)
{
}

void lobby_chat(SVR_USER_CTX* user_ctx)
{
}

void lobby_warehouse_get(SVR_USER_CTX* user_ctx)
{
}

void lobby_equipment_get(SVR_USER_CTX* user_ctx)
{
}

void room_chat(SVR_USER_CTX* user_ctx)
{
}

void room_walk(SVR_USER_CTX* user_ctx)
{
}

void room_set_singer(SVR_USER_CTX* user_ctx)
{
}

void room_load_complete(SVR_USER_CTX* user_ctx)
{
}

void room_set_ready(SVR_USER_CTX* user_ctx)
{
}

int SVR_Newstream(SVR_USER_CTX* ctx, STREAM** ptr)
{
	return 0;
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
