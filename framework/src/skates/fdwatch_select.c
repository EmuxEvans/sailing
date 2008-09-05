#include <assert.h>
#include <string.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/fdwatch.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#define WSAGetLastError()	errno
#define WSAEINTR			EINTR
#else
#include <winsock2.h>
#define FD_SETSIZE 64
#endif

#define FDWATCH_DISABLE				(1<<10)

typedef struct FDWATCH {
	int					inuse;
	RLIST_HEAD			enable_list;
	os_mutex_t			list_mtx;
	int					quit_flag;
} FDWATCH_SELECT;

FDWATCH_SELECT array[FDWATCH_MAX];

int fdwatch_init()
{
	memset(array, 0, sizeof(array));
	return ERR_NOERROR;
}

int fdwatch_final()
{
	return ERR_NOERROR;
}

FDWATCH_HANDLE fdwatch_create()
{
	int index;

	for(index=0; index<sizeof(array)/sizeof(array[0]); index++) {
		if(!array[index].inuse) break;
	}
	if(index==sizeof(array)/sizeof(array[0])) return NULL;

	array[index].inuse = 1;
	rlist_init(&array[index].enable_list);
	os_mutex_init(&array[index].list_mtx);
	array[index].quit_flag = 0;
	return &array[index];
}

int fdwatch_destroy(FDWATCH_HANDLE handle)
{
	assert(handle && handle->inuse);
	if(handle==NULL || (!handle->inuse)) return ERR_INVALID_HANDLE;
	os_mutex_destroy(&handle->list_mtx);
	handle->inuse = 0;
	return ERR_NOERROR;
}

int fdwatch_set(FDWATCH_ITEM* item, int fd, int flags, FDWATCH_EVENT event, void* ptr)
{
	assert(!(fd<=0 || !(flags&(FDWATCH_READ|FDWATCH_WRITE))));
	if(fd<=0 || !(flags&(FDWATCH_READ|FDWATCH_WRITE))) return ERR_INVALID_PARAMETER;

	rlist_set_userdata(&item->item, ptr);
	item->fd	= fd;
	item->flags	= flags&(FDWATCH_ONCE|FDWATCH_READ|FDWATCH_WRITE);
	item->event	= event;
	return ERR_NOERROR;
}

int fdwatch_add(FDWATCH_HANDLE handle, FDWATCH_ITEM* item)
{
	assert(handle && handle->inuse);
	if(handle==NULL || (!handle->inuse)) return ERR_INVALID_HANDLE;

	os_mutex_lock(&handle->list_mtx);
	rlist_push_back(&handle->enable_list, &item->item);
	os_mutex_unlock(&handle->list_mtx);

	return ERR_NOERROR;
}

int fdwatch_remove(FDWATCH_HANDLE handle, FDWATCH_ITEM* item)
{
	assert(handle && handle->inuse);
	if(handle==NULL || (!handle->inuse)) return ERR_INVALID_HANDLE;

	os_mutex_lock(&handle->list_mtx);
	rlist_remove(&handle->enable_list, &item->item);
	os_mutex_unlock(&handle->list_mtx);

	return ERR_NOERROR;
}

int fdwatch_rearm(FDWATCH_HANDLE handle, FDWATCH_ITEM* item)
{
	assert(handle && handle->inuse);
	if(handle==NULL || (!handle->inuse)) return ERR_INVALID_HANDLE;

	assert(item->flags&FDWATCH_DISABLE);
	if(!(item->flags&FDWATCH_DISABLE)) return ERR_INVALID_PARAMETER;

//	os_mutex_lock(&handle->list_mtx);
	item->flags &= ~FDWATCH_DISABLE;
//	os_mutex_lock(&handle->list_mtx);

	return ERR_NOERROR;
}

int fdwatch_loop(FDWATCH_HANDLE handle)
{
	assert(handle && handle->inuse);
	if(handle==NULL || (!handle->inuse)) return ERR_INVALID_HANDLE;

	while(!handle->quit_flag) {
		fdwatch_dispatch(handle, 100); // timeout = 100ms
	}

	return ERR_NOERROR;
}

int fdwatch_dispatch(FDWATCH_HANDLE handle, int timeout)
{
	FDWATCH_ITEM* item;
	FDWATCH_ITEM* pool[FD_SETSIZE];
	int count, ret, fdmax;
	fd_set rdfds, wrfds;

	assert(handle && handle->inuse);
	if(handle==NULL || (!handle->inuse)) return ERR_INVALID_HANDLE;

	//
	FD_ZERO(&rdfds);
	FD_ZERO(&wrfds);
	fdmax = 0;
	count = 0;

	while(1) {
		os_sleep(10);

		os_mutex_lock(&handle->list_mtx);
		item = (FDWATCH_ITEM*)rlist_front(&handle->enable_list);
		while(!rlist_is_head(&handle->enable_list, &item->item) && count<FD_SETSIZE) {
			if((item->flags&FDWATCH_DISABLE)==0) {
				pool[count++] = item;
				if(item->flags&FDWATCH_READ)	FD_SET(item->fd, &rdfds);
				if(item->flags&FDWATCH_WRITE)	FD_SET(item->fd, &wrfds);
				if(fdmax<item->fd) fdmax = item->fd;
			}
 			item = (FDWATCH_ITEM*)rlist_next(&item->item);
		}
		os_mutex_unlock(&handle->list_mtx);

		if(count>0) break;
		if(timeout!=FDWATCH_INFINITE) {
			os_sleep(timeout);
			return ERR_NOERROR;
		}
		os_sleep(50);
	}

	for(;;) {
		if(timeout!=FDWATCH_INFINITE) {
			struct timeval tv;
			tv.tv_sec	= timeout / 1000;
			tv.tv_usec	= (timeout%1000)*1000;
			ret = select(fdmax+1, &rdfds, &wrfds, NULL, &tv);
		} else {
			ret = select(fdmax+1, &rdfds, &wrfds, NULL, NULL);
		}
		if(ret>=0) break;
		if(WSAGetLastError()!=WSAEINTR) {
//			SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_loop(SELECT): Failed to select(), errno=%d", errno);
			return ERR_UNKNOWN;
		}
	}

	for(;count>0; count--) {
		int events = 0;
		if(pool[count-1]->flags&FDWATCH_READ && FD_ISSET(pool[count-1]->fd, &rdfds)) {
			events |= FDWATCH_READ;
		}
		if(pool[count-1]->flags&FDWATCH_WRITE && FD_ISSET(pool[count-1]->fd, &wrfds)) {
			events |= FDWATCH_WRITE;
		}
		if(events!=0) {
			if(pool[count-1]->flags&FDWATCH_ONCE) pool[count-1]->flags |= FDWATCH_DISABLE;
			pool[count-1]->event(pool[count-1], events);
		}
	}

	return ERR_NOERROR;
}

int fdwatch_break(FDWATCH_HANDLE handle)
{
	assert(handle && handle->inuse);
	if(handle==NULL || (!handle->inuse)) return ERR_INVALID_HANDLE;

	handle->quit_flag = 1;
	return ERR_NOERROR;
}

