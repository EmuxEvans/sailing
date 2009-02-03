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
	int				state;		// ���ӵ�״̬
	int				delay;		// ������ʱ

	// for room
	STREAM_ROOM*	room;		// �����ָ��
	int				rindex;		//

	int				enable;		// �Ƿ�ɽ�������
	int				nosend;		// δ�յ���Ӧ���ֽ���
	int				nosend_max;	// ���δ�յ���Ӧ�ֽ���
};

#define STREAM_USERSTATE_OPEN		0	// ���Ӹս���
#define STREAM_USERSTATE_AUTH		1	// ��֤���
#define STREAM_USERSTATE_PING		2	// ������ʱ�������
#define STREAM_USERSTATE_ROOM		3	// �Ѿ��ڷ�������

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
