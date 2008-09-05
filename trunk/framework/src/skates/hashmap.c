#include <stdlib.h>
#include <string.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/hashmap.h"

struct HASHMAP {
	unsigned int			mask;
	unsigned int			flag;
	HASHMAP_HASHFUNCTION	hashfunc;

	union {
		struct {
			os_mutex_t		mutex;
			RLIST_HEAD		head;
		} array[0];

		RLIST_HEAD			heads[0];
	};
};

#define GET_ENTRY(map, hashvalue)	((map->flag&HASHMAP_FLAG_THREADSAFE)!=0?&map->array[hashvalue].head:&map->heads[hashvalue])

static unsigned int default_hashfunc(const void* char_key, unsigned int klen)
{
    unsigned int hash = 0;
    const unsigned char *key = (const unsigned char *)char_key;
    const unsigned char *p;
    unsigned int i;
    
    /*
     * This is the popular `times 33' hash algorithm which is used by
     * perl and also appears in Berkeley DB. This is one of the best
     * known hash functions for strings because it is both computed
     * very fast and distributes very well.
     *
     * The originator may be Dan Bernstein but the code in Berkeley DB
     * cites Chris Torek as the source. The best citation I have found
     * is "Chris Torek, Hash function for text in C, Usenet message
     * <27038@mimsy.umd.edu> in comp.lang.c , October, 1990." in Rich
     * Salz's USENIX 1992 paper about INN which can be found at
     * <http://citeseer.nj.nec.com/salz92internetnews.html>.
     *
     * The magic of number 33, i.e. why it works better than many other
     * constants, prime or not, has never been adequately explained by
     * anyone. So I try an explanation: if one experimentally tests all
     * multipliers between 1 and 256 (as I did while writing a low-level
     * data structure library some time ago) one detects that even
     * numbers are not useable at all. The remaining 128 odd numbers
     * (except for the number 1) work more or less all equally well.
     * They all distribute in an acceptable way and this way fill a hash
     * table with an average percent of approx. 86%.
     *
     * If one compares the chi^2 values of the variants (see
     * Bob Jenkins ``Hashing Frequently Asked Questions'' at
     * http://burtleburtle.net/bob/hash/hashfaq.html for a description
     * of chi^2), the number 33 not even has the best value. But the
     * number 33 and a few other equally good numbers like 17, 31, 63,
     * 127 and 129 have nevertheless a great advantage to the remaining
     * numbers in the large set of possible multipliers: their multiply
     * operation can be replaced by a faster operation based on just one
     * shift plus either a single addition or subtraction operation. And
     * because a hash function has to both distribute good _and_ has to
     * be very fast to compute, those few numbers should be preferred.
     *
     *                  -- Ralf S. Engelschall <rse@engelschall.com>
     */
     
    for (p = key, i = klen; i; i--, p++) {
        hash = hash * 33 + *p;
    }

    return hash;
}

HASHMAP_HANDLE hashmap_create(int bits, HASHMAP_HASHFUNCTION hashfunc, int flag)
{
	HASHMAP* map;
	unsigned int l, size;
	if(bits<1 || bits>16) return NULL;
	size = 1<<bits;
	if(flag&HASHMAP_FLAG_THREADSAFE) {
		map = (HASHMAP*)malloc(sizeof(HASHMAP)+sizeof(map->array[0])*size);
	} else {
		map = (HASHMAP*)malloc(sizeof(HASHMAP)+sizeof(map->heads[0])*size);
	}
	if(map==NULL) return NULL;
	map->mask = size-1;
	map->flag = flag;
	map->hashfunc = hashfunc!=NULL?hashfunc:default_hashfunc;
	for(l=0; l<size; l++) {
		if(map->flag&HASHMAP_FLAG_THREADSAFE) {
			rlist_init(&map->array[l].head);
			os_mutex_init(&map->array[l].mutex);
		} else {
			rlist_init(&map->heads[l]);
		}
	}
	return(map);
}

void hashmap_destroy(HASHMAP_HANDLE map)
{
	unsigned int l;
	if(map->flag&HASHMAP_FLAG_THREADSAFE)
		for(l=0; l<=map->mask; l++)
			os_mutex_destroy(&map->array[l].mutex);
	free(map);
}

static HASHMAP_ITEM* get_item(RLIST_HEAD* list, const void* key, int klen)
{
	HASHMAP_ITEM* item;
	for(item=(HASHMAP_ITEM*)rlist_front(list); !rlist_is_head(list, (RLIST_ITEM*)item); item=(HASHMAP_ITEM*)rlist_next((RLIST_ITEM*)item)) {
		if(item->klen==klen && memcmp(item->key, key, klen)==0) return(item);
	}
	return(NULL);
}

void hashmap_add_unsafe(HASHMAP_HANDLE map, HASHMAP_ITEM* item, const void* key, unsigned int klen)
{
	unsigned int hashvalue;

	hashvalue = map->hashfunc(key, klen) & map->mask;

	if(map->flag&HASHMAP_FLAG_THREADSAFE)
	os_mutex_lock(&map->array[hashvalue].mutex);

	rlist_push_front(GET_ENTRY(map, hashvalue), (RLIST_ITEM*)item);
	item->key	= key;
	item->klen	= klen;

	if(map->flag&HASHMAP_FLAG_THREADSAFE)
	os_mutex_unlock(&map->array[hashvalue].mutex);
}

HASHMAP_ITEM* hashmap_add(HASHMAP_HANDLE map, HASHMAP_ITEM* item, const void* key, unsigned int klen)
{
	HASHMAP_ITEM* ret;
	unsigned int hashvalue;

	hashvalue = map->hashfunc(key, klen) & map->mask;

	if(map->flag&HASHMAP_FLAG_THREADSAFE)
	os_mutex_lock(&map->array[hashvalue].mutex);

	ret = get_item(GET_ENTRY(map, hashvalue), key, klen);
	if(ret==NULL) {
		rlist_push_front(GET_ENTRY(map, hashvalue), (RLIST_ITEM*)item);
		item->key	= key;
		item->klen	= klen;
		ret = item;
	}

	if(map->flag&HASHMAP_FLAG_THREADSAFE)
	os_mutex_unlock(&map->array[hashvalue].mutex);

	return(ret);
}

HASHMAP_ITEM* hashmap_get(HASHMAP_HANDLE map, const void* key, unsigned int klen)
{
	HASHMAP_ITEM* item;
	unsigned int hashvalue;

	hashvalue = map->hashfunc(key, klen) & map->mask;

	if(map->flag&HASHMAP_FLAG_THREADSAFE)
	os_mutex_lock(&map->array[hashvalue].mutex);

	item = get_item(GET_ENTRY(map, hashvalue), key, klen);

	if(map->flag&HASHMAP_FLAG_THREADSAFE)
	os_mutex_unlock(&map->array[hashvalue].mutex);

	return(item);
}

void hashmap_erase(HASHMAP_HANDLE map, HASHMAP_ITEM* item)
{
	if(item==NULL) return;

	if(map->flag&HASHMAP_FLAG_THREADSAFE)
	os_mutex_lock(&map->array[item->hashvalue].mutex);

	rlist_remove(GET_ENTRY(map, item->hashvalue), (RLIST_ITEM*)item);

	if(map->flag&HASHMAP_FLAG_THREADSAFE)
	os_mutex_unlock(&map->array[item->hashvalue].mutex);
}

HASHMAP_ITEM* hashmap_erase_bykey(HASHMAP_HANDLE map, const void* key, unsigned int klen)
{
	HASHMAP_ITEM* item;
	unsigned int hashvalue;

	hashvalue = map->hashfunc(key, klen) & map->mask;

	if(map->flag&HASHMAP_FLAG_THREADSAFE)
	os_mutex_lock(&map->array[hashvalue].mutex);

	item = get_item(GET_ENTRY(map, hashvalue), key, klen);
	if(item!=NULL) rlist_remove(GET_ENTRY(map, hashvalue), (RLIST_ITEM*)item);

	if(map->flag&HASHMAP_FLAG_THREADSAFE)
	os_mutex_unlock(&map->array[hashvalue].mutex);

	return(item);
}
