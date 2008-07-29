#ifndef _PLAYER_H_
#define _PLAYER_H_

struct PLAYER_INSTANCE;
typedef struct PLAYER_INSTANCE PLAYER_INSTANCE;

typedef struct PLAYER_INFO_DESC {
	int size;
	int (*update)(PLAYER_INSTANCE* player, DBAPI_HANDLE handle, void* key);
	int (*get)(PLAYER_INSTANCE* player, DBAPI_HANDLE handle, void* key);
} PLAYER_INFO_DESC;

int player_calcsize(PLAYER_INFO_DESC descs[], int count);
int player_load(int dbid, const char* dbstr, PLAYER_INSTANCE* player, PLAYER_INFO_DESC descs[], int count, void* key);
int player_update(PLAYER_INSTANCE* player, void* key);

void* player_block_get(PLAYER_INSTANCE* player, int index);
void player_block_dirty(PLAYER_INSTANCE* player, int index);
int player_block_update(PLAYER_INSTANCE* player, int index, void* key);

int player_get_dbid(PLAYER_INSTANCE* player);
const char* player_get_dbstr(PLAYER_INSTANCE* player);

#endif

