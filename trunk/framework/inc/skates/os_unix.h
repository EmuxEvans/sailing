
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <unistd.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ZION_API
#define ZION_API
#endif
#ifndef ZION_CLASS
#define ZION_CLASS
#endif
#ifndef ZION_INLINE
#define ZION_INLINE		static inline
#endif
#ifndef ZION_CALLBACK
#define ZION_CALLBACK
#endif
#ifndef ZION_EXPORT
#define ZION_EXPORT
#endif

typedef unsigned char		os_byte;
typedef unsigned short		os_word;
typedef unsigned int		os_dword;
typedef unsigned long long	os_qword;
typedef char				os_char;
typedef short				os_short;
typedef int					os_int;
typedef long long			os_long;
typedef float				os_float;
typedef double				os_double;

ZION_INLINE int os_last_error();

typedef long long os_time_t;
ZION_INLINE void os_sleep(unsigned int u);
ZION_INLINE void os_time_get(os_time_t* t);

typedef pthread_mutex_t os_mutex_t;
ZION_INLINE void os_mutex_init(os_mutex_t* mtx);
ZION_INLINE void os_mutex_destroy(os_mutex_t* mtx);
ZION_INLINE int os_mutex_lock(os_mutex_t* mtx);
ZION_INLINE int os_mutex_unlock(os_mutex_t* mtx);

typedef pthread_cond_t os_condition_t;
ZION_INLINE void os_condition_init(os_condition_t* cond);
ZION_INLINE void os_condition_destroy(os_condition_t* cond);
ZION_INLINE int os_condition_wait(os_condition_t* cond, os_mutex_t* mtx);
ZION_INLINE int os_condition_signal(os_condition_t* cond);
ZION_INLINE int os_condition_broadcast(os_condition_t* cond);

typedef sem_t os_sem_t;
ZION_INLINE int os_sem_init(os_sem_t* sem, unsigned int init);
ZION_INLINE int os_sem_destroy(os_sem_t* sem);
ZION_INLINE int os_sem_wait(os_sem_t* handle);
ZION_INLINE int os_sem_post(os_sem_t* sem);

typedef pthread_t os_thread_t;
ZION_INLINE int os_thread_begin(os_thread_t* handle, unsigned int (ZION_CALLBACK *proc)(void*), void* arg);
ZION_INLINE int os_thread_close(os_thread_t handle);
ZION_INLINE void os_thread_exit(unsigned int code);
ZION_INLINE int os_thread_wait(os_thread_t handle, unsigned int* retcode);

typedef pthread_key_t os_thread_key_t;
ZION_INLINE int os_thread_key_init(os_thread_key_t* key);
ZION_INLINE int os_thread_key_destroy(os_thread_key_t* key);
ZION_INLINE void* os_thread_key_get(os_thread_key_t* key);
ZION_INLINE int os_thread_key_set(os_thread_key_t* key, void* value);

typedef pid_t os_process_t;
ZION_INLINE os_process_t os_process_get();
ZION_INLINE int os_process_getid();

#define OS_LIBARAY_PREFIX		".so"
typedef void* os_library_t;
ZION_INLINE int os_library_open(os_library_t* handle, const char* name);
ZION_INLINE int os_library_close(os_library_t handle);
ZION_INLINE void* os_library_get(os_library_t handle, const char* name);
ZION_INLINE const char* os_library_error();

#define atom_inc(p)				atom_unix_inc((os_dword volatile*)p)
#define atom_dec(p)				atom_unix_dec((os_dword volatile*)p)
#define atom_swap(p, v)			atom_unix_swap((os_dword volatile*)p, (os_dword)v)
#define atom_cas(p, v, c)		atom_unix_cas((os_dword volatile*)p, (os_dword)v, (os_dword)c)
#define atom_cas_ptr(p, v, c)	atom_unix_cas_ptr((void* volatile*)p, (void*)v, (void*)c)
#define atom_exchg_add(p, v)	atom_unix_exchg_add((os_dword volatile*)p, (os_dword)v)

typedef struct ATOM_SLIST_ENTRY			{ struct ATOM_SLIST_ENTRY *Next; }		ATOM_SLIST_ENTRY;
typedef struct ATOM_SLIST_HEADER		{ ATOM_SLIST_ENTRY* First; long Count;}	ATOM_SLIST_HEADER;
ZION_API void				atom_slist_init(ATOM_SLIST_HEADER* header);
ZION_API ATOM_SLIST_ENTRY*	atom_slist_pop(ATOM_SLIST_HEADER* header);
ZION_API ATOM_SLIST_ENTRY*	atom_slist_push(ATOM_SLIST_HEADER* head, ATOM_SLIST_ENTRY* ListEntry);
ZION_API ATOM_SLIST_ENTRY*	atom_slist_flush(ATOM_SLIST_HEADER* header);

ZION_API int os_fileexist(const char* file);
ZION_API int os_isdir(const char* dir);
ZION_API int os_mkdir(const char* dir);

ZION_INLINE int os_last_error()
{
	return errno;
}

ZION_INLINE void os_sleep(unsigned int u)
{
	usleep(u*1000);
}

ZION_INLINE void os_time_get(os_time_t* t)
{
	struct timeval tv;
	gettimeofday(&tv, NULL) ;

	*t = (os_time_t)tv.tv_sec * 1000 + (os_time_t)tv.tv_usec / 1000;
}

ZION_INLINE void os_mutex_init(os_mutex_t* mtx)
{
	pthread_mutex_init(mtx, NULL);
}

ZION_INLINE void os_mutex_destroy(os_mutex_t* mtx)
{
	pthread_mutex_destroy(mtx);
}

ZION_INLINE int os_mutex_lock(os_mutex_t* mtx)
{
	return pthread_mutex_lock(mtx);
}

ZION_INLINE int os_mutex_unlock(os_mutex_t* mtx)
{
	return pthread_mutex_unlock(mtx);
}

ZION_INLINE void os_condition_init(os_condition_t* cond)
{
	pthread_cond_init(cond, NULL);
}

ZION_INLINE void os_condition_destroy(os_condition_t* cond)
{
	pthread_cond_destroy(cond);
}

ZION_INLINE int os_condition_wait(os_condition_t* cond, os_mutex_t* mtx)
{
	return pthread_cond_wait(cond, mtx);
}

ZION_INLINE int os_condition_signal(os_condition_t* cond)
{
	return pthread_cond_signal(cond);
}

ZION_INLINE int os_condition_broadcast(os_condition_t* cond)
{
	return pthread_cond_broadcast(cond);
}

ZION_INLINE int os_sem_init(os_sem_t* sem, unsigned int init)
{
	return sem_init(sem, 0, init);
}

ZION_INLINE int os_sem_destroy(os_sem_t* sem)
{
	return sem_destroy(sem);
}

ZION_INLINE int os_sem_wait(os_sem_t* handle)
{
	while(1) {
		if(sem_wait(handle)==0) return 0;
		if(errno!=EINTR) return errno;
	}
}

ZION_INLINE int os_sem_post(os_sem_t* sem)
{
	return sem_post(sem);
}

ZION_INLINE int os_thread_begin(os_thread_t* handle, unsigned int (ZION_CALLBACK *proc)(void*), void* arg)
{
	return pthread_create(handle, NULL, (void *(*)(void*))proc, arg);
}

ZION_INLINE int os_thread_close(os_thread_t handle)
{
	return 0;
}

ZION_INLINE void os_thread_exit(unsigned int code)
{
	union { unsigned int uint; void* ptr; } aa;
	aa.uint = code;
	pthread_exit(aa.ptr);
}

ZION_INLINE int os_thread_wait(os_thread_t handle, unsigned int* retcode)
{
	return pthread_join(handle, (void**)retcode);
}

ZION_INLINE int os_thread_key_init(os_thread_key_t* key)
{
	return pthread_key_create(key, NULL);
}

ZION_INLINE int os_thread_key_destroy(os_thread_key_t* key)
{
	return pthread_key_delete(*key);
}

ZION_INLINE void* os_thread_key_get(os_thread_key_t* key)
{
	return pthread_getspecific(*key);
}

ZION_INLINE int os_thread_key_set(os_thread_key_t* key, void* value)
{
	return pthread_setspecific(*key, value);
}

ZION_INLINE os_process_t os_process_get()
{
	return getpid();
}

ZION_INLINE int os_process_getid()
{
	return (int)getpid();
}

ZION_INLINE int os_library_open(os_library_t* handle, const char* name)
{
	void* ptr;
	ptr = dlopen(name, RTLD_NOW);
	if(ptr==NULL) return errno;
	*handle = ptr;
	return 0;
}

ZION_INLINE int os_library_close(os_library_t handle)
{
	return dlclose(handle);
}

ZION_INLINE void* os_library_get(os_library_t handle, const char* name)
{
	return dlsym(handle, name);
}

ZION_INLINE const char* os_library_error()
{
	return dlerror();
}

ZION_API os_dword atom_unix_inc(os_dword volatile *p);
ZION_API os_dword atom_unix_dec(os_dword volatile *p);
ZION_API os_dword atom_unix_swap(os_dword volatile *p, os_dword v);
ZION_API os_dword atom_unix_cas(os_dword volatile *p, os_dword v, unsigned c);
ZION_API void* atom_unix_cas_ptr(void* volatile *p, void* v, void* c);
ZION_API os_dword atom_unix_exchg_add(os_dword volatile *p, os_dword v);

#ifdef __cplusplus
}
#endif
