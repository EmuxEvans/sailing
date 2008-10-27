
#include <string.h>
#include <assert.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/threadpool.h"
#include "../../inc/skates/log.h"
#include "../../inc/skates/applog.h"

// STATIC CONFIG : Start
#define THREADPOOL_QUEUE_MAX		(20*1000)
// STATIC CONFIG : End

typedef struct MSG_DATA {
	THREAD_PROC			proc;
	void*				arg;
} MSG_DATA;

typedef struct THREAD_CTX {
	int				index;
	os_thread_t		tid;
	unsigned int (ZION_CALLBACK *proc)(void*);
	void*			arg;

	THREADPOOL_INFO	info;

} THREAD_CTX;

// declare internal funcations
static unsigned int ZION_CALLBACK workthread_proc(void* arg);
static int threadpool_postquitmessage();

// define internal variables
static os_mutex_t				msgq_mtx;
static os_sem_t					msgq_sem;
static MSG_DATA					msgq[THREADPOOL_QUEUE_MAX];
static unsigned int				msgq_cur, msgq_count;
static unsigned int				thread_count;
static unsigned int				thread_busy = 0;
static os_thread_key_t			thread_key;
static THREAD_CTX				threads[THREADPOOL_MAX_WORKERTHREAD];
static os_mutex_t				threads_mtx;

// define external function
int threadpool_init(unsigned int thread_num)
{
	unsigned int i;

	os_mutex_init(&msgq_mtx);
	os_sem_init(&msgq_sem, 0);
	msgq_cur = msgq_count = 0;

	thread_count = 0;
	os_thread_key_init(&thread_key);
	memset(&threads, 0xff, sizeof(threads));
	os_mutex_init(&threads_mtx);

	for(i=0; i<thread_num; i++) {
		threads[i].index = i;
		threads[i].proc = NULL;
		threads[i].arg = NULL;

		if(os_thread_begin(&threads[i].tid, workthread_proc, &threads[i])!=0) {
			for(i=0; i<thread_count; i++) {
				threadpool_postquitmessage();
			}
			for(; i>=0; i--) {
				os_thread_wait(threads[i].tid, NULL);
			}

			os_mutex_init(&msgq_mtx);
			os_sem_init(&msgq_sem, 0);
			os_thread_key_init(&thread_key);

			return(ERR_UNKNOWN);
		}

		++thread_count;
	}

	return(ERR_NOERROR);
}

int threadpool_final()
{
	unsigned int i;

	// wait worker thread
	for(i=0; i<thread_count; i++) {
		threadpool_postquitmessage();
	}
	for(i=0; i<thread_count; i++) {
		os_thread_wait(threads[i].tid, NULL);
	}

	//
	for(i=thread_count; i<sizeof(threads)/sizeof(threads[0]); i++) {
		if(threads[i].index==-1) continue;
		SYSLOG(LOG_WARNING, MODULE_NAME, "threadpool: userthread(%d) alreay running.", i);
	}

	//
	os_mutex_destroy(&threads_mtx);
	os_sem_destroy(&msgq_sem);
	os_thread_key_destroy(&thread_key);
	os_mutex_destroy(&msgq_mtx);

	return(ERR_NOERROR);
}

int threadpool_queueitem(THREAD_PROC proc, void* arg)
{
	int ret;
	if(proc==NULL) return ERR_INVALID_PARAMETER;
	os_mutex_lock(&msgq_mtx);
	if(msgq_count<THREADPOOL_QUEUE_MAX) {
		msgq[(msgq_cur+msgq_count)%THREADPOOL_QUEUE_MAX].proc	= proc;
		msgq[(msgq_cur+msgq_count)%THREADPOOL_QUEUE_MAX].arg	= arg;
		msgq_count++;
		ret = ERR_NOERROR;
	} else {
		// do someting to log
		ret = ERR_FULL;
	}
	os_mutex_unlock(&msgq_mtx);
	if(ret==ERR_NOERROR) os_sem_post(&msgq_sem);
	return ret;
}

int ZION_API threadpool_getindex()
{
	THREAD_CTX* tc;
	tc = os_thread_key_get(&thread_key);
	return(tc!=NULL?tc->index:-1);
}

int threadpool_getcount()
{
	return thread_count;
}

unsigned int ZION_CALLBACK workthread_proc(void* arg)
{
	int ret;
	THREAD_CTX* tc;
	MSG_DATA msg;

	tc = (THREAD_CTX*)arg;

	if(os_thread_key_set(&thread_key, (void*)tc)!=0)
		assert(0);
	if(os_thread_key_get(&thread_key)!=tc)
		assert(0);

	if(tc->proc==NULL) {
		for(;;) {
			ret = os_sem_wait(&msgq_sem);
			if(ret!=0) {
				continue;
			}

			os_mutex_lock(&msgq_mtx);
			assert(msgq_count>0);
			memcpy(&msg, &msgq[msgq_cur], sizeof(MSG_DATA));
			msgq_cur = (msgq_cur+1) % THREADPOOL_QUEUE_MAX;
			msgq_count--;
			os_mutex_unlock(&msgq_mtx);

			if(msg.proc==NULL) break;
			atom_inc(&thread_busy);
			msg.proc(msg.arg);
			atom_dec(&thread_busy);
		}
	} else {
		tc->proc(tc->arg);
	}

	threads[tc->index].index = -1;
	return 0;
}

int threadpool_postquitmessage()
{
	int ret;
	os_mutex_lock(&msgq_mtx);
	if(msgq_count<THREADPOOL_QUEUE_MAX) {
		msgq[(msgq_cur+msgq_count)%THREADPOOL_QUEUE_MAX].proc	= NULL;
		msgq[(msgq_cur+msgq_count)%THREADPOOL_QUEUE_MAX].arg	= NULL;
		msgq_count++;
		ret = ERR_NOERROR;
	} else {
		// do someting to log
		ret = ERR_FULL;
	}
	os_mutex_unlock(&msgq_mtx);
	if(ret==ERR_NOERROR) os_sem_post(&msgq_sem);
	return ret;
}

int threadpool_begin(os_thread_t* handle, unsigned int (ZION_CALLBACK *proc)(void*), void* arg)
{
	int ret;
	int index;

	os_mutex_lock(&threads_mtx);
	for(index=thread_count; index<sizeof(threads)/sizeof(threads[0]); index++) {
		if(threads[index].index==-1) break;
	}
	if(index==sizeof(threads)/sizeof(threads[0])) {
		os_mutex_unlock(&threads_mtx);
		return ERR_FULL;
	}
	threads[index].index = index;
	threads[index].proc = proc;
	threads[index].arg = arg;
	os_mutex_unlock(&threads_mtx);

	ret = os_thread_begin(&threads[index].tid, workthread_proc, &threads[index]);
	if(ret!=0) {
		threads[index].index = -1;
		return ERR_UNKNOWN;
	}

	memcpy(handle, &threads[index].tid, sizeof(*handle));
	return ERR_NOERROR;
}

int threadpool_s()
{
	int index;

	os_mutex_lock(&threads_mtx);
	for(index=thread_count; index<sizeof(threads)/sizeof(threads[0]); index++) {
		if(threads[index].index==-1) break;
	}
	if(index==sizeof(threads)/sizeof(threads[0])) {
		os_mutex_unlock(&threads_mtx);
		return ERR_FULL;
	}
	threads[index].index = index;
	threads[index].proc = NULL;
	threads[index].arg = NULL;
	os_thread_get(&threads[index].tid);
	os_mutex_unlock(&threads_mtx);

	if(os_thread_key_set(&thread_key, (void*)&threads[index])!=0)
		assert(0);
	if(os_thread_key_get(&thread_key)!=(void*)&threads[index])
		assert(0);

	return ERR_NOERROR;
}

int threadpool_e()
{
	int index;

	index = threadpool_getindex();
	if(index<0) return ERR_NOT_WORKER;

	threads[index].index = -1;

	if(os_thread_key_set(&thread_key, NULL)!=0)
		assert(0);
	if(os_thread_key_get(&thread_key)!=NULL)
		assert(0);

	return ERR_NOERROR;
}

int threadpool_getinfo(int index, THREADPOOL_INFO* info)
{
	if(index<0 || index>=sizeof(threads)/sizeof(threads[0])) return ERR_INVALID_PARAMETER;
	if(threads[index].index==-1) return ERR_EMPTY;
	memcpy(info, &threads[index].info, sizeof(*info));
	return ERR_NOERROR;
}

void threadpool_getstatus(THREADPOOL_STATUS* status)
{
	status->queue_size = msgq_count;
	status->count = thread_count;
	status->busy = thread_busy;
}
