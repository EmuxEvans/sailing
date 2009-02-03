#include <skates/skates.h>

#include "streamserver.h"

struct STREAM_ROOM;
typedef struct STREAM_ROOM STREAM_ROOM;
struct STEAM_USER;
typedef struct STEAM_USER STREAM_USER;

struct STREAM_ROOM {
	STREAM_USER*	userlist[100];
	int				usercount;
	char			name[40];
};

struct STEAM_USER {
	int				state;		// 连接的状态
	int				delay;		// 网络延时

	// for room
	STREAM_ROOM*	room;		// 房间的指针
	int				rindex;		//

	int				enable;		// 是否可接受数据
	int				nosend;		// 未收到响应的字节数
	int				nosend_max;	// 最大未收到响应字节数
};

#define STREAM_USERSTATE_OPEN		0	// 连接刚建立
#define STREAM_USERSTATE_AUTH		1	// 认证完成
#define STREAM_USERSTATE_PING		2	// 网络延时计算完毕
#define STREAM_USERSTATE_ROOM		3	// 已经在房间里了

static void onaccept(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname);
static void onconnect(NETWORK_HANDLE handle, void* userptr);
static void ondata(NETWORK_HANDLE handle, void* userptr);
static void ondisconnect(NETWORK_HANDLE handle, void* userptr);

static STREAM_ROOM* alloc_room(const char* name);
static STREAM_ROOM* find_room(const char* name);
static void free_room(const char* name);

void streamserver_init()
{
}

void streamserver_final()
{
}

int streamserver_start(SOCK_ADDR* sa)
{
	return ERR_NOERROR;
}

int streamserver_stop(const SOCK_ADDR* sa)
{
	return ERR_NOERROR;
}

void onaccept(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname)
{
}

void onconnect(NETWORK_HANDLE handle, void* userptr)
{
}

void ondata(NETWORK_HANDLE handle, void* userptr)
{
}

void ondisconnect(NETWORK_HANDLE handle, void* userptr)
{
}
