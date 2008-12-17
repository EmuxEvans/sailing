
#ifndef __CUBE_SERVER_INCLUDE__
#define __CUBE_SERVER_INCLUDE__

#ifdef __cplusplus
extern "C" {
#endif

#define CUBE_ROOM_TIMER		100		// ms
#define CUBE_ROOM_TIMEOUT	40000	// ms

struct CUBE_CONNECTION;
typedef struct CUBE_CONNECTION CUBE_CONNECTION;
struct CUBE_ROOM;
typedef struct CUBE_ROOM CUBE_ROOM;

struct CUBE_CONNECTION {
	int index;
	NETWORK_HANDLE handle;
	int uuid;
	char nick[CUBE_NICK_LEN+1];

	char ri[CUBE_ROLEINFO_LEN+1];
	char wh[CUBE_WAREHOUSE_LEN+1];
	char equ[CUBE_EQUIPMENT_LEN+1];

	CUBE_ROOM* room;
	int room_idx;

	char recv_buf[10*1024];
};

struct CUBE_ROOM {
	char owner[CUBE_NICK_LEN+1];
	char name[CUBE_ROOM_NAME_LEN+1];

	int state;
	unsigned int start_time;
	char singer[CUBE_NICK_LEN+1];

	struct {
		CUBE_CONNECTION* conn;
		int ready;
		int loaded;
		int terminated;
		unsigned int p2p_status;
		char pos[200];
	} members[CUBE_ROOM_MEMBER_MAX];

	struct {
		char nick[CUBE_NICK_LEN+1];
		char song[CUBE_ROOM_SONG_LEN+1];
	} microphone[CUBE_ROOM_MEMBER_MAX];
};

struct SVR_USER_CTX {
	CUBE_CONNECTION*	conn;
	MEM_STREAM			stream;
	char				buf[10*1024];
};

extern SOCK_ADDR cube_sa;
extern char cube_dbstr[100];
extern CUBE_CONNECTION* conn_list[1000];
extern CUBE_ROOM* room_list[1000];

CUBE_ROOM* cube_room_create(CUBE_CONNECTION* conn, const char* name, const char* owner);
void cube_room_leave(CUBE_ROOM* room, CUBE_CONNECTION* conn, int reason);
void cube_room_check(CUBE_ROOM* room);
int cube_room_member_count(CUBE_ROOM* room);
int cube_room_member_index(CUBE_ROOM* room, const char* nick);
int cube_room_get_singer(CUBE_ROOM* room);
int cube_room_joinable(CUBE_ROOM* room, CUBE_CONNECTION* conn);

void cube_room_tick(CUBE_ROOM* room);
unsigned int cube_room_curtime(CUBE_ROOM* room);
unsigned int cube_room_p2pmask(CUBE_ROOM* room, int midx);

void cube_room_onjoin(CUBE_ROOM* room, CUBE_CONNECTION* conn);
void cube_room_terminate(CUBE_ROOM* room);

void cube_room_sync(CUBE_ROOM* room, CUBE_CONNECTION* conn);
void cube_room_acquire(CUBE_ROOM* room, const char* nick, const char* song);
void cube_room_giveup(CUBE_ROOM* room, const char* nick);

int cube_can_change_equipment(CUBE_CONNECTION* conn);

#ifdef __cplusplus
}
#endif

#endif
