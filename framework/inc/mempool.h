#ifndef _MEMPOOL_INCLUDE_
#define _MEMPOOL_INCLUDE_

#ifdef _DEBUG
#	define MEMPOOL_DEBUG
#endif
#ifdef MEMPOOL_DEBUG_OFF
#	undef MEMPOOL_DEBUG
#endif
#ifdef MEMPOOL_DEBUG_ON
#	define MEMPOOL_DEBUG
#endif

struct MEMPOOL_CB;
typedef struct MEMPOOL_CB		MEMPOOL_CB;
typedef struct MEMPOOL_CB*		MEMPOOL_HANDLE;

void ZION_API mempool_init();
void ZION_API mempool_final();

#ifndef MEMPOOL_DEBUG
	#define mempool_create_debug(file, line, size, initcount)	mempool_create(size, initcount)
	#define mempool_alloc_debug(file, line, handle)				mempool_alloc(handle)
	#define mempool_free_debug(file, line, handle, ptr)			mempool_free(handle, ptr)

	MEMPOOL_HANDLE ZION_API mempool_create(unsigned int size, unsigned int initcount);
	ZION_API void* mempool_alloc(MEMPOOL_HANDLE handle);
	ZION_API void mempool_free(MEMPOOL_HANDLE handle, void* ptr);
	ZION_API void mempool_destroy(MEMPOOL_HANDLE handle);
#else
	#define mempool_create(size, initcount)		mempool_create_debug(__FILE__, __LINE__, size, initcount)
	#define mempool_alloc(handle)				mempool_alloc_debug(__FILE__, __LINE__, handle)
	#define mempool_free(handle, ptr)			mempool_free_debug(__FILE__, __LINE__, handle, ptr)

	ZION_API MEMPOOL_HANDLE mempool_create_debug(const char* filename, unsigned int lineno, unsigned int size, unsigned int initcount);
	ZION_API void* mempool_alloc_debug(const char* filename, unsigned int lineno, MEMPOOL_HANDLE handle);
	ZION_API void mempool_free_debug(const char* filename, unsigned int lineno, MEMPOOL_HANDLE handle, void* ptr);
	ZION_API void mempool_destroy(MEMPOOL_HANDLE handle);
#endif


#endif

