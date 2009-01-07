#ifndef _THREADPOOL_INCLUDE_
#define _THREADPOOL_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#define THREADPOOL_MAX_WORKERTHREAD			20

// thread pool
typedef void (*THREAD_PROC)(void* arg);

ZION_API int threadpool_init(unsigned int maxnum);
ZION_API int threadpool_final();

ZION_API int threadpool_queueitem(THREAD_PROC proc, void* arg);
ZION_API int threadpool_getindex();
ZION_API int threadpool_getcount();

ZION_API int threadpool_begin(os_thread_t* handle, unsigned int (ZION_CALLBACK *proc)(void*), void* arg);
ZION_API int threadpool_s();
ZION_API int threadpool_e();

typedef struct {
	unsigned int	times;
	void*			event_proc;
} THREADPOOL_INFO;

ZION_API int threadpool_getinfo(int index, THREADPOOL_INFO* info);
ZION_API unsigned int threadpool_queuesize();

#ifdef __cplusplus
}
#endif

#endif
