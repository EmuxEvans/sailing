#ifndef _FES_USERCTX_H_
#define _FES_USERCTX_H_

#define FES_USERCTX_MAXCOUNT			3000

struct FES_USER_CTX;
typedef struct FES_USER_CTX FES_USER_CTX;

typedef struct FES_FILTER_DESC {
	unsigned int size;
	void (*init)(FES_USER_CTX* ctx, void* filter);
	void (*disconnect)(FES_USER_CTX* ctx, void* filter);
	void (*destroy)(FES_USER_CTX* ctx, void* filter);
} FES_FILTER_DESC;

void fes_userctx_init();
void fes_userctx_final();

int fes_userctx_calcsize(FES_FILTER_DESC descs[], int count);

FES_USER_CTX* fes_userctx_create(MEMPOOL_HANDLE mempool, FES_FILTER_DESC descs[], int count);
void fes_userctx_destroy(FES_USER_CTX* ctx);
void fes_userctx_disconnect(FES_USER_CTX* ctx);

void fes_userctx_get_objid(FES_USER_CTX* ctx, OBJECT_ID* id);
FES_USER_CTX* fes_userctx_get_byid(const OBJECT_ID* id);
FES_USER_CTX* fes_userctx_get_byindex(const int index);
void fes_userctx_release(FES_USER_CTX* ctx);

void* fes_userctx_getfilter(FES_USER_CTX* ctx, int index);
void fes_userctx_enable_filter(FES_USER_CTX* ctx, int index);
void fes_userctx_disable_filter(FES_USER_CTX* ctx, int index);

#endif

