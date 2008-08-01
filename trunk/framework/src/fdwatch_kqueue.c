#include <string.h> 
#include <assert.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/rlist.h"
#include "../inc/fdwatch.h"

typedef struct FDWATCH {
	int kq;
	int quit_flag;
} FDWATCH_KQUEUE;

FDWATCH_KQUEUE array[FDWATCH_MAX];

int fdwatch_init()
{
	int i;
	memset(array, 0, sizeof(array));
	for(i=0; i<sizeof(array)/sizeof(array[0]); i++) {
		array[i].kq = -1;
	}
	return ERR_NOERROR;
}

int fdwatch_final()
{
	return ERR_NOERROR;
}

FDWATCH_HANDLE fdwatch_create()
{
	int i;

	for(i=0; i<sizeof(array)/sizeof(array[0]); i++) {
		if(array[i].kq==-1) break;
	}
	if(i==sizeof(array)/sizeof(array[0])) return NULL;

	array[i].kq = kqueue();
	if(array[i].kq==-1) {
//		SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_create(kqueue) : kqueue() fail, errno=%d", errno);
		return NULL;
	}

	array[i].quit_flag = 0;
	return &array[i];
}

int fdwatch_destroy(FDWATCH_HANDLE handle)
{
	close(handle->kq);
	handle->kq = -1;
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
	struct kevent kqevt[2];
	int ret;

	memset(&kqevt,0,sizeof(struct kevent));
	if(item->flags&FDWATCH_READ) {
		kqevt[0].ident	= item->fd;
		kqevt[0].filter	= EVFILT_READ;
		if(item->flags&FDWATCH_ONCE)	kqevt[0].flags  = EV_ADD|EV_ENABLE|EV_ONESHOT;
		else							kqevt[0].flags  = EV_ADD|EV_ENABLE;
		kqevt[0].udata = item;
	}	
	if(item->flags&FDWATCH_WRITE) {
		kqevt[1].ident	= item->fd;
		kqevt[1].filter	= EVFILT_WRITE;
		if(item->flags&FDWATCH_ONCE)	kqevt[1].flags  = EV_ADD|EV_ENABLE|EV_ONESHOT;
		else							kqevt[1].flags  = EV_ADD|EV_ENABLE;
		kqevt[1].udata = item;
	}	

	switch(item->flags&(FDWATCH_READ|FDWATCH_WRITE)) {
	case FDWATCH_READ:					ret = kevent(handle->kq, &kqevt[0], 1, NULL, 0, NULL); break;
	case FDWATCH_WRITE:					ret = kevent(handle->kq, &kqevt[1], 1, NULL, 0, NULL); break;
	case FDWATCH_READ|FDWATCH_WRITE:	ret = kevent(handle->kq, &kqevt[0], 2, NULL, 0, NULL); break;
	default: return ERR_INVALID_PARAMETER;
	}
	if(ret==-1) {
//		SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_add(epoll) : epoll_ctl() fail, errno()=%s", errno);
		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

int fdwatch_remove(FDWATCH_HANDLE handle, FDWATCH_ITEM* item)
{
	struct kevent kqevt[2];
	int ret;

	memset(&kqevt,0,sizeof(struct kevent));
	if(item->flags&FDWATCH_READ) {
		kqevt[0].ident	= item->fd;
		kqevt[0].filter	= EVFILT_READ;
		kqevt[0].flags  = EV_DELETE;
		kqevt[0].udata	= item;
	}
	if(item->flags&FDWATCH_WRITE) {
		kqevt[1].ident	= item->fd;
		kqevt[1].filter	= EVFILT_WRITE;
		kqevt[1].flags  = EV_DELETE;
		kqevt[1].udata	= item;
	}

	switch(item->flags&(FDWATCH_READ|FDWATCH_WRITE)) {
	case FDWATCH_READ:					ret = kevent(handle->kq, &kqevt[0], 1, NULL, 0, NULL); break;
	case FDWATCH_WRITE:					ret = kevent(handle->kq, &kqevt[1], 1, NULL, 0, NULL); break;
	case FDWATCH_READ|FDWATCH_WRITE:	ret = kevent(handle->kq, &kqevt[0], 2, NULL, 0, NULL); break;
	default: return ERR_INVALID_PARAMETER;
	}
	if(ret==-1) {
//		SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_add(epoll) : epoll_ctl() fail, errno()=%s", errno);
		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

int fdwatch_rearm(FDWATCH_HANDLE handle, FDWATCH_ITEM* item)
{
	struct kevent kqevt[2];
	int ret;

	memset(&kqevt,0,sizeof(struct kevent));
	if(item->flags&FDWATCH_READ) {
		kqevt[0].ident	= item->fd;
		kqevt[0].filter	= EVFILT_READ;
		if(item->flags&FDWATCH_ONCE)	kqevt[0].flags  = EV_ADD|EV_ENABLE|EV_ONESHOT;
		else							kqevt[0].flags  = EV_ADD|EV_ENABLE;
		kqevt[0].udata = item;
	}	
	if(item->flags&FDWATCH_WRITE) {
		kqevt[1].ident	= item->fd;
		kqevt[1].filter	= EVFILT_WRITE;
		if(item->flags&FDWATCH_ONCE)	kqevt[1].flags  = EV_ADD|EV_ENABLE|EV_ONESHOT;
		else							kqevt[1].flags  = EV_ADD|EV_ENABLE;
		kqevt[1].udata = item;
	}	

	switch(item->flags&(FDWATCH_READ|FDWATCH_WRITE)) {
	case FDWATCH_READ:					ret = kevent(handle->kq, &kqevt[0], 1, NULL, 0, NULL); break;
	case FDWATCH_WRITE:					ret = kevent(handle->kq, &kqevt[1], 1, NULL, 0, NULL); break;
	case FDWATCH_READ|FDWATCH_WRITE:	ret = kevent(handle->kq, &kqevt[0], 2, NULL, 0, NULL); break;
	default: return ERR_INVALID_PARAMETER;
	}
	if(ret==-1) {
//		SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_add(epoll) : epoll_ctl() fail, errno()=%s", errno);
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
	struct kevent kqevts[2000];

	for(;;) {
		if(timeout==FDWATCH_INFINITE) {
			ret = kevent(handle->kq, NULL, 0, kqevts, sizeof(kqevts)/sizeof(kqevts[0]), NULL);
		} else {
			struct timespec tv;
			tv.tv_sec = timeout /1000;
			tv.tv_nsec = (timeout%1000) * 1000;
			ret = kevent(handle->kq, NULL, 0, kqevts, sizeof(kqevts)/sizeof(kqevts[0]), &tv);
		}
		if(ret>=0) break;
		if(errno!=EINTR) {
//			SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_add(epoll) : epoll_ctl() fail, errno()=%s", errno);
			return ERR_UNKNOWN;
		}
	}

	for(; ret>0; ret--) {
		int events = 0;
		if(kqevts[ret-1].filter&EVFILT_READ) events |= FDWATCH_READ;
		if(kqevts[ret-1].filter&EVFILT_WRITE) events |= FDWATCH_WRITE;
		((FDWATCH_ITEM*)kqevts[ret-1].udata)->event((FDWATCH_ITEM*)kqevts[ret-1].udata, events);
	}

	return ERR_NOERROR;
}

int fdwatch_break(FDWATCH_HANDLE handle)
{
	handle->quit_flag = 1;
	return ERR_NOERROR;
}
