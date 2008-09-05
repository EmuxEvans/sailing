#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#ifdef __cplusplus
extern "C" {
#endif

struct HASHMAP;
typedef struct HASHMAP		HASHMAP;
typedef struct HASHMAP*		HASHMAP_HANDLE;

typedef struct HASHMAP_ITEM {
	RLIST_ITEM		item;
	const void*		key;
	unsigned int	klen;
	unsigned int	hashvalue;
} HASHMAP_ITEM;

typedef unsigned int (*HASHMAP_HASHFUNCTION)(const void* key, unsigned int klen);

#define HASHMAP_ITEM_SETPTR(kitem, ptr)	rlist_set_userdata(&(kitem)->item, ptr)
#define HASHMAP_ITEM_GETPTR(kitem)		rlist_get_userdata(&(kitem)->item)

#define HASHMAP_FLAG_THREADSAFE			1

HASHMAP_HANDLE hashmap_create(int bits, HASHMAP_HASHFUNCTION hashfunc, int flag);
void hashmap_destroy(HASHMAP_HANDLE map);

void hashmap_add_unsafe(HASHMAP_HANDLE map, HASHMAP_ITEM* item, const void* key, unsigned int klen);
HASHMAP_ITEM* hashmap_add(HASHMAP_HANDLE map, HASHMAP_ITEM* item, const void* key, unsigned int klen);
HASHMAP_ITEM* hashmap_get(HASHMAP_HANDLE map, const void* key, unsigned int klen);
void hashmap_erase(HASHMAP_HANDLE map, HASHMAP_ITEM* item);
HASHMAP_ITEM* hashmap_erase_bykey(HASHMAP_HANDLE map, const void* key, unsigned int klen);

#ifdef __cplusplus
}
#endif

#endif
