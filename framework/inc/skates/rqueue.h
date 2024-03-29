#ifndef __RQUEUE_INCLUDE__
#define __RQUEUE_INCLUDE__

#ifdef __cplusplus
extern "C" {
#endif

#define RQUEUE_DATA_UNSAFE		(1<<1)
#define RQUEUE_WRITE_FILE		(1<<2)

struct RQUEUE;
typedef struct RQUEUE RQUEUE;

struct RQUEUE_CLIENT;
typedef struct RQUEUE_CLIENT RQUEUE_CLIENT;

struct RQUEUE_BUFFER;
typedef struct RQUEUE_BUFFER RQUEUE_BUFFER;

ZION_API void rqueue_init();
ZION_API void rqueue_final();

ZION_API RQUEUE* rqueue_create(const SOCK_ADDR* sa, unsigned int bufsize, unsigned int flags, const char* logpath);
ZION_API void rqueue_destroy(RQUEUE* queue);
ZION_API int rqueue_read(RQUEUE* queue, void* buf, unsigned int* len, unsigned int flags);

ZION_API RQUEUE_CLIENT* rqueue_get(const SOCK_ADDR* sa);
ZION_API RQUEUE_CLIENT* rqueue_client(RQUEUE* queue);
ZION_API int rqueue_write(RQUEUE_CLIENT* client, const void* buf, unsigned int len, unsigned int flags);

#ifdef __cplusplus
}
#endif

#endif
