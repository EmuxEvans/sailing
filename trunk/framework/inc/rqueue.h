#ifndef __RQUEUE_H__
#define __RQUEUE_H__

/*
#define RQUEUE_WRITE_FILE			0x001
#define RQUEUE_WRITE_TABLE			0x002

struct RQUEUE;
typedef struct RQUEUE RQUEUE;

struct RQUEUE_CLIENT;
typedef struct RQUEUE_CLIENT RQUEUE_CLIENT;

struct RQUEUE_BUFFER;
typedef struct RQUEUE_BUFFER RQUEUE_BUFFER;

ZION_API int rqueue_init();
ZION_API int rqueue_final();

ZION_API RQUEUE* rqueue_create(SOCK_ADDR* sa, int flags, const char* path);
ZION_API void rqueue_destroy(RQUEUE* queue);
ZION_API int rqueue_process(RQUEUE* queue, int time);

ZION_API RQUEUE_CLIENT* rqueue_get(const SOCK_ADDR* sa);
ZION_API int rqueue_write(RQUEUE_CLIENT* client, RQUEUE_BUFFER* buffer);
ZION_API RQUEUE_BUFFER* rqueue_read(RQUEUE* queue);
ZION_API void rqueue_free(RQUEUE_BUFFER* buffer);
ZION_API RQUEUE_BUFFER* rqueue_alloc(const void* buf, unsigned short len);

struct RQUEUE_BUFFER {
	char			data[RQUEUE_BUFFER_SIZE];
	unsigned short	len;
};
*/
#endif
