#include <string.h>

#include "../inc/skates.h"
#include "../inc/octopus.h"

struct PLAYER_INSTANCE {
	PLAYER_INFO_DESC*	descs;
	int					count;
	int					dbid;
	const char*			dbstr;

	struct {
		void*			block;
		char			dirty;
	} block_data[0];
};

int player_calcsize(PLAYER_INFO_DESC descs[], int count)
{
	int l, ret;
	ret = sizeof(PLAYER_INSTANCE) + sizeof(((PLAYER_INSTANCE*)NULL)->block_data[0])*count;
	for(l=0; l<count; l++) {
		ret += descs[l].size;
	}
	return ret;
}

int player_load(int dbid, const char* dbstr, PLAYER_INSTANCE* player, PLAYER_INFO_DESC descs[], int count, void* key)
{
	int l, ret=ERR_NOERROR;
	char* cur;
	DBAPI_HANDLE conn;

	player->descs	= descs;
	player->count	= count;
	player->dbid	= dbid;
	player->dbstr	= dbstr;
	cur = (char*)player + sizeof(PLAYER_INSTANCE) + sizeof(player->block_data[0])*count;

	for(l=0; l<count; l++) {
		player->block_data[l].block = cur;
		player->block_data[l].dirty = 0;
		cur += player->descs[l].size;
	}
	
	conn=dbapi_connect(player->dbstr);
	if (conn==NULL) return ERR_UNKNOWN;

	for(l=0; l<count; l++) {
		ret = player->descs[l].get(player, conn, key);
		if(ret!=ERR_NOERROR) break;
	}
	
   	if (ret==ERR_NOERROR) {
		ret = dbapi_release(conn);
	} else {
		dbapi_release(conn);
	}
	return ret;
}

int player_update(PLAYER_INSTANCE* player, void* key)
{
	int l, ret;
	DBAPI_HANDLE conn;

	conn = dbapi_connect(player->dbstr);
	if(conn==NULL) return ERR_UNKNOWN;

	ret = dbapi_begin(conn);
	if(ret!=ERR_NOERROR) {
		dbapi_release(conn);
		return ret;
	}
	
	for(l=0; l<player->count; l++) {
		if(player->block_data[l].dirty==0) continue;
		if(player->descs[l].update==NULL) continue;
		ret = player->descs[l].update(player, conn, key);
		if(ret!=ERR_NOERROR) break;
	}

	if(ret==ERR_NOERROR) {
		ret = dbapi_commit(conn);
	} else {
		dbapi_rollback(conn);
	}

	if (ret==ERR_NOERROR) {
		ret = dbapi_release(conn);
	} else {
		dbapi_release(conn);
	}

	return ret;
}

void* player_block_get(PLAYER_INSTANCE* player, int index)
{
	if(index<0 || index>=player->count) return NULL;
	return player->block_data[index].block;
}

void player_block_dirty(PLAYER_INSTANCE* player, int index)
{
	if(index<0 || index>=player->count) return;
	player->block_data[index].dirty = 1;
}

int player_block_update(PLAYER_INSTANCE* player, int index, void* key)
{
	int ret;
	DBAPI_HANDLE conn;

	if(index<0 || index>=player->count) return ERR_UNKNOWN;
	if(player->block_data[index].dirty==0) return ERR_NOERROR;

	conn = dbapi_connect(player->dbstr);
	if(conn==NULL) return ERR_UNKNOWN;

	ret = dbapi_begin(conn);
	if(ret!=ERR_NOERROR) {
		dbapi_release(conn);
		return ret;
	}
	
	ret = player->descs[index].update(player, conn, key);
	if(ret==ERR_NOERROR) {
		ret = dbapi_commit(conn);
	} else {
		dbapi_rollback(conn);
	}
	if(ret==ERR_NOERROR) {
		ret = dbapi_release(conn);
	} else {
		dbapi_release(conn);
	}

	return ret;
}

int player_get_dbid(PLAYER_INSTANCE* player)
{
	return player->dbid;
}

const char* player_get_dbstr(PLAYER_INSTANCE* player)
{
	return player->dbstr;
}
