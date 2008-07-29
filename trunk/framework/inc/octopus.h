#ifndef APPBOX_WITHOUT_OCTOPUS
#ifndef _OCTOPUS_INCLUDE_
#define _OCTOPUS_INCLUDE_

typedef struct OBJECT_ID {
	int index, seq;
	SOCK_ADDR address;
} OBJECT_ID;

#include "gameroom.h"
#include "fes_userctx.h"
#include "player.h"
/*
typedef void (*USER_EVENT)(USER_TUNNEL* tunnel, unsigned int code, unsigned char* data, int data_len);

USER_CONNECTION* user_connection_new();
void user_connection_free(USER_CONNECTION* conn);
void user_connection_disconnect(USER_CONNECTION* conn);

USER_TUNNEL* user_tunnel_connect(unsigned int type, unsigned char* data, int data_len);
user_tunnel_disconnect(USER_TUNNEL* tunnel, unsigned char* data, int data_len);
user_tunnel_notify(USER_TUNNEL* tunnel, unsigned int code, unsigned char* data, int data_len)

void user_tunnel_register(unsigned int type, USER_EVENT event);
void user_tunnel_unregister(unsigned int type, USER_EVENT event);
*/
#endif
#endif
