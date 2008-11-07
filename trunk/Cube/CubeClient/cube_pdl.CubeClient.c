#include <stdlib.h>
#include <string.h>
#include "cube_pdl.CubeClient.h"

// Define command : BEGIN
int login_login_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int login_create_player_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int lobby_roomlist_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int lobby_roomlist_end_stub(CLT_USER_CTX* ctx, STREAM* stream);
int lobby_room_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int lobby_chat_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int lobby_roleinfo_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int lobby_warehouse_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int lobby_equipment_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_info_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_notify_join_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_notify_leave_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_notify_load_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_notify_start_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_notify_terminate_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_notify_ready_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_chat_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_xuxu_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_walk_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_status_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
// Define command : END

// extern dispatcher function : BEGIN
int CLT_Dispatcher(CLT_USER_CTX* ctx, STREAM* stream)
{
	unsigned int code;

	if(stream_get(stream, &code, sizeof(code))!=0) return -1;

	switch(code) {
	case 0x00020000|LOGIN_FILTER_ID:	return login_login_callback_stub(ctx, stream);
	case 0x00040000|LOGIN_FILTER_ID:	return login_create_player_callback_stub(ctx, stream);
	case 0x00020000|LOBBY_FILTER_ID:	return lobby_roomlist_callback_stub(ctx, stream);
	case 0x00030000|LOBBY_FILTER_ID:	return lobby_roomlist_end_stub(ctx, stream);
	case 0x00060000|LOBBY_FILTER_ID:	return lobby_room_callback_stub(ctx, stream);
	case 0x00080000|LOBBY_FILTER_ID:	return lobby_chat_callback_stub(ctx, stream);
	case 0x000b0000|LOBBY_FILTER_ID:	return lobby_roleinfo_callback_stub(ctx, stream);
	case 0x000e0000|LOBBY_FILTER_ID:	return lobby_warehouse_callback_stub(ctx, stream);
	case 0x00110000|LOBBY_FILTER_ID:	return lobby_equipment_callback_stub(ctx, stream);
	case 0x00020000|ROOM_FILTER_ID:	return room_info_callback_stub(ctx, stream);
	case 0x00030000|ROOM_FILTER_ID:	return room_notify_join_stub(ctx, stream);
	case 0x00050000|ROOM_FILTER_ID:	return room_notify_leave_stub(ctx, stream);
	case 0x00060000|ROOM_FILTER_ID:	return room_notify_load_stub(ctx, stream);
	case 0x00070000|ROOM_FILTER_ID:	return room_notify_start_stub(ctx, stream);
	case 0x00080000|ROOM_FILTER_ID:	return room_notify_terminate_stub(ctx, stream);
	case 0x00090000|ROOM_FILTER_ID:	return room_notify_ready_stub(ctx, stream);
	case 0x000b0000|ROOM_FILTER_ID:	return room_chat_callback_stub(ctx, stream);
	case 0x000d0000|ROOM_FILTER_ID:	return room_xuxu_callback_stub(ctx, stream);
	case 0x000f0000|ROOM_FILTER_ID:	return room_walk_callback_stub(ctx, stream);
	case 0x00120000|ROOM_FILTER_ID:	return room_status_callback_stub(ctx, stream);
	default: return ERR_UNKNOWN;
	}
}
// extern dispatcher function : END

int login_login(CLT_USER_CTX* user_ctx, const char* token)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = LOGIN_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 1;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // len [token]
	__reserve.len = strlen(token)+1;
    // check rule [token]
    if(!(__reserve.len<=CUBE_LOGIN_TOKEN_LEN+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [token]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [token]
	__reserve.ret = stream_put(__reserve.stream, token, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int login_login_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		int ret;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	ret = stream_get(stream, &__reserve.ret, sizeof(int));
	if(ret!=ERR_NOERROR) goto ON_ERROR;

	// call
	login_login_callback(user_ctx, __reserve.ret);

	// free memory
	// end
	return ERR_NOERROR;

ON_ERROR:

	return ret;
}

int login_create_player(CLT_USER_CTX* user_ctx, const char* nick, const char* role)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = LOGIN_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 3;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // len [nick]
	__reserve.len = strlen(nick)+1;
    // check rule [nick]
    if(!(__reserve.len<=CUBE_NICK_LEN+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [nick]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [nick]
	__reserve.ret = stream_put(__reserve.stream, nick, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // len [role]
	__reserve.len = strlen(role)+1;
    // check rule [role]
    if(!(__reserve.len<=CUBE_ROLEINFO_LEN+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [role]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [role]
	__reserve.ret = stream_put(__reserve.stream, role, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int login_create_player_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		int ret;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	ret = stream_get(stream, &__reserve.ret, sizeof(int));
	if(ret!=ERR_NOERROR) goto ON_ERROR;

	// call
	login_create_player_callback(user_ctx, __reserve.ret);

	// free memory
	// end
	return ERR_NOERROR;

ON_ERROR:

	return ret;
}

int lobby_roomlist_get(CLT_USER_CTX* user_ctx)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = LOBBY_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 1;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int lobby_roomlist_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		int index;
		char* name;
		char* map;
		char* singer;
		int state;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	ret = stream_get(stream, &__reserve.index, sizeof(int));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get [name] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [name]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.name, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [name]
	ret = stream_get(stream, __reserve.name, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.name[len-1] = '\0';
	// get [map] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [map]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.map, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [map]
	ret = stream_get(stream, __reserve.map, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.map[len-1] = '\0';
	// get [singer] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [singer]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.singer, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [singer]
	ret = stream_get(stream, __reserve.singer, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.singer[len-1] = '\0';
	ret = stream_get(stream, &__reserve.state, sizeof(int));
	if(ret!=ERR_NOERROR) goto ON_ERROR;

	// call
	lobby_roomlist_callback(user_ctx, __reserve.index, __reserve.name, __reserve.map, __reserve.singer, __reserve.state);

	// free memory
	CLT_Free(user_ctx, __reserve.singer);
	CLT_Free(user_ctx, __reserve.map);
	CLT_Free(user_ctx, __reserve.name);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.singer!=NULL) CLT_Free(user_ctx, __reserve.singer);
	if(__reserve.map!=NULL) CLT_Free(user_ctx, __reserve.map);
	if(__reserve.name!=NULL) CLT_Free(user_ctx, __reserve.name);

	return ret;
}

int lobby_roomlist_end_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	int len, ret;

	// init
	len = 0;
	ret = 0;

	// read from stream

	// call
	lobby_roomlist_end(user_ctx);

	// free memory
	// end
	return ERR_NOERROR;
}

int lobby_room_create(CLT_USER_CTX* user_ctx, const char* name, const char* map)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = LOBBY_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 4;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // len [name]
	__reserve.len = strlen(name)+1;
    // check rule [name]
    if(!(__reserve.len<=CUBE_ROOM_NAME_LEN+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [name]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [name]
	__reserve.ret = stream_put(__reserve.stream, name, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // len [map]
	__reserve.len = strlen(map)+1;
    // check rule [map]
    if(!(__reserve.len<=CUBE_ROOM_MAP_LEN+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [map]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [map]
	__reserve.ret = stream_put(__reserve.stream, map, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int lobby_room_join(CLT_USER_CTX* user_ctx, int idx)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = LOBBY_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 5;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // write data [idx]
	__reserve.ret = stream_put(__reserve.stream, &idx, sizeof(int)*(1));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int lobby_room_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		int ret;
		char* name;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	ret = stream_get(stream, &__reserve.ret, sizeof(int));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get [name] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [name]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.name, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [name]
	ret = stream_get(stream, __reserve.name, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.name[len-1] = '\0';

	// call
	lobby_room_callback(user_ctx, __reserve.ret, __reserve.name);

	// free memory
	CLT_Free(user_ctx, __reserve.name);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.name!=NULL) CLT_Free(user_ctx, __reserve.name);

	return ret;
}

int lobby_chat(CLT_USER_CTX* user_ctx, const char* what)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = LOBBY_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 7;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // len [what]
	__reserve.len = strlen(what)+1;
    // check rule [what]
    if(!(__reserve.len<=300+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [what]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [what]
	__reserve.ret = stream_put(__reserve.stream, what, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int lobby_chat_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		char* nick;
		char* what;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	// get [nick] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [nick]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.nick, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [nick]
	ret = stream_get(stream, __reserve.nick, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.nick[len-1] = '\0';
	// get [what] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [what]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.what, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [what]
	ret = stream_get(stream, __reserve.what, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.what[len-1] = '\0';

	// call
	lobby_chat_callback(user_ctx, __reserve.nick, __reserve.what);

	// free memory
	CLT_Free(user_ctx, __reserve.what);
	CLT_Free(user_ctx, __reserve.nick);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.what!=NULL) CLT_Free(user_ctx, __reserve.what);
	if(__reserve.nick!=NULL) CLT_Free(user_ctx, __reserve.nick);

	return ret;
}

int lobby_roleinfo_set(CLT_USER_CTX* user_ctx, const char* value)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = LOBBY_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 9;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // len [value]
	__reserve.len = strlen(value)+1;
    // check rule [value]
    if(!(__reserve.len<=CUBE_WAREHOUSE_LEN+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [value]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [value]
	__reserve.ret = stream_put(__reserve.stream, value, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int lobby_roleinfo_get(CLT_USER_CTX* user_ctx)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = LOBBY_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 10;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int lobby_roleinfo_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		char* value;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	// get [value] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [value]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.value, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [value]
	ret = stream_get(stream, __reserve.value, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.value[len-1] = '\0';

	// call
	lobby_roleinfo_callback(user_ctx, __reserve.value);

	// free memory
	CLT_Free(user_ctx, __reserve.value);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.value!=NULL) CLT_Free(user_ctx, __reserve.value);

	return ret;
}

int lobby_warehouse_set(CLT_USER_CTX* user_ctx, const char* value)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = LOBBY_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 12;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // len [value]
	__reserve.len = strlen(value)+1;
    // check rule [value]
    if(!(__reserve.len<=CUBE_WAREHOUSE_LEN+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [value]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [value]
	__reserve.ret = stream_put(__reserve.stream, value, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int lobby_warehouse_get(CLT_USER_CTX* user_ctx)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = LOBBY_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 13;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int lobby_warehouse_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		char* value;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	// get [value] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [value]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.value, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [value]
	ret = stream_get(stream, __reserve.value, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.value[len-1] = '\0';

	// call
	lobby_warehouse_callback(user_ctx, __reserve.value);

	// free memory
	CLT_Free(user_ctx, __reserve.value);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.value!=NULL) CLT_Free(user_ctx, __reserve.value);

	return ret;
}

int lobby_equipment_set(CLT_USER_CTX* user_ctx, const char* value)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = LOBBY_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 15;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // len [value]
	__reserve.len = strlen(value)+1;
    // check rule [value]
    if(!(__reserve.len<=CUBE_EQUIPMENT_LEN+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [value]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [value]
	__reserve.ret = stream_put(__reserve.stream, value, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int lobby_equipment_get(CLT_USER_CTX* user_ctx)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = LOBBY_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 16;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int lobby_equipment_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		char* value;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	// get [value] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [value]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.value, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [value]
	ret = stream_get(stream, __reserve.value, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.value[len-1] = '\0';

	// call
	lobby_equipment_callback(user_ctx, __reserve.value);

	// free memory
	CLT_Free(user_ctx, __reserve.value);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.value!=NULL) CLT_Free(user_ctx, __reserve.value);

	return ret;
}

int room_info_set(CLT_USER_CTX* user_ctx, const char* singer, const char* map)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = ROOM_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 1;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // len [singer]
	__reserve.len = strlen(singer)+1;
    // check rule [singer]
    if(!(__reserve.len<=CUBE_NICK_LEN+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [singer]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [singer]
	__reserve.ret = stream_put(__reserve.stream, singer, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // len [map]
	__reserve.len = strlen(map)+1;
    // check rule [map]
    if(!(__reserve.len<=CUBE_ROOM_MAP_LEN+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [map]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [map]
	__reserve.ret = stream_put(__reserve.stream, map, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int room_info_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		char* name;
		char* singer;
		char* map;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	// get [name] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [name]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.name, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [name]
	ret = stream_get(stream, __reserve.name, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.name[len-1] = '\0';
	// get [singer] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [singer]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.singer, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [singer]
	ret = stream_get(stream, __reserve.singer, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.singer[len-1] = '\0';
	// get [map] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [map]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.map, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [map]
	ret = stream_get(stream, __reserve.map, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.map[len-1] = '\0';

	// call
	room_info_callback(user_ctx, __reserve.name, __reserve.singer, __reserve.map);

	// free memory
	CLT_Free(user_ctx, __reserve.map);
	CLT_Free(user_ctx, __reserve.singer);
	CLT_Free(user_ctx, __reserve.name);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.map!=NULL) CLT_Free(user_ctx, __reserve.map);
	if(__reserve.singer!=NULL) CLT_Free(user_ctx, __reserve.singer);
	if(__reserve.name!=NULL) CLT_Free(user_ctx, __reserve.name);

	return ret;
}

int room_notify_join_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		char* nick;
		char* equ;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	// get [nick] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [nick]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.nick, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [nick]
	ret = stream_get(stream, __reserve.nick, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.nick[len-1] = '\0';
	// get [equ] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [equ]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.equ, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [equ]
	ret = stream_get(stream, __reserve.equ, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.equ[len-1] = '\0';

	// call
	room_notify_join(user_ctx, __reserve.nick, __reserve.equ);

	// free memory
	CLT_Free(user_ctx, __reserve.equ);
	CLT_Free(user_ctx, __reserve.nick);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.equ!=NULL) CLT_Free(user_ctx, __reserve.equ);
	if(__reserve.nick!=NULL) CLT_Free(user_ctx, __reserve.nick);

	return ret;
}

int room_leave(CLT_USER_CTX* user_ctx)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = ROOM_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 4;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int room_notify_leave_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		char* nick;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	// get [nick] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [nick]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.nick, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [nick]
	ret = stream_get(stream, __reserve.nick, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.nick[len-1] = '\0';

	// call
	room_notify_leave(user_ctx, __reserve.nick);

	// free memory
	CLT_Free(user_ctx, __reserve.nick);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.nick!=NULL) CLT_Free(user_ctx, __reserve.nick);

	return ret;
}

int room_notify_load_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	int len, ret;

	// init
	len = 0;
	ret = 0;

	// read from stream

	// call
	room_notify_load(user_ctx);

	// free memory
	// end
	return ERR_NOERROR;
}

int room_notify_start_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	int len, ret;

	// init
	len = 0;
	ret = 0;

	// read from stream

	// call
	room_notify_start(user_ctx);

	// free memory
	// end
	return ERR_NOERROR;
}

int room_notify_terminate_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	int len, ret;

	// init
	len = 0;
	ret = 0;

	// read from stream

	// call
	room_notify_terminate(user_ctx);

	// free memory
	// end
	return ERR_NOERROR;
}

int room_notify_ready_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		char* nick;
		int flag;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	// get [nick] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [nick]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.nick, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [nick]
	ret = stream_get(stream, __reserve.nick, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.nick[len-1] = '\0';
	ret = stream_get(stream, &__reserve.flag, sizeof(int));
	if(ret!=ERR_NOERROR) goto ON_ERROR;

	// call
	room_notify_ready(user_ctx, __reserve.nick, __reserve.flag);

	// free memory
	CLT_Free(user_ctx, __reserve.nick);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.nick!=NULL) CLT_Free(user_ctx, __reserve.nick);

	return ret;
}

int room_chat(CLT_USER_CTX* user_ctx, const char* what)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = ROOM_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 10;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // len [what]
	__reserve.len = strlen(what)+1;
    // check rule [what]
    if(!(__reserve.len<=300+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [what]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [what]
	__reserve.ret = stream_put(__reserve.stream, what, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int room_chat_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		char* nick;
		char* what;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	// get [nick] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [nick]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.nick, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [nick]
	ret = stream_get(stream, __reserve.nick, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.nick[len-1] = '\0';
	// get [what] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [what]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.what, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [what]
	ret = stream_get(stream, __reserve.what, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.what[len-1] = '\0';

	// call
	room_chat_callback(user_ctx, __reserve.nick, __reserve.what);

	// free memory
	CLT_Free(user_ctx, __reserve.what);
	CLT_Free(user_ctx, __reserve.nick);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.what!=NULL) CLT_Free(user_ctx, __reserve.what);
	if(__reserve.nick!=NULL) CLT_Free(user_ctx, __reserve.nick);

	return ret;
}

int room_xuxu(CLT_USER_CTX* user_ctx, int loud, const char* who, const char* what)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = ROOM_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 12;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // write data [loud]
	__reserve.ret = stream_put(__reserve.stream, &loud, sizeof(int)*(1));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // len [who]
	__reserve.len = strlen(who)+1;
    // check rule [who]
    if(!(__reserve.len<=CUBE_NICK_LEN+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [who]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [who]
	__reserve.ret = stream_put(__reserve.stream, who, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // len [what]
	__reserve.len = strlen(what)+1;
    // check rule [what]
    if(!(__reserve.len<=300+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [what]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [what]
	__reserve.ret = stream_put(__reserve.stream, what, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int room_xuxu_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		int loud;
		char* from;
		char* who;
		char* what;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	ret = stream_get(stream, &__reserve.loud, sizeof(int));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get [from] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [from]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.from, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [from]
	ret = stream_get(stream, __reserve.from, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.from[len-1] = '\0';
	// get [who] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [who]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.who, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [who]
	ret = stream_get(stream, __reserve.who, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.who[len-1] = '\0';
	// get [what] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [what]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.what, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [what]
	ret = stream_get(stream, __reserve.what, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.what[len-1] = '\0';

	// call
	room_xuxu_callback(user_ctx, __reserve.loud, __reserve.from, __reserve.who, __reserve.what);

	// free memory
	CLT_Free(user_ctx, __reserve.what);
	CLT_Free(user_ctx, __reserve.who);
	CLT_Free(user_ctx, __reserve.from);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.what!=NULL) CLT_Free(user_ctx, __reserve.what);
	if(__reserve.who!=NULL) CLT_Free(user_ctx, __reserve.who);
	if(__reserve.from!=NULL) CLT_Free(user_ctx, __reserve.from);

	return ret;
}

int room_walk(CLT_USER_CTX* user_ctx, const char* pos)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = ROOM_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 14;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // len [pos]
	__reserve.len = strlen(pos)+1;
    // check rule [pos]
    if(!(__reserve.len<=CUBE_ROOM_POSITION_LEN+1)) {
        return(ERR_INVALID_DATA);
    }
    // write len [pos]
	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
    // write data [pos]
	__reserve.ret = stream_put(__reserve.stream, pos, sizeof(char)*(__reserve.len));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int room_walk_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	struct {
		char* nick;
		char* pos;
	} __reserve;
	int len, ret;

	// init
	memset(&__reserve, 0, sizeof(__reserve));
	len = 0;
	ret = 0;

	// read from stream
	// get [nick] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [nick]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.nick, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [nick]
	ret = stream_get(stream, __reserve.nick, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.nick[len-1] = '\0';
	// get [pos] len
	len = 0;
	ret = stream_get(stream, &len, sizeof(short));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// alloc memory [pos]
	ret = CLT_Alloc(user_ctx, stream, (void**)&__reserve.pos, sizeof(char)*(len+1));
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	// get data [pos]
	ret = stream_get(stream, __reserve.pos, sizeof(char)*len);
	if(ret!=ERR_NOERROR) goto ON_ERROR;
	__reserve.pos[len-1] = '\0';

	// call
	room_walk_callback(user_ctx, __reserve.nick, __reserve.pos);

	// free memory
	CLT_Free(user_ctx, __reserve.pos);
	CLT_Free(user_ctx, __reserve.nick);
	// end
	return ERR_NOERROR;

ON_ERROR:
	if(__reserve.pos!=NULL) CLT_Free(user_ctx, __reserve.pos);
	if(__reserve.nick!=NULL) CLT_Free(user_ctx, __reserve.nick);

	return ret;
}

int room_load_complete(CLT_USER_CTX* user_ctx)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = ROOM_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 16;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int room_set_ready(CLT_USER_CTX* user_ctx, int flag)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = ROOM_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 17;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill
    // write data [flag]
	__reserve.ret = stream_put(__reserve.stream, &flag, sizeof(int)*(1));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

int room_status_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	int len, ret;

	// init
	len = 0;
	ret = 0;

	// read from stream

	// call
	room_status_callback(user_ctx);

	// free memory
	// end
	return ERR_NOERROR;
}

int room_terminate(CLT_USER_CTX* user_ctx)
{
	// define
	struct {
		int filter_id, command_id;
		int len;
		STREAM* stream;
		int ret;
	} __reserve;

	// init
	__reserve.filter_id = 0;
	__reserve.command_id = 0;
	__reserve.ret = CLT_Newstream(user_ctx, &__reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;
	__reserve.filter_id = ROOM_FILTER_ID;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}
	__reserve.command_id = 19;
	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// fill

	// send
	__reserve.ret = CLT_Send(user_ctx, __reserve.stream);
	if(__reserve.ret!=ERR_NOERROR) {
		stream_destroy(__reserve.stream);
		return __reserve.ret;
	}

	// end
	return ERR_NOERROR;
}

