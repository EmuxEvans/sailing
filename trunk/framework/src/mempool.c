#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/rlist.h"
#include "../inc/mempool.h"
#include "../inc/log.h"
#include "../inc/applog.h"

// STATIC CONFIG : Start
#define MEMPOOL_ARRAY_SIZE 30
// STATIC CONFIG : End

struct MEMPOOL_ITEM;
typedef struct MEMPOOL_ITEM MEMPOOL_ITEM;

struct MEMPOOL_ITEM {
	union {
		MEMPOOL_ITEM*		next;
		ATOM_SLIST_ENTRY	entry;
	};

#ifdef MEMPOOL_DEBUG
	const char*				filename;
	int						lineno;
	const char*				free_filename;
	int						free_lineno;
	MEMPOOL_ITEM*			use_next;
	MEMPOOL_ITEM*			use_prev;
#endif
};

struct MEMPOOL_CB {
	ATOM_SLIST_HEADER		unuse_list;
	const char*				name;
	int						size;
	int						res_count;
	int						unuse_count;
	int						init_count;

#ifdef MEMPOOL_DEBUG
	MEMPOOL_ITEM* use_list;
	const char* filename;
	int lineno;
#endif
};

static MEMPOOL_CB cb_array[MEMPOOL_ARRAY_SIZE];
static MEMPOOL_HANDLE cb_alloc();
static void cb_free(MEMPOOL_HANDLE handle);

#ifdef MEMPOOL_DEBUG
	static os_mutex_t used_mutex;
	#define unuse_mutex_init		os_mutex_init(&used_mutex)
	#define unuse_mutex_del			os_mutex_destroy(&used_mutex)
	#define unuse_mutex_lock		os_mutex_lock(&used_mutex)
	#define unuse_mutex_unlock		os_mutex_unlock(&used_mutex)
#else
	#define unuse_mutex_init
	#define unuse_mutex_del
	#define unuse_mutex_lock
	#define unuse_mutex_unlock
#endif

void mempool_init()
{
	memset(cb_array, 0, sizeof(cb_array));
	unuse_mutex_init;
}

void mempool_final()
{
	int idx;
	for(idx=0; idx<sizeof(cb_array)/sizeof(cb_array[0]); idx++) {
		if(cb_array[idx].size==0) continue;
		assert(0);
//#ifdef MEMPOOL_DEBUG
//		SYSLOG(LOG_WARNING, MODULE_NAME, "mempool not destroy (%d) %s:%d", idx, cb_array[idx].filename, cb_array[idx].lineno);
//#else
//		SYSLOG(LOG_WARNING, MODULE_NAME, "mempool not destroy (%d)", idx);
//#endif
		mempool_destroy(cb_array+idx);
	}
	unuse_mutex_del;
}

#ifdef MEMPOOL_DEBUG
MEMPOOL_HANDLE mempool_create_debug(const char* filename, unsigned int lineno, const char* name, unsigned int size, unsigned int initcount)
#else
MEMPOOL_HANDLE mempool_create(const char* name, unsigned int size, unsigned int initcount)
#endif
{
	MEMPOOL_HANDLE handle;

	handle = cb_alloc();
	if(handle==NULL) return NULL;

	atom_slist_init(&handle->unuse_list);
	handle->name		= name;
	handle->size		= size;
	handle->res_count	= 0;
	handle->unuse_count	= 0;

#ifdef MEMPOOL_DEBUG
	handle->use_list		= NULL;
	handle->filename		= filename;
	handle->lineno		= lineno;
#endif
	
	return handle;
}

void mempool_destroy(MEMPOOL_HANDLE handle)
{
	MEMPOOL_ITEM* item;

	if(handle->res_count!=handle->unuse_count) {
#ifdef MEMPOOL_DEBUG
		SYSLOG(LOG_WARNING,	MODULE_NAME, "%s(%s:%d): mempool resource leak. %d/%d",
							handle->name, handle->filename, handle->lineno,
							handle->unuse_count, handle->res_count);
#else
		SYSLOG(LOG_WARNING, MODULE_NAME, "%s(%d): mempool resource leak. %d/%d",
							handle->name, handle-cb_array,
							handle->unuse_count, handle->res_count);
#endif
	}

#ifdef MEMPOOL_DEBUG
	while(handle->use_list!=NULL) {
		SYSLOG(LOG_WARNING, MODULE_NAME, "%p %s:%d",
							handle->use_list+1,
							handle->use_list->filename, handle->use_list->lineno);
		mempool_free(handle, (void*)(handle->use_list+1));
	}
#endif

	for(;;) {
		item = (MEMPOOL_ITEM*)atom_slist_pop(&handle->unuse_list);
		if(item==NULL) break;
		free(item);
		atom_dec((unsigned int*)&handle->res_count);
	}

	cb_free(handle);
}

#ifdef MEMPOOL_DEBUG
void* mempool_alloc_debug(const char* filename, unsigned int lineno, MEMPOOL_HANDLE handle)
#else
void* mempool_alloc(MEMPOOL_HANDLE handle)
#endif
{
	MEMPOOL_ITEM* item;

	item = (MEMPOOL_ITEM*)atom_slist_pop(&handle->unuse_list);
	if(item==NULL) {
		item = (MEMPOOL_ITEM*)malloc(sizeof(MEMPOOL_ITEM)+handle->size);
		if(item==NULL) return NULL;
		atom_inc((unsigned int*)&handle->res_count);
	} else {
		atom_dec((unsigned int*)&handle->unuse_count);
	}

#ifdef MEMPOOL_DEBUG
	item->filename		= filename;
	item->lineno		= lineno;
	item->free_filename	= NULL;
	item->free_lineno	= 0;

	unuse_mutex_lock;
	item->use_prev		= NULL;
	item->use_next		= handle->use_list;
	if(handle->use_list!=NULL) handle->use_list->use_prev = item;
	handle->use_list = item;
	unuse_mutex_unlock;
#endif

	return (void*)(item+1);
}

#ifdef MEMPOOL_DEBUG
void mempool_free_debug(const char* filename, unsigned int lineno, MEMPOOL_HANDLE handle, void* ptr)
#else
void mempool_free(MEMPOOL_HANDLE handle, void* ptr)
#endif
{
	MEMPOOL_ITEM* item;

	item = (MEMPOOL_ITEM*)ptr - 1;

#ifdef MEMPOOL_DEBUG
	assert(item->filename!=NULL);
	assert(item->lineno!=0);
	item->filename		= NULL;
	item->lineno		= 0;
	item->free_filename	= filename;
	item->free_lineno	= lineno;

	unuse_mutex_lock;
	if(item->use_prev!=NULL) item->use_prev->use_next = item->use_next;
	if(item->use_next!=NULL) item->use_next->use_prev = item->use_prev;
	if(handle->use_list==item) handle->use_list = item->use_next;
	unuse_mutex_unlock;

	item->use_next = NULL;
	item->use_prev = NULL;
#endif
	
	atom_inc((unsigned int*)&handle->unuse_count);
	atom_slist_push(&handle->unuse_list, &item->entry);
}

int mempool_get_info(MEMPOOL_INFO* info, int count)
{
	return 0;
}

MEMPOOL_HANDLE cb_alloc()
{
	int i;

	for(i=0; i<sizeof(cb_array)/sizeof(cb_array[0]); i++) {
		if(cb_array[i].size==0) return cb_array+i;
	}

	return NULL;
}

void cb_free(MEMPOOL_HANDLE handle)
{
	handle->size = 0;
}

