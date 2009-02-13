#include <assert.h>

#include <skates/skates.h>

#include "streamserver.h"

struct STREAM_ROOM;
typedef struct STREAM_ROOM STREAM_ROOM;
struct STEAM_USER;
typedef struct STEAM_USER STREAM_USER;

struct STREAM_ROOM {
	HASHMAP_ITEM	item;

	char			name[40];
	RLIST_HEAD		userlist;
};

struct STEAM_USER {
	RLIST_ITEM		item;
	STREAM_ROOM*	room;		// 房间的指针

	NETWORK_HANDLE	handle;
	char			recvbuf[100*1024];

	int				state;		// 连接的状态
	int				delay;		// 网络延时

	// for room
	int				enable;		// 是否可接受数据
	int				nosend;		// 未收到响应的字节数
	int				nosend_max;	// 最大未收到响应字节数
};

#define STREAM_USERSTATE_OPEN		0	// 连接刚建立
#define STREAM_USERSTATE_AUTH		1	// 认证完成
#define STREAM_USERSTATE_PING		2	// 网络延时计算完毕
#define STREAM_USERSTATE_ROOM		3	// 已经在房间里了
#define STREAM_USERSTATE_DISCONNECT	4	// 断开连接

static void onaccept(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname);
static void onconnect(NETWORK_HANDLE handle, void* userptr);
static void ondata(NETWORK_HANDLE handle, void* userptr);
static void ondisconnect(NETWORK_HANDLE handle, void* userptr);

static STREAM_ROOM* alloc_room(const char* name);
static STREAM_ROOM* find_room(const char* name);
static void free_room(STREAM_ROOM* room);

static MEMPOOL_HANDLE	user_pool;
static MEMPOOL_HANDLE	room_pool;
static HASHMAP_HANDLE	room_map;

void streamserver_init()
{
	user_pool = mempool_create("STREAMSERVER_USER", sizeof(STREAM_USER), 0);
	room_pool = mempool_create("STREAMSERVER_ROOM", sizeof(STREAM_ROOM), 0);
	room_map  = hashmap_create(12, NULL, 0);
}

void streamserver_final()
{
	mempool_destroy(user_pool);
	mempool_destroy(room_pool);
	hashmap_destroy(room_map);
}

int streamserver_start(SOCK_ADDR* sa)
{
	return network_tcp_register(sa, onaccept, NULL);
}

int streamserver_stop(const SOCK_ADDR* sa)
{
	int ret;
	ret = network_tcp_unregister(sa);
	return ERR_NOERROR;
}

void onaccept(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname)
{
	STREAM_USER* user;
	NETWORK_EVENT event;

	user = (STREAM_USER*)mempool_alloc(user_pool);
	if(!user) {
		sock_disconnect(sock);
		return;
	}

	event.OnConnect = onconnect;
	event.OnData = ondata;
	event.OnDisconnect = ondisconnect;
	event.recvbuf_buf = user->recvbuf;
	event.recvbuf_max = sizeof(user->recvbuf);
	event.recvbuf_pool = NULL;

	user->state = STREAM_USERSTATE_OPEN;
	user->handle = network_add(sock, &event, user);
	user->room = NULL;
}

void onconnect(NETWORK_HANDLE handle, void* userptr)
{
}

void ondata(NETWORK_HANDLE handle, void* userptr)
{
	STREAM_USER* user = (STREAM_USER*)userptr;

	for(;;) {
		unsigned short len;
		char pkg_buf[0x8000];
		const void* buf;

		if(network_recvbuf_len(handle)<sizeof(len)) break;
		network_recvbuf_get(handle, &len, 0, sizeof(len));

		if(len&0x8000) {
			buf = NULL;
		} else {
			buf = network_recvbuf_ptr(handle, sizeof(len), len&0x7fff);
			if(!buf) {
				if(network_recvbuf_get(handle, pkg_buf, sizeof(len), len&0x7fff)!=ERR_NOERROR) {
					break;
				}
				buf = pkg_buf;
			}
		}

		switch(user->state) {
		case STREAM_USERSTATE_OPEN:
			if((len&0x8000)!=0 || len==0 || len>100 || *((char*)buf+len-1)!='\0') {
				network_disconnect(handle);
				user->state = STREAM_USERSTATE_DISCONNECT;
				return;
			}
			user->room = find_room((const char*)buf);
			if(!user->room) {
				user->room = alloc_room((const char*)buf);
				if(!user->room) {
					network_disconnect(handle);
					user->state = STREAM_USERSTATE_DISCONNECT;
					return;
				}
			}
			rlist_clear(&user->item, NULL);
			rlist_push_back(&user->room->userlist, &user->item);
			user->enable = 1;
			user->delay = 0;
			user->nosend = 0;
			user->nosend_max = 100*1024;
			user->state = STREAM_USERSTATE_ROOM;
			break;
		case STREAM_USERSTATE_ROOM:
			if(len&0x8000) {
				if(len==0x8000) {
					user->enable = 1;
					user->nosend = 0;
				} else {
					assert(user->nosend>=(len&0x7fff));
					user->nosend -= len&0x7fff;
				}
			} else {
				RLIST_ITEM* item;
				for(item=rlist_front(&user->room->userlist); !rlist_is_head(&user->room->userlist, item); item=rlist_next(item)) {
					NETWORK_DOWNBUF* bufs[100];
					unsigned int bufs_count;

//					if((STREAM_USER*)item==user) continue;
					if(!((STREAM_USER*)item)->enable) continue;

					((STREAM_USER*)item)->nosend += sizeof(len) + len;
					if(((STREAM_USER*)item)->nosend > ((STREAM_USER*)item)->nosend_max) {
						unsigned code;

						((STREAM_USER*)item)->enable = 0;
						bufs_count = network_downbufs_alloc(bufs, sizeof(bufs)/sizeof(bufs[0]), sizeof(code));
						if(bufs_count==0) continue;
						code = 0x8000;
						network_downbufs_fill(bufs, bufs_count, 0, &code, sizeof(code));
						if(network_send(((STREAM_USER*)item)->handle, bufs, bufs_count)!=ERR_NOERROR) {
							network_downbufs_free(bufs, bufs_count);
						}
					} else {
						bufs_count = network_downbufs_alloc(bufs, sizeof(bufs)/sizeof(bufs[0]), sizeof(len) + len);
						if(bufs_count==0) continue;
						network_downbufs_fill(bufs, bufs_count, 0, &len, sizeof(len));
						network_downbufs_fill(bufs, bufs_count, sizeof(len), buf, len);

						if(network_send(((STREAM_USER*)item)->handle, bufs, bufs_count)!=ERR_NOERROR) {
							network_downbufs_free(bufs, bufs_count);
						}
					}
				}
			}
			break;
		default:
			user->state = STREAM_USERSTATE_DISCONNECT;
			network_disconnect(handle);
			return;
		}

		if(len&0x8000) {
			network_recvbuf_commit(handle, sizeof(len));
		} else {
			network_recvbuf_commit(handle, sizeof(len)+len);
		}
	}

}

void ondisconnect(NETWORK_HANDLE handle, void* userptr)
{
	STREAM_USER* user = (STREAM_USER*)userptr;
	if(user->room) {
		rlist_remove(&user->room->userlist, &user->item);
		if(rlist_empty(&user->room->userlist)) {
			free_room(user->room);
		}
	}
	mempool_free(user_pool, user);
}

STREAM_ROOM* alloc_room(const char* name)
{
	STREAM_ROOM* room;
	HASHMAP_ITEM* item;
	room = find_room(name);
	if(room) return room;
	room = mempool_alloc(room_pool);
	if(!room) return NULL;
	strcpy(room->name, name);
	rlist_init(&room->userlist);
	item = hashmap_add(room_map, &room->item, room->name, strlen(room->name));
	assert(item);
	return room;
}

STREAM_ROOM* find_room(const char* name)
{
	STREAM_ROOM* room;
	room = (STREAM_ROOM*)hashmap_get(room_map, name, strlen(name));
	return room;
}

void free_room(STREAM_ROOM* room)
{
	hashmap_erase(room_map, &room->item);
	mempool_free(room_pool, room);
}
