#ifndef _GAMEROOM_INCLUDE_
#define _GAMEROOM_INCLUDE_

#define GAMEROOM_STATE_MAX			200		// 扩展模块ID的最大值
#define GAMEROOM_STATE_COUNT		6		// 扩展模块的最大个数
#define GAMEROOM_MAX_COUNT			9000	// 房间实例的最大个数
#define GAMEROOM_TIMER_COUNT		10		// 一个房间内定时器的最大个数
#define GAMEROOM_TIMER_INTERVAL		50		// 全局定时器心跳间隔
#define GAMEROOM_STRID_LEN			50		// 房间ID String的最大长度

// state flag's define
#define GAMEROOM_AUTO_ATTACH		(1<<0)	// 当Member加入房间时自动 Attach 到 State
#define GAMEROOM_AUTO_DETACH		(1<<1)	// 当房间里没有Member的时候自动释放
#define GAMEROOM_NEVER_ENABLE		(1<<2)	// State 不可被激活
// room  flag's define
#define GAMEROOM_AUTO_FREE			(1<<1)	// 当房间里没有Member的时候自动释放
#define GAMEROOM_DELETE				(1<<0)	// 房间被删除了

#define GAMEROOM_MATCH				(1<<0)	// 匹配
#define GAMEROOM_NOTMATCH			(1<<1)	// 不匹配
#define GAMEROOM_SELF				(1<<2)	// 包含自己
#define GAMEROOM_EXCEPT				(1<<3)	// 除了自己

#define GAMEROOM_NOTIFY_SHUTDOWN	(0)
#define GAMEROOM_NOTIFY_INIT		(1)
#define GAMEROOM_NOTIFY_DESTROY		(2)
#define GAMEROOM_NOTIFY_ATTACH		(3)
#define GAMEROOM_NOTIFY_DETACH		(4)
#define GAMEROOM_NOTIFY_CONNECT		(5)
#define GAMEROOM_NOTIFY_DISCONNECT	(6)

#define GAMEROOM_CLASS_BEGIN(classname)				\
GAMEROOM_CLASS classname = {
#define	GAMEROOM_CLASS_CALLBACK(rpcsend, rpcleave)	\
	rpcsend, rpcleave,
#define GAMEROOM_CLASS_OPTION(flags, member_max)	\
	flags, member_max,
#define GAMEROOM_CLASS_STATE_BEGIN(count)			\
	count,											\
	{
#define GAMEROOM_CLASS_STATE(id, flags, ssize, msize, ontimer, onnotify)	\
		{ id, flags, ssize, msize, ontimer, onnotify, 0, 0, 0 },
#define GAMEROOM_CLASS_STATE_END()					\
	},												\
	{ NULL, NULL, NULL, NULL, NULL, NULL},
#define GAMEROOM_CLASS_END(classname)				\
	NULL, NULL, 0, -1, -1	\
};

/*
GAMEROOM_CLASS_BEGIN(demoroom)
	GAMEROOM_CLASS_CALLBACK(rpcsend, rpcleave)
	GAMEROOM_CLASS_OPTION(GAMEROOM_AUTO_FREE, 100)
	GAMEROOM_CLASS_STATE_BEGIN()
		GAMEROOM_CLASS_STATE(0, GAMEROOM_AUTO_ATTACH|GAMEROOM_AUTO_DETACH, 0, 0, onshutdown, ontimer, onmembernotify)
		GAMEROOM_CLASS_STATE_NULL(id)
		GAMEROOM_CLASS_STATE_NULL(id)
		GAMEROOM_CLASS_STATE_NULL(id)
		GAMEROOM_CLASS_STATE_NULL(id)
		GAMEROOM_CLASS_STATE_NULL(id)
	GAMEROOM_CLASS_STATE_END()
GAMEROOM_CLASS_END(classname)
*/

struct GAMEROOM;
typedef struct GAMEROOM GAMEROOM;

struct GAMEROOM_MEMBER;
typedef struct GAMEROOM_MEMBER GAMEROOM_MEMBER;

typedef struct GAMEROOM_STATE {
	unsigned int id;
	unsigned int flags;
	unsigned int state_size, member_size;

	void (*OnTimer)(GAMEROOM* room, unsigned int state_id, int timer_id);
	int (*OnNotify)(GAMEROOM* room, int code, unsigned int state_id, GAMEROOM_MEMBER* memb);

	unsigned int index;
	unsigned int state_offset;
	unsigned int member_offset;
} GAMEROOM_STATE;

typedef struct GAMEROOM_CLASS {
	void (*RpcSend)(GAMEROOM* room, GAMEROOM_MEMBER* memb, void* data, int data_len);
	void (*RpcLeave)(GAMEROOM* room, GAMEROOM_MEMBER* member);

	unsigned int flags, member_max;

	unsigned int	state_count;
	GAMEROOM_STATE	states[GAMEROOM_STATE_COUNT];
	GAMEROOM_STATE*	state_map[GAMEROOM_STATE_MAX];

	MEMPOOL_HANDLE	room_pool;
	MEMPOOL_HANDLE	memb_pool;
	int room_count;
	int room_size, memb_size;
} GAMEROOM_CLASS;

//
void gameroom_init();
void gameroom_final();

// class
int gameroom_class_init(GAMEROOM_CLASS* gc);
int gameroom_class_destroy(GAMEROOM_CLASS* gc);
int gameroom_class_get_count(const GAMEROOM_CLASS* gc);
int gameroom_class_shutdown(GAMEROOM_CLASS* gc);

//
GAMEROOM* gameroom_create(GAMEROOM_CLASS* gc, const char* strid, void* fud);
void gameroom_destroy(GAMEROOM* room);

const char* gameroom_get_strid(GAMEROOM* room);
void* gameroom_get_fud(GAMEROOM* room);
void gameroom_set_fud(GAMEROOM* room, void* fud);

// object
const GAMEROOM_CLASS* gameroom_get_class(GAMEROOM* room);
int gameroom_find(const char* strid, OBJECT_ID* objid);
const OBJECT_ID* gameroom_objid(GAMEROOM* room, OBJECT_ID* objid);
GAMEROOM* gameroom_lock(const OBJECT_ID* obj_id);
int gameroom_unlock(GAMEROOM* room);

// time
int gameroom_timer_reset(GAMEROOM* room, unsigned int id, unsigned int state_id);
int gameroom_timer_set(GAMEROOM* room, unsigned int id, unsigned int state_id, unsigned int duetime, unsigned int period);

//
void* gameroom_get_statedata(GAMEROOM* room, unsigned int state_id);
void* gameroom_get_membdata(GAMEROOM* room, unsigned int state_id, GAMEROOM_MEMBER* memb);

//
GAMEROOM_MEMBER* gameroom_member_create(GAMEROOM* room, const char* strid, const OBJECT_ID* oid);
int gameroom_member_release(GAMEROOM* room, GAMEROOM_MEMBER* member);
int gameroom_member_delete(GAMEROOM* room, GAMEROOM_MEMBER* member);

const char* gameroom_member_get_strid(GAMEROOM_MEMBER* member);
int gameroom_member_set_strid(GAMEROOM* room, GAMEROOM_MEMBER* member, const char* strid);
GAMEROOM_MEMBER* gameroom_member_getbystrid(GAMEROOM* room, const char* strid);

int gameroom_member_connect(GAMEROOM* room, GAMEROOM_MEMBER* member, const OBJECT_ID* oid);
int gameroom_member_disconnect(GAMEROOM* room, GAMEROOM_MEMBER* member);
int gameroom_member_isonline(GAMEROOM_MEMBER* member);

void gameroom_notify(GAMEROOM* room, int code, GAMEROOM_MEMBER* memb);
void gameroom_notify_state(GAMEROOM* room, int code, unsigned int state_id, GAMEROOM_MEMBER* memb);

unsigned int gameroom_member_getseq(GAMEROOM* room, GAMEROOM_MEMBER* memb);
int gameroom_member_getindex(GAMEROOM* room, GAMEROOM_MEMBER* memb);
GAMEROOM_MEMBER* gameroom_member_getbyseq(GAMEROOM* room, unsigned int seq);
GAMEROOM_MEMBER* gameroom_member_getbyindex(GAMEROOM* room, unsigned int index);

void gameroom_member_attach(GAMEROOM* room, unsigned int state_id, GAMEROOM_MEMBER* memb);
void gameroom_member_detach(GAMEROOM* room, unsigned int state_id, GAMEROOM_MEMBER* memb);

const OBJECT_ID* gameroom_member_get_objid(GAMEROOM_MEMBER* memb, OBJECT_ID* objid);
RPCNET_GROUP* gameroom_member_get_group(GAMEROOM_MEMBER* memb);

void gameroom_member_set_mask(GAMEROOM* room, GAMEROOM_MEMBER* memb, unsigned int mask);
unsigned int gameroom_member_get_mask(GAMEROOM* room, GAMEROOM_MEMBER* memb);

// send data
int gameroom_sendex(GAMEROOM* room, GAMEROOM_MEMBER* memb, unsigned int mask, unsigned int value, unsigned int flags, unsigned char* data, unsigned int len);

ZION_INLINE int gameroom_send_self(GAMEROOM* room, GAMEROOM_MEMBER* memb, unsigned char* data, unsigned int len)
{
	return gameroom_sendex(room, memb, 0, 0, GAMEROOM_SELF, data, len);
}

ZION_INLINE int gameroom_send_except(GAMEROOM* room, GAMEROOM_MEMBER* memb, unsigned char* data, unsigned int len)
{
	return gameroom_sendex(room, memb, 0, 0, GAMEROOM_MATCH|GAMEROOM_NOTMATCH|GAMEROOM_EXCEPT, data, len);
}

ZION_INLINE int gameroom_send_match(GAMEROOM* room, unsigned int mask, unsigned int value, unsigned char* data, unsigned int len)
{
	return gameroom_sendex(room, NULL, mask, value, GAMEROOM_MATCH, data, len);
}

ZION_INLINE int gameroom_send_notmatch(GAMEROOM* room, unsigned int mask, unsigned int value, unsigned char* data, unsigned int len)
{
	return gameroom_sendex(room, NULL, mask, value, GAMEROOM_NOTMATCH, data, len);
}

ZION_INLINE int gameroom_send_all(GAMEROOM* room, unsigned char* data, unsigned int len)
{
	return gameroom_sendex(room, NULL, 0, 0, GAMEROOM_MATCH|GAMEROOM_NOTMATCH, data, len);
}

#endif
