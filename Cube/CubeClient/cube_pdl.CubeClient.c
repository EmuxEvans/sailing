#include <stdlib.h>
#include <string.h>
#include "cube_pdl.CubeClient.h"

// Define command : BEGIN
int login_login_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int login_create_player_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int lobby_roomlist_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int lobby_chat_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int lobby_roleinfo_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int lobby_warehouse_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int lobby_equipment_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_join_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_leave_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_leave_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
int room_chat_callback_stub(CLT_USER_CTX* ctx, STREAM* stream);
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
	case 0x00040000|LOBBY_FILTER_ID:	return lobby_chat_callback_stub(ctx, stream);
	case 0x00070000|LOBBY_FILTER_ID:	return lobby_roleinfo_callback_stub(ctx, stream);
	case 0x000a0000|LOBBY_FILTER_ID:	return lobby_warehouse_callback_stub(ctx, stream);
	case 0x000d0000|LOBBY_FILTER_ID:	return lobby_equipment_callback_stub(ctx, stream);
	case 0x00010000|ROOM_FILTER_ID:	return room_join_callback_stub(ctx, stream);
	case 0x00020000|ROOM_FILTER_ID:	return room_leave_stub(ctx, stream);
	case 0x00030000|ROOM_FILTER_ID:	return room_leave_callback_stub(ctx, stream);
	case 0x00050000|ROOM_FILTER_ID:	return room_chat_callback_stub(ctx, stream);
	case 0x00070000|ROOM_FILTER_ID:	return room_walk_callback_stub(ctx, stream);
	case 0x000b0000|ROOM_FILTER_ID:	return room_status_callback_stub(ctx, stream);
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
	int len, ret;

	// init
	len = 0;
	ret = 0;

	// read from stream

	// call
	lobby_roomlist_callback(user_ctx);

	// free memory
	// end
	return ERR_NOERROR;
}

int lobby_chat(CLT_USER_CTX* user_ctx)
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
	__reserve.command_id = 3;
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

int lobby_chat_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	int len, ret;

	// init
	len = 0;
	ret = 0;

	// read from stream

	// call
	lobby_chat_callback(user_ctx);

	// free memory
	// end
	return ERR_NOERROR;
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
	__reserve.command_id = 5;
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
	__reserve.command_id = 6;
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
	__reserve.command_id = 8;
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
	__reserve.command_id = 9;
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
	__reserve.command_id = 11;
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
	__reserve.command_id = 12;
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

int room_join_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	int len, ret;

	// init
	len = 0;
	ret = 0;

	// read from stream

	// call
	room_join_callback(user_ctx);

	// free memory
	// end
	return ERR_NOERROR;
}

int room_leave_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	int len, ret;

	// init
	len = 0;
	ret = 0;

	// read from stream

	// call
	room_leave(user_ctx);

	// free memory
	// end
	return ERR_NOERROR;
}

int room_leave_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	int len, ret;

	// init
	len = 0;
	ret = 0;

	// read from stream

	// call
	room_leave_callback(user_ctx);

	// free memory
	// end
	return ERR_NOERROR;
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
	__reserve.command_id = 4;
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

int room_walk(CLT_USER_CTX* user_ctx)
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
	__reserve.command_id = 6;
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

int room_walk_callback_stub(CLT_USER_CTX* user_ctx, STREAM* stream)
{
	// define
	int len, ret;

	// init
	len = 0;
	ret = 0;

	// read from stream

	// call
	room_walk_callback(user_ctx);

	// free memory
	// end
	return ERR_NOERROR;
}

int room_set_singer(CLT_USER_CTX* user_ctx)
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
	__reserve.command_id = 8;
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
	__reserve.command_id = 9;
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

int room_set_ready(CLT_USER_CTX* user_ctx)
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

