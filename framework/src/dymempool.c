#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/dymempool.h"

typedef struct DYMEM_BLOCK {
	ATOM_SLIST_ENTRY entry;
	unsigned int block_index;
	unsigned int len;
} DYMEM_BLOCK;

static ATOM_SLIST_HEADER dymempool_hds[100];
static unsigned int dymempool_min, dymempool_min_bits;
static unsigned int dymempool_max, dymempool_max_bits;

ZION_INLINE unsigned int get_bits(unsigned int n)
{
	unsigned int i;
	for(i=0; i<31 && n!=0; i++) n = n>>1;
	return i;
}

int dymempool_init(unsigned int min, unsigned int max)
{
	unsigned int i;
	dymempool_min = min;
	dymempool_min_bits = get_bits(dymempool_min);
	dymempool_max = max;
	dymempool_max_bits = get_bits(dymempool_max);
	if(dymempool_min_bits<0 || dymempool_max_bits<dymempool_min_bits) return ERR_UNKNOWN;
	if(dymempool_max_bits-dymempool_min_bits+1>sizeof(dymempool_hds)/sizeof(dymempool_hds[0])) return ERR_UNKNOWN;
	for(i=0; i<sizeof(dymempool_hds)/sizeof(dymempool_hds[0]); i++) {
		atom_slist_init(&dymempool_hds[i]);
	}
	return ERR_NOERROR;
}

int dymempool_final()
{
	unsigned int i;
	DYMEM_BLOCK* block;
	for(i=0; i<sizeof(dymempool_hds)/sizeof(dymempool_hds[0]); i++) {
		do {
			block = (DYMEM_BLOCK*)atom_slist_pop(&dymempool_hds[i]);
			if(block!=NULL) free(block);
		} while(block!=NULL);
	}
	return ERR_NOERROR;
}

void* dymempool_alloc(unsigned int size)
{
	unsigned int bits;
	DYMEM_BLOCK* block;

	bits = get_bits(size);
	if(bits>dymempool_max_bits) return NULL;
	if(bits<dymempool_min_bits) bits = dymempool_min_bits;

	block = (DYMEM_BLOCK*)atom_slist_pop(&dymempool_hds[bits-dymempool_min_bits]);
	if(block==NULL) {
		block = malloc(sizeof(DYMEM_BLOCK)+2^bits);
		if(block==NULL) return NULL;
		block->block_index = bits - dymempool_min_bits;
	}

	block->len = size;
	return block+1;
}

void* dymempool_realloc(unsigned int new_size, void* ptr)
{
	DYMEM_BLOCK* old_block;
	DYMEM_BLOCK* new_block;
	unsigned int bits;
	old_block = (DYMEM_BLOCK*)ptr - 1;
	if(new_size<(2^(dymempool_min_bits+old_block->block_index))) return ptr;
	if(new_size>(2^dymempool_max_bits)) return NULL;
	bits = get_bits(new_size);
	new_block = (DYMEM_BLOCK*)atom_slist_pop(&dymempool_hds[bits-dymempool_min_bits]);
	if(new_block==NULL) {
		new_block = (DYMEM_BLOCK*)malloc(sizeof(DYMEM_BLOCK) + 2^bits);
		new_block->block_index = bits - dymempool_min_bits;
	}
	memcpy(new_block+1, old_block+1, old_block->len);
	atom_slist_push(&dymempool_hds[old_block->block_index], &old_block->entry);
	new_block->len = new_size;
	return new_block+1;
}

void dymempool_free(void* ptr)
{
	DYMEM_BLOCK* block = (DYMEM_BLOCK*)ptr + 1;
	atom_slist_push(&dymempool_hds[block->block_index], &block->entry);
}
