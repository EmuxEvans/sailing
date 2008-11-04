#ifndef _CLT_INCLUDE
#define _CLT_INCLUDE

#include <skates/skates.h>

#ifdef __cplusplus
extern "C" {
#endif

// C_DEF : BEGIN

struct CLT_USER_CTX;
typedef struct CLT_USER_CTX CLT_USER_CTX;
struct SVR_USER_CTX;
typedef struct SVR_USER_CTX SVR_USER_CTX;

#define	LOGIN_FILTER_ID		0x1
#define	LOBBY_FILTER_ID		0x2
#define	ROOM_FILTER_ID		0x3

#include "../cube_data.h"

// C_DEF : END

// implement&stub functions : BEGIN
int login_login(CLT_USER_CTX* user_ctx, const char* token);
void login_login_callback(CLT_USER_CTX* user_ctx, int ret);
int login_create_player(CLT_USER_CTX* user_ctx, const char* nick, const char* role);
void login_create_player_callback(CLT_USER_CTX* user_ctx, int ret);
int lobby_roomlist_get(CLT_USER_CTX* user_ctx);
void lobby_roomlist_callback(CLT_USER_CTX* user_ctx);
int lobby_chat(CLT_USER_CTX* user_ctx);
void lobby_chat_callback(CLT_USER_CTX* user_ctx);
int lobby_roleinfo_set(CLT_USER_CTX* user_ctx, const char* value);
int lobby_roleinfo_get(CLT_USER_CTX* user_ctx);
void lobby_roleinfo_callback(CLT_USER_CTX* user_ctx, const char* value);
int lobby_warehouse_set(CLT_USER_CTX* user_ctx, const char* value);
int lobby_warehouse_get(CLT_USER_CTX* user_ctx);
void lobby_warehouse_callback(CLT_USER_CTX* user_ctx, const char* value);
int lobby_equipment_set(CLT_USER_CTX* user_ctx, const char* value);
int lobby_equipment_get(CLT_USER_CTX* user_ctx);
void lobby_equipment_callback(CLT_USER_CTX* user_ctx, const char* value);
void room_join_callback(CLT_USER_CTX* user_ctx);
void room_leave(CLT_USER_CTX* user_ctx);
void room_leave_callback(CLT_USER_CTX* user_ctx);
int room_chat(CLT_USER_CTX* user_ctx, const char* what);
void room_chat_callback(CLT_USER_CTX* user_ctx, const char* nick, const char* what);
int room_walk(CLT_USER_CTX* user_ctx);
void room_walk_callback(CLT_USER_CTX* user_ctx);
int room_set_singer(CLT_USER_CTX* user_ctx);
int room_load_complete(CLT_USER_CTX* user_ctx);
int room_set_ready(CLT_USER_CTX* user_ctx);
void room_status_callback(CLT_USER_CTX* user_ctx);
// implement&stub functions : END

// extern dispatcher function
int CLT_Dispatcher(CLT_USER_CTX* ctx, STREAM* stream);
// user define function
int CLT_Newstream(CLT_USER_CTX* ctx, STREAM** ptr);
int CLT_Send(CLT_USER_CTX* ctx, STREAM* stream);
int CLT_Alloc(CLT_USER_CTX* ctx, STREAM* stream, void** ptr, int size);
void CLT_Free(CLT_USER_CTX* ctx, void* ptr);

#ifdef __cplusplus
};
#endif

#endif
