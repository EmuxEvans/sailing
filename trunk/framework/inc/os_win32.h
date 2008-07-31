
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>






#ifndef ZION_API
#define ZION_API
#endif
#ifndef ZION_INLINE
#define ZION_INLINE		__inline
#endif
#ifndef ZION_CALLBACK
#define ZION_CALLBACK	__stdcall
#endif

typedef unsigned char		os_uchar;
typedef unsigned short		os_word;
typedef unsigned int		os_dword;
typedef unsigned __int64	os_qword;
typedef char				os_char;
typedef short				os_short;
typedef int					os_int;
typedef __int64				os_long;

ZION_INLINE int os_last_error();

typedef __int64 os_time_t;
ZION_INLINE void os_sleep(unsigned int u);
ZION_INLINE void os_time_get(os_time_t* t);

typedef CRITICAL_SECTION os_mutex_t;
ZION_INLINE void os_mutex_init(os_mutex_t* mtx);
ZION_INLINE void os_mutex_destroy(os_mutex_t* mtx);
ZION_INLINE int os_mutex_lock(os_mutex_t* mtx);
ZION_INLINE int os_mutex_unlock(os_mutex_t* mtx);

typedef struct { HANDLE sem; DWORD count; } os_condition_t;
ZION_INLINE void os_condition_init(os_condition_t* cond);
ZION_INLINE void os_condition_destroy(os_condition_t* cond);
ZION_API int os_condition_wait(os_condition_t* cond, os_mutex_t* mtx);
ZION_API int os_condition_signal(os_condition_t* cond);
ZION_API int os_condition_boardcast(os_condition_t* cond);

typedef HANDLE os_sem_t;
ZION_INLINE int os_sem_init(os_sem_t* sem, unsigned int init);
ZION_INLINE int os_sem_destroy(os_sem_t* sem);
ZION_INLINE int os_sem_wait(os_sem_t* handle);
ZION_INLINE int os_sem_post(os_sem_t* sem);

typedef HANDLE os_thread_t;
ZION_INLINE int os_thread_begin(os_thread_t* handle, unsigned int (ZION_CALLBACK *proc)(void*), void* arg);
ZION_INLINE int os_thread_close(os_thread_t handle);
ZION_INLINE void os_thread_exit(unsigned int code);
ZION_INLINE int os_thread_wait(os_thread_t handle, unsigned int* retcode);

typedef DWORD os_thread_key_t;
ZION_INLINE int os_thread_key_init(os_thread_key_t* key);
ZION_INLINE int os_thread_key_destroy(os_thread_key_t* key);
ZION_INLINE void* os_thread_key_get(os_thread_key_t* key);
ZION_INLINE int os_thread_key_set(os_thread_key_t* key, void* value);
ZION_INLINE void os_thread_switch();

typedef HANDLE os_process_t;
ZION_INLINE os_process_t os_process_get();
ZION_INLINE int os_process_getid();

typedef HANDLE os_shm_t;
ZION_API int os_shm_create(os_shm_t* sm, const char* name, unsigned int size);
ZION_API int os_shm_open(os_shm_t* sm, const char* name);
ZION_API int os_shm_close(os_shm_t sm);
ZION_API void* os_shm_map(os_shm_t sm, unsigned int* size);
ZION_API int os_shm_unmap(os_shm_t sm, void* ptr, unsigned int size);

#define OS_LIBARAY_PREFIX		".dll"
typedef HMODULE os_library_t;
ZION_INLINE int os_library_open(os_library_t* handle, const char* name);
ZION_INLINE int os_library_close(os_library_t handle);
ZION_INLINE void* os_library_get(os_library_t handle, const char* name);
ZION_INLINE const char* os_library_error();

#define atom_inc(p)						(long)InterlockedIncrement((LONG*)p)
#define atom_dec(p)						(long)InterlockedDecrement((LONG*)p)
#define atom_exchg(p, v)				(long)InterlockedExchange((LONG*), (LONG*)v)
#define atom_cmp_exchg(p, v, c)			(long)InterlockedCompareExchange((LONG*)p, (LONG)v, (LONG)c)
#define atom_cmp_exchg_ptr(p, v, c)		InterlockedCompareExchangePointer(p, v, c)
#define atom_exchg_add(p, v)			(long)InterlockedExchangeAdd((LONG*)p, (LONG)v)

#define ATOM_SLIST_ENTRY				SLIST_ENTRY
#define ATOM_SLIST_HEADER				SLIST_HEADER
#define atom_slist_init(head)			InitializeSListHead(head)
#define atom_slist_pop(head)			InterlockedPopEntrySList(head)
#define atom_slist_push(head, item)		InterlockedPushEntrySList(head, item)
#define atom_slist_flush(head)			InterlockedFlushSList(head)

ZION_API int os_fileexist(const char* file);
ZION_API int os_isdir(const char* dir);
ZION_API int os_mkdir(const char* dir);

ZION_INLINE int os_last_error()
{
	return (int)GetLastError();
}

ZION_INLINE void os_sleep(unsigned int u)
{
	Sleep(u);
}

ZION_INLINE void os_time_get(os_time_t* t)
{
	GetSystemTimeAsFileTime((FILETIME*)t);
	*t /= 10*1000;
}

ZION_INLINE void os_mutex_init(os_mutex_t* mtx)
{
	InitializeCriticalSection(mtx);
}

ZION_INLINE void os_mutex_destroy(os_mutex_t* mtx)
{
	DeleteCriticalSection(mtx);
}

ZION_INLINE int os_mutex_lock(os_mutex_t* mtx)
{
	EnterCriticalSection(mtx);
	return 0;
}

ZION_INLINE int os_mutex_unlock(os_mutex_t* mtx)
{
	LeaveCriticalSection(mtx);
	return 0;
}

ZION_INLINE void os_condition_init(os_condition_t* cond)
{
	cond->sem = CreateSemaphore(NULL, 0, 5000, NULL);
	cond->count = 0;
}

ZION_INLINE void os_condition_destroy(os_condition_t* cond)
{
	CloseHandle(cond->sem);
	cond->sem = NULL;
}

ZION_INLINE int os_sem_init(os_sem_t* sem, unsigned int init)
{
	*sem = CreateSemaphore(NULL, (LONG)init, 65536, NULL);
	if(*sem==NULL) return GetLastError();
	return 0;
}

ZION_INLINE int os_sem_destroy(os_sem_t* sem)
{
	return CloseHandle(*sem)?0:(int)GetLastError();
}

ZION_INLINE int os_sem_wait(os_sem_t* handle)
{
	DWORD ret;
	ret = WaitForSingleObject(*handle, INFINITE);
	return ret==WAIT_OBJECT_0?0:(int)ret;
}

ZION_INLINE int os_sem_post(os_sem_t* sem)
{
	return ReleaseSemaphore(*sem, 1, NULL)?0:(int)GetLastError();
}

ZION_INLINE int os_thread_begin(os_thread_t* handle, unsigned int (ZION_CALLBACK *proc)(void*), void* arg)
{
	HANDLE th;
	th = (HANDLE)_beginthreadex(NULL, 0, proc, arg, 0, NULL);
	if(th==NULL) return (int)GetLastError();
	*handle = th;
	return 0;
}

ZION_INLINE int os_thread_close(os_thread_t handle)
{
	return CloseHandle(handle)?0:(int)GetLastError();
}

ZION_INLINE void os_thread_exit(unsigned int code)
{
	_endthreadex(code);
}

ZION_INLINE int os_thread_wait(os_thread_t handle, unsigned int* retcode)
{
	DWORD ret;
	ret = WaitForSingleObject(handle, INFINITE);
	if(ret != WAIT_OBJECT_0) return ret;
	if(retcode!=NULL && GetExitCodeThread(handle, (LPDWORD)retcode)) return (int)GetLastError();
	return 0;
}

ZION_INLINE int os_thread_key_init(os_thread_key_t* key)
{
	*key = TlsAlloc();
	return (*key==TLS_OUT_OF_INDEXES)?(int)GetLastError():0;
}

ZION_INLINE int os_thread_key_destroy(os_thread_key_t* key)
{
	return TlsFree(*key)?0:(int)GetLastError();
}

ZION_INLINE void* os_thread_key_get(os_thread_key_t* key)
{
	return TlsGetValue(*key);
}

ZION_INLINE void os_thread_switch()
{
}

ZION_INLINE int os_thread_key_set(os_thread_key_t* key, void* value)
{
	return TlsSetValue(*key, value)?0:(int)GetLastError();
}

ZION_INLINE os_process_t os_process_get()
{
	return GetCurrentProcess();
}

ZION_INLINE int os_process_getid()
{
	return GetCurrentProcessId();
}

ZION_INLINE int os_library_open(os_library_t* handle, const char* name)
{
	HMODULE m;
	m = LoadLibraryA(name);
	if(m==NULL) return (int)GetLastError();
	*handle = m;
	return 0;
}

ZION_INLINE int os_library_close(os_library_t handle)
{
	return FreeLibrary(handle)?0:(int)GetLastError();
}

ZION_INLINE void* os_library_get(os_library_t handle, const char* name)
{
	return GetProcAddress(handle, name);
}

ZION_INLINE const char* os_library_error()
{
	return "";
}
