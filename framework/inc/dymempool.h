#ifndef _DYMEMPOOL_
#define _DYMEMPOOL_

int dymempool_init(unsigned int min, unsigned int max);
int dymempool_final();

void* dymempool_alloc(unsigned int size);
void* dymempool_realloc(unsigned int new_size, void* ptr);
void dymempool_free(void* ptr);

#endif
