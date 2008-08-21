
#ifndef _FDWATCH_INCLUDE_
#define _FDWATCH_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#define FDWATCH_MAX					20

#define FDWATCH_ONCE				(1<<0)
#define FDWATCH_READ				(1<<1)
#define FDWATCH_WRITE				(1<<2)

#define FDWATCH_INVALID_HANDLE		(-1)
#define FDWATCH_INFINITE			(-1)

struct FDWATCH_ITEM;
typedef struct FDWATCH_ITEM FDWATCH_ITEM;

typedef void(*FDWATCH_EVENT)(FDWATCH_ITEM* item, int events);

struct FDWATCH_ITEM {
	RLIST_ITEM		item;
	int				fd;
	int				flags;
	FDWATCH_EVENT	event;
};

struct FDWATCH;
typedef struct FDWATCH*	FDWATCH_HANDLE;

ZION_API int fdwatch_init();
ZION_API int fdwatch_final();

ZION_API FDWATCH_HANDLE fdwatch_create();
ZION_API int fdwatch_destroy(FDWATCH_HANDLE handle);

ZION_API int fdwatch_set(FDWATCH_ITEM* item, int fd, int flags, FDWATCH_EVENT event, void* ptr);
ZION_API int fdwatch_add(FDWATCH_HANDLE handle, FDWATCH_ITEM* item);
ZION_API int fdwatch_remove(FDWATCH_HANDLE handle, FDWATCH_ITEM* item);
ZION_API int fdwatch_rearm(FDWATCH_HANDLE handle, FDWATCH_ITEM* item);

ZION_API int fdwatch_dispatch(FDWATCH_HANDLE handle, int timeout);
ZION_API int fdwatch_loop(FDWATCH_HANDLE handle);
ZION_API int fdwatch_break(FDWATCH_HANDLE handle);

#define fdwatch_getfd(item)			((item)->fd)
#define fdwatch_getptr(item)		rlist_get_userdata(&((item)->item))
#define fdwatch_setptr(item, ptr)	rlist_set_userdata(&((item)->item), ptr)

#ifdef __cplusplus
}
#endif

#endif

