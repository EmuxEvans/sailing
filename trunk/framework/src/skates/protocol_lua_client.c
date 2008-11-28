#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/dymempool.h"
#include "../../inc/skates/applog.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/sock.h"
#include "../../inc/skates/stream.h"
#include "../../inc/skates/hashmap.h"
#include "../../inc/skates/rpc_net.h"
#include "../../inc/skates/rpc_fun.h"
#include "../../inc/skates/protocol_def.h"
#include "../../inc/skates/protocol_lua.h"
#include "../../inc/skates/protocol_lua_debug.h"
#include "../../inc/skates/protocol_lua_client.h"
#include "../../inc/skates/protocol.h"
#include "../../inc/skates/threadpool.h"
#include "protocol_lua.h"

struct PROTOCOL_LUA_CLIENT {
	int refcount;
	unsigned int cid;

	RPCNET_GROUP* host;
	unsigned int sid;

	PROTOCOL_LUA_DEBUG_CALLBACK callback;

	int tid;
};

static unsigned int client_seq = 0x2000;
static PROTOCOL_LUA_CLIENT client_list[100];
static os_mutex_t client_mutex;

static PROTOCOL_LUA_CLIENT* client_add(RPCNET_GROUP* host, unsigned int sid);
static PROTOCOL_LUA_CLIENT* client_get(unsigned int cid);
static void client_del(PROTOCOL_LUA_CLIENT* clt);

void protocol_luaclt_init()
{
	memset(client_list, 0, sizeof(client_list));
	os_mutex_init(&client_mutex);
}

void protocol_luaclt_final()
{
	int idx;

	for(idx=0; idx<sizeof(client_list)/sizeof(client_list[0]); idx++) {
		if(!client_list[idx].host) continue;
		SYSLOG(LOG_WARNING, MODULE_NAME, "protocol_lua_final: client(%d) not detach", idx);
	}

	os_mutex_destroy(&client_mutex);
}

void luaclt_del(PROTOCOL_LUA_CLIENT* client)
{
	for(;;) {
		int refcount;
		assert(client->refcount>0);

		os_mutex_lock(&client_mutex);
		refcount = client->refcount;
		if(refcount==1) {
			client_del(client);
		}
		os_mutex_unlock(&client_mutex);

		if(refcount==1) break;

		os_sleep(10);
	}
}

int protocol_luaclt_state(RPCNET_GROUP* host, LUADEBUG_STATEINFO* infos, int* count)
{
	int ret;

	threadpool_s();
	ret = LuaDebugHostRpc_GetStateList(host, infos, count);
	threadpool_e();

	return ret;
}

PROTOCOL_LUA_CLIENT* protocol_luaclt_attach(RPCNET_GROUP* grp, unsigned int sid, PROTOCOL_LUA_DEBUG_CALLBACK* callback)
{
	PROTOCOL_LUA_CLIENT* clt;
	int ret;

	if(grp==NULL) return NULL;

	threadpool_s();

	os_mutex_lock(&client_mutex);
	clt = client_add(grp, sid);
	if(clt) {
		memcpy(&clt->callback, callback, sizeof(*callback));
		clt->tid = threadpool_getindex();
		clt->refcount++;
	}
	os_mutex_unlock(&client_mutex);

	if(clt) {
		ret = LuaDebugHostRpc_Attach(grp, sid, clt->cid);

		os_mutex_lock(&client_mutex);
		clt->tid = -1;
		clt->refcount--;
		os_mutex_unlock(&client_mutex);
	} else {
		ret = ERR_NOT_FOUND;
	}

	threadpool_e();

	if(ret!=ERR_NOERROR) {
		luaclt_del(clt);
		return NULL;
	}

	return clt;
}

void protocol_luaclt_detach(PROTOCOL_LUA_CLIENT* client)
{
	int ret;

	if(!client) return;
	os_mutex_lock(&client_mutex);
	client = client_get(client->cid);
	if(client) {
		client->refcount++;
	}
	os_mutex_unlock(&client_mutex);
	if(!client) return;

	threadpool_s();
	ret = LuaDebugHostRpc_Detach(client->host, client->sid, client->cid);
	threadpool_e();

	os_mutex_lock(&client_mutex);
	client->refcount--;
	os_mutex_unlock(&client_mutex);

	luaclt_del(client);
}

void* protocol_luaclt_userptr(PROTOCOL_LUA_CLIENT* client)
{
	return client->callback.userptr;
}

int protocol_luaclt_runcmd(PROTOCOL_LUA_CLIENT* client, const char* cmd)
{
	int ret;

	if(!client) return ERR_INVALID_PARAMETER;
	os_mutex_lock(&client_mutex);
	client = client_get(client->cid);
	if(client) {
		client->refcount++;
	}
	os_mutex_unlock(&client_mutex);
	if(!client) return ERR_OPT_FORBIDDEN;

	threadpool_s();
	ret = LuaDebugHostRpc_RunCmd(client->host, client->sid, cmd);
	threadpool_e();

	os_mutex_lock(&client_mutex);
	client->refcount--;
	os_mutex_unlock(&client_mutex);

	return ret;
}

int protocol_luaclt_getstack(PROTOCOL_LUA_CLIENT* client, LUADEBUG_CALLSTACK* stack, int* count)
{
	int ret;

	if(!client) return ERR_INVALID_PARAMETER;
	os_mutex_lock(&client_mutex);
	client = client_get(client->cid);
	if(client) {
		client->refcount++;
	}
	os_mutex_unlock(&client_mutex);
	if(!client) return ERR_OPT_FORBIDDEN;

	threadpool_s();
	ret = LuaDebugHostRpc_GetCallStack(client->host, client->sid, stack, count);
	threadpool_e();

	os_mutex_lock(&client_mutex);
	client->refcount--;
	os_mutex_unlock(&client_mutex);

	return ret;
}

int LuaDebugClientRpc_Attach_impl(RPCNET_GROUP* group, os_dword cid, os_dword sid)
{
	PROTOCOL_LUA_CLIENT* clt;

	clt = client_get(cid);
	if(!clt || clt->tid!=threadpool_getindex()) {
		return ERR_NOT_FOUND;
	}

	clt->callback.attach(clt);

	return ERR_NOERROR;
}

int LuaDebugClientRpc_Detach_impl(RPCNET_GROUP* group, os_dword cid, os_dword sid)
{
	PROTOCOL_LUA_CLIENT* clt;

	os_mutex_lock(&client_mutex);
	clt = client_get(cid);
	if(clt) {
		clt->refcount++;
	}
	os_mutex_unlock(&client_mutex);
	if(!clt) return ERR_NOT_FOUND;

	clt->callback.detach(clt);

	os_mutex_lock(&client_mutex);
	clt->refcount--;
	os_mutex_unlock(&client_mutex);

	return ERR_NOERROR;
}

int LuaDebugClientRpc_BreakPoint_impl(RPCNET_GROUP* group, os_dword cid)
{
	PROTOCOL_LUA_CLIENT* clt;

	os_mutex_lock(&client_mutex);
	clt = client_get(cid);
	if(clt) {
		clt->refcount++;
	}
	os_mutex_unlock(&client_mutex);
	if(!clt) return ERR_NOT_FOUND;

	clt->callback.debugbreak(clt);

	os_mutex_lock(&client_mutex);
	clt->refcount--;
	os_mutex_unlock(&client_mutex);

	return ERR_NOERROR;
}

int LuaDebugClientRpc_DebugMsg_impl(RPCNET_GROUP* group, os_dword cid, int Type, const char* Msg)
{
	PROTOCOL_LUA_CLIENT* clt;

	os_mutex_lock(&client_mutex);
	clt = client_get(cid);
	if(clt) {
		clt->refcount++;
	}
	os_mutex_unlock(&client_mutex);
	if(!clt) return ERR_NOT_FOUND;

	clt->callback.debugmsg(clt, Type, Msg);

	os_mutex_lock(&client_mutex);
	clt->refcount--;
	os_mutex_unlock(&client_mutex);

	return ERR_NOERROR;
}

PROTOCOL_LUA_CLIENT* client_add(RPCNET_GROUP* host, unsigned int sid)
{
	int idx, fidx=-1;

	for(idx=0; idx<sizeof(client_list)/sizeof(client_list[0]); idx++) {
		if(client_list[idx].host==NULL) {
			fidx = idx;
			continue;
		}
		if(client_list[idx].host==host && client_list[idx].sid==sid) {
			return NULL;
		}
	}
	if(fidx<0) return NULL;

	client_list[fidx].refcount = 1;
	client_list[fidx].cid = (((client_seq++)&0x7fff)<<16) | ((unsigned int)fidx);
	client_list[fidx].host = host;
	client_list[fidx].sid = sid;
	client_list[idx].tid = -1;

	return &client_list[fidx];
}

PROTOCOL_LUA_CLIENT* client_get(unsigned int cid)
{
	unsigned int idx;

	idx = cid&0xffff;
	if(idx>=sizeof(client_list)/sizeof(client_list[0])) return NULL;
	if(client_list[idx].cid!=cid || client_list[idx].host==NULL) return NULL;

	return &client_list[idx];
}

void client_del(PROTOCOL_LUA_CLIENT* clt)
{
	memset(clt, 0, sizeof(*clt));
}
