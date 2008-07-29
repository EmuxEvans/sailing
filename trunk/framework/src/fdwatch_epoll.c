#include <assert.h>
#include <string.h>
#include <sys/epoll.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/rlist.h"
#include "../inc/fdwatch.h"

// STATIC CONFIG : Start
#define FDWATCH_EPOLL_EVENT_SIZE	2000
// STATIC CONFIG : End

#define FDWATCH_REMOVELIST	(1<<10)

typedef struct FDWATCH {
	int				epollfd;
	int				quit_flag;
} FDWATCH_EPOLL;

static FDWATCH_EPOLL array[FDWATCH_MAX];

int fdwatch_init()
{
	int i;
	memset(array, 0, sizeof(array));
	for(i=0; i<sizeof(array)/sizeof(array[0]); i++) {
		array[i].epollfd = -1;
	}
	return ERR_NOERROR;
}

int fdwatch_final()
{
	return ERR_NOERROR;
}

int fdwatch_create(FDWATCH_HANDLE* ret)
{
	int i;

	for(i=0; i<sizeof(array)/sizeof(array[0]); i++) {
		if(array[i].epollfd==-1) break;
	}
	if(i==sizeof(array)/sizeof(array[0])) return ERR_NO_ENOUGH_MEMORY;

	array[i].epollfd = epoll_create(1000);
	if(array[i].epollfd==-1) {
		return ERR_UNKNOWN;
	}

	array[i].quit_flag = 0;
	*ret = &array[i];
	return ERR_NOERROR;
}

int fdwatch_destroy(FDWATCH_HANDLE handle)
{
	close(handle->epollfd);
	handle->epollfd = -1;
	return ERR_NOERROR;
}

int fdwatch_set(FDWATCH_ITEM* item, int fd, int flags, FDWATCH_EVENT event, void* ptr)
{
	assert(!(fd<=0 || !(flags&(FDWATCH_READ|FDWATCH_WRITE))));
	if(fd<=0 || !(flags&(FDWATCH_READ|FDWATCH_WRITE)))
		return ERR_INVALID_PARAMETER;

	rlist_set_userdata(&item->item, ptr);
	item->fd	= fd;
	item->flags	= flags&(FDWATCH_ONCE|FDWATCH_READ|FDWATCH_WRITE);
	item->event	= event;
	return ERR_NOERROR;
}

int fdwatch_add(FDWATCH_HANDLE handle, FDWATCH_ITEM* item)
{
	struct epoll_event epevt;

	epevt.events = 0;
	if(item->flags&FDWATCH_ONCE)	epevt.events |= EPOLLONESHOT;
	else							epevt.events |= EPOLLET;
	if(item->flags&FDWATCH_READ)	epevt.events |= EPOLLIN;
	if(item->flags&FDWATCH_WRITE)	epevt.events |= EPOLLOUT;
	epevt.data.ptr	= item;

	if(epoll_ctl(handle->epollfd, EPOLL_CTL_ADD, fdwatch_getfd(item), &epevt)==-1) {
//		SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_add(epoll) : epoll_ctl() fail, errno()=%s", errno);
		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

int fdwatch_remove(FDWATCH_HANDLE handle, FDWATCH_ITEM* item)
{
	if(epoll_ctl(handle->epollfd, EPOLL_CTL_MOD, fdwatch_getfd(item), NULL)==-1) {
//		SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_add(epoll) : epoll_ctl() fail, errno()=%s", errno);
		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

int fdwatch_rearm(FDWATCH_HANDLE handle, FDWATCH_ITEM* item)
{
	struct epoll_event epevt;

	assert(item->flags&FDWATCH_ONCE);
	if(!(item->flags&FDWATCH_ONCE)) return ERR_INVALID_PARAMETER;

	epevt.events = EPOLLONESHOT;
	if(item->flags&FDWATCH_READ)  epevt.events |= EPOLLIN;
	if(item->flags&FDWATCH_WRITE) epevt.events |= EPOLLOUT;
	epevt.data.ptr  = item;

	if(epoll_ctl(handle->epollfd, EPOLL_CTL_MOD, fdwatch_getfd(item), &epevt)==-1) {
//		SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_rearm(epoll) : epoll_ctl() fail, errno()=%d", errno);
		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

int fdwatch_loop(FDWATCH_HANDLE handle)
{
	while(!handle->quit_flag) {
		fdwatch_dispatch(handle, 100); // timeout = 100ms
	}
	return ERR_NOERROR;
}

int fdwatch_dispatch(FDWATCH_HANDLE handle, int timeout)
{
	int ret;
	struct epoll_event epevts[FDWATCH_EPOLL_EVENT_SIZE];

	for(;;) {
		ret = epoll_wait(handle->epollfd, epevts, sizeof(epevts)/sizeof(epevts[0]), timeout);
		if(ret>=0) break;
		if(errno!=EINTR) {
//			SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_wait(epoll) : epoll_wait fail, errno()=%d", errno);
			return ERR_UNKNOWN;
		}
	}

	for(; ret>0; ret--) {
		int events = 0;
		if(epevts[ret-1].events&EPOLLIN)	events |= FDWATCH_READ;
		if(epevts[ret-1].events&EPOLLOUT)	events |= FDWATCH_WRITE;
		((FDWATCH_ITEM*)epevts[ret-1].data.ptr)->event((FDWATCH_ITEM*)epevts[ret-1].data.ptr, events);
	}

	return ERR_NOERROR;
}

int ZION_API fdwatch_break(FDWATCH_HANDLE handle)
{
	handle->quit_flag = 1;
	return ERR_NOERROR;
}
