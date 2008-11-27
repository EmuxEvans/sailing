#ifndef _DYMEMPOOL_
#define _DYMEMPOOL_

ZION_API int dymempool_init(unsigned int min, unsigned int max);
ZION_API int dymempool_final();

ZION_API void* dymempool_alloc(unsigned int size);
ZION_API void* dymempool_realloc(void* ptr, unsigned int new_size);
ZION_API void dymempool_free(void* ptr);

#endif
