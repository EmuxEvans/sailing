
#ifndef __CUBE_SERVER_INCLUDE__
#define __CUBE_SERVER_INCLUDE__

#ifdef __cplusplus
extern "C" {
#endif

struct CUBE_CONNECTION;
typedef struct CUBE_CONNECTION CUBE_CONNECTION;
struct CUBE_ROOM;
typedef struct CUBE_ROOM CUBE_ROOM;

struct CUBE_CONNECTION {
	NETWORK_HANDLE handle;
	int uid;
	char nick[CUBE_NICK_LEN+1];

	char ri[CUBE_ROLEINFO_LEN+1];
	char wh[CUBE_WAREHOUSE_LEN+1];
	char equ[CUBE_EQUIPMENT_LEN+1];

	CUBE_ROOM* room;

	char recv_buf[10*1024];
};


struct CUBE_ROOM {
	char name[CUBE_ROOM_NAME_LEN+1];
	int state;
	CUBE_CONNECTION* conns[CUBE_ROOM_MEMBER_MAX];
	int singer;
};

struct SVR_USER_CTX {
	CUBE_CONNECTION*	conn;
	MEM_STREAM			stream;
	char				buf[10*1024];
};

extern SOCK_ADDR cube_sa;
extern char cube_dbstr[100];
extern CUBE_CONNECTION* conn_list[1000];

#ifdef __cplusplus
}
#endif

#endif
