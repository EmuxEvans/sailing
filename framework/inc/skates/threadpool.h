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

typedef struct {
	const char* name;
} THREADPOOL_INFO;

typedef struct {
	int		queue_size;
	int		count;
	int		busy;
} THREADPOOL_STATUS;

ZION_API int threadpool_getinfo(int index, THREADPOOL_INFO* info);
ZION_API void threadpool_getstatus(THREADPOOL_STATUS* status);

#ifdef __cplusplus
}
#endif

#endif
