#include <assert.h>
#include <string.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/sock.h"
#include "../inc/rqueue.h"
#include "../inc/rlist.h"
#include "../inc/mempool.h"
#include "../inc/network.h"

struct RQUEUE {
	SOCK_ADDR		queue_addr;
	RLIST_HEAD		queue_clts;
	os_sem_t		queue_sem;
	os_mutex_t		queue_mutex;
};

struct RQUEUE_CLIENT {
	RLIST_ITEM		item;
	RQUEUE*			queue;
	SOCK_ADDR		addr;
	NETWORK_HANDLE	handle;
};

void rqueue_init()
{
}

void rqueue_final()
{
}

RQUEUE* rqueue_create(SOCK_ADDR* sa, int flags, const char* path)
{
	return NULL;
}

void rqueue_destroy(RQUEUE* queue)
{
}

RQUEUE_CLIENT* rqueue_get(const SOCK_ADDR* sa)
{
	return NULL;
}

int rqueue_write(RQUEUE_CLIENT* client, void* buf, int len)
{
	return 0;
}

int rqueue_read(RQUEUE* queue, void* buf, int size)
{
	return 0;
}
