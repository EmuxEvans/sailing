#include <assert.h>
#include <string.h>

#include "../inc/skates.h"
#include "../inc/octopus.h"

struct FES_USER_CTX {
	OBJECT_ID				obj_id;
	int						filter_count;
	unsigned int			state;
	struct {
		void*				filter;
		FES_FILTER_DESC*	desc;
	} filter_list[0];
};

typedef struct FES_USERCTX_ITEM {
	ATOM_SLIST_ENTRY		entry;
	int						index;
	os_mutex_t				mutex;
	FES_USER_CTX*			ctx;
	MEMPOOL_HANDLE			mempool;
} FES_USERCTX_ITEM;

static FES_USERCTX_ITEM		userctx_list[FES_USERCTX_MAXCOUNT];
static ATOM_SLIST_HEADER	userctx_unused;
static unsigned int			userctx_seq = 0;

static struct { int index, ref_count; } userctx_thread[THREADPOOL_MAX_WORKERTHREAD];

void fes_userctx_init()
{
	int l;
	atom_slist_init(&userctx_unused);
	memset(userctx_thread, 0xff, sizeof(userctx_thread));
	for(l=0; l<FES_USERCTX_MAXCOUNT; l++) {
		userctx_list[l].index = l;
		os_mutex_init(&userctx_list[l].mutex);
		userctx_list[l].ctx = NULL;
		userctx_list[l].mempool	= NULL;
		atom_slist_push(&userctx_unused, &userctx_list[l].entry);
	}
}

void fes_userctx_final()
{
}
 
int fes_userctx_calcsize(FES_FILTER_DESC descs[], int count)
{
	int l, size;
	size = sizeof(FES_USER_CTX) + sizeof(((FES_USER_CTX*)NULL)->filter_list[0])*count;
	for(l=0; l<count; l++) size += descs[l].size;
	return size;
}

FES_USER_CTX* fes_userctx_create(MEMPOOL_HANDLE mempool, FES_FILTER_DESC descs[], int count)
{
	int l;
	char* cur;
	FES_USER_CTX* ctx;
	FES_USERCTX_ITEM* item;

	// 1: alloc user_ctx memory
	ctx = mempool_alloc(mempool);
	if(ctx==NULL) return NULL;
	// 2: alloc userctx_item
	item = (FES_USERCTX_ITEM*)atom_slist_pop(&userctx_unused);
	if(item==NULL) { mempool_free(mempool, ctx); return NULL; }

//	os_mutex_lock(&userctx_list[item->index].mutex);
	// 3: alloc filter memory
	ctx->filter_count	= count;
	ctx->state			= 0;
	cur = (char*)ctx + sizeof(FES_USER_CTX) + sizeof(ctx->filter_list[0])*count;
	for(l=0; l<ctx->filter_count; l++) {
		ctx->filter_list[l].filter	= cur;
		ctx->filter_list[l].desc	= &descs[l];
		cur += descs[l].size;
	}
	// 4: call filter init function
	for(l=0; l<count; l++) {
		ctx->filter_list[l].desc->init(ctx, ctx->filter_list[l].filter);
	}
	// 5: gen OBJECT_ID
	ctx->obj_id.index	= item->index;
	ctx->obj_id.seq		= (unsigned short)atom_inc(&userctx_seq);
	rpcnet_group_get_endpoint(rpcnet_getgroup(NULL), &ctx->obj_id.address);
	// 6:
	userctx_list[item->index].ctx		= ctx;
	userctx_list[item->index].mempool	= mempool;

	return ctx;
}

void fes_userctx_destroy(FES_USER_CTX* ctx)
{
	int l, index;

	index = ctx->obj_id.index;
	assert(index>=0 && index<FES_USERCTX_MAXCOUNT);
	assert(userctx_list[index].ctx==ctx);
	assert(userctx_list[index].index==ctx->obj_id.index);

	// 1: call filter destroy function
	for(l=0; l<ctx->filter_count; l++) {
		ctx->filter_list[l].desc->destroy(ctx, ctx->filter_list[l].filter);
	}
	// 2: 
	userctx_list[index].ctx		= NULL;
	// 3: free user_ctx memory
	mempool_free(userctx_list[index].mempool, ctx);
        userctx_list[index].mempool	= NULL;
	// 4: free userctx_item
	atom_slist_push(&userctx_unused, &userctx_list[index].entry);
}

void fes_userctx_disconnect(FES_USER_CTX* ctx)
{
	int l;
	for(l=0; l<ctx->filter_count; l++) {
		ctx->filter_list[l].desc->disconnect(ctx, ctx->filter_list[l].filter);
	}
}

void fes_userctx_get_objid(FES_USER_CTX* ctx, OBJECT_ID* id)
{
	memcpy(id, &ctx->obj_id, sizeof(ctx->obj_id));
}

FES_USER_CTX* fes_userctx_get_byid(const OBJECT_ID* id)
{
	FES_USER_CTX* ctx;
	int index;
	
	if(id->index>=FES_USERCTX_MAXCOUNT) return NULL;

	index = threadpool_getindex();
	if(index>=0 && userctx_thread[index].index==id->index) {
		userctx_thread[index].ref_count++;
		return userctx_list[id->index].ctx;
	} else {
		os_mutex_lock(&userctx_list[id->index].mutex);
		ctx = userctx_list[id->index].ctx;
		if(ctx==NULL || ctx->obj_id.seq!=id->seq) {
			os_mutex_unlock(&userctx_list[id->index].mutex);
			return NULL;
		} else {
			if(index>=0 && userctx_thread[index].index<0) {
				userctx_thread[index].index = id->index;
				userctx_thread[index].ref_count = 1;
			}
			return ctx;
		}
	}
}

FES_USER_CTX* fes_userctx_get_byindex(const int index)
{
	FES_USER_CTX* ctx = NULL;
	int thread_index;

	if(index>=FES_USERCTX_MAXCOUNT || index<0) {
		return NULL;
	}

	thread_index = threadpool_getindex();
	if(thread_index>=0 && userctx_thread[thread_index].index==index) {
		userctx_thread[index].ref_count++;
		return userctx_list[index].ctx;
	} else {
		os_mutex_lock(&userctx_list[index].mutex);
		ctx = userctx_list[index].ctx;
		if(ctx==NULL) {
			os_mutex_unlock(&userctx_list[index].mutex);
			return NULL;
		} else {
			if(index>=0 && userctx_thread[thread_index].index<0) {
				userctx_thread[thread_index].index = index;
				userctx_thread[thread_index].ref_count = 1;
			}
			return ctx;
		}
	}
}

void fes_userctx_release(FES_USER_CTX* ctx)
{
	int index;

	index = threadpool_getindex();
	if(index>=0) {
		if(userctx_thread[index].index==ctx->obj_id.index) {
 			if(userctx_thread[index].ref_count>1) {
				userctx_thread[index].ref_count--;
				return;
			} else {
				userctx_thread[index].index = -1;
			}
		}
	}
	if(ctx->state==0) {
		fes_userctx_destroy(ctx);
	}
	os_mutex_unlock(&userctx_list[ctx->obj_id.index].mutex);
}

void* fes_userctx_getfilter(FES_USER_CTX* ctx, int index)
{
	if(index<0 || index>=ctx->filter_count) return NULL;
	return ctx->filter_list[index].filter;
}

void fes_userctx_enable_filter(FES_USER_CTX* ctx, int index)
{
	unsigned int mask = 1;
	if(index<0 || index>=ctx->filter_count) return;
	mask = mask << index;
	ctx->state |= mask;
}

void fes_userctx_disable_filter(FES_USER_CTX* ctx, int index)
{
	unsigned int mask = 1;
	if(index<0 || index>=ctx->filter_count) return;
	mask = (mask << index) ^ 0xffffffffl;
	ctx->state &= mask;
}

