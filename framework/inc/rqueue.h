#ifndef __RQUEUE_INCLUDE__
#define __RQUEUE_INCLUDE__

struct RQUEUE;
typedef struct RQUEUE RQUEUE;

struct RQUEUE_CLIENT;
typedef struct RQUEUE_CLIENT RQUEUE_CLIENT;

ZION_API void rqueue_init();
ZION_API void rqueue_final();

ZION_API RQUEUE* rqueue_create(SOCK_ADDR* sa, int flags, const char* path);
ZION_API void rqueue_destroy(RQUEUE* queue);

ZION_API RQUEUE_CLIENT* rqueue_get(const SOCK_ADDR* sa);
ZION_API int rqueue_write(RQUEUE_CLIENT* client, void* buf, int len);
ZION_API int rqueue_read(RQUEUE* queue, void* buf, int size);

#endif
