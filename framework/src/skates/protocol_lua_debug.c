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
#include "../../inc/skates/protocol.h"
#include "../../inc/skates/protocol_lua.h"
#include "../../inc/skates/protocol_lua_debug.h"
#include "../../inc/skates/threadpool.h"
#include "protocol_lua.h"

typedef struct PROTOCOL_LUA_STATE{
	unsigned int	sid;
	char			name[100];
	lua_State*		L;

	RPCNET_GROUP*	host;
	unsigned int	cid;

	RPCNET_GROUP*	debug_host;
	unsigned int	debug_cid;
	lua_State*		dbgL;
} PROTOCOL_LUA_STATE;

static PROTOCOL_LUA_STATE state_list[20];
static unsigned int state_seq = 0x2008;
static os_mutex_t state_mutex;

static PROTOCOL_LUA_STATE* state_add(const char* name, lua_State* L);
static PROTOCOL_LUA_STATE* state_get(unsigned int sid);
static void state_del(PROTOCOL_LUA_STATE* state);
static void* protocol_alloc(void *ud, void *ptr, size_t osize, size_t nsize);
static int protocol_panic(lua_State *L);

void protocol_lua_init()
{
	os_mutex_init(&state_mutex);
	memset(state_list, 0, sizeof(state_list));
	rpcfun_register(__protocol_lua_desc, 0);
}

void protocol_lua_final()
{
	int idx;

	rpcfun_unregister(__protocol_lua_desc, 0);

	for(idx=0; idx<sizeof(state_list)/sizeof(state_list[0]); idx++) {
		if(!state_list[idx].L) continue;
		SYSLOG(LOG_WARNING, MODULE_NAME, "protocol_lua_final: state(%s) not free", state_list[idx].name);
	}

	os_mutex_destroy(&state_mutex);
}

lua_State* protocol_lua_newstate(lua_CFunction panic, const char* name)
{
	lua_State* L;
	PROTOCOL_LUA_STATE* state;

	L = lua_newstate(protocol_alloc, NULL);
	if(!L) return NULL;

	lua_atpanic(L, panic?panic:protocol_panic);

	os_mutex_lock(&state_mutex);
	state = state_add(name, L);
	os_mutex_unlock(&state_mutex);
	if(!state) {
		lua_close(L);
		return NULL;
	}

	lua_pushstring(L, "PROTOCOL_LUA_ID");
	lua_pushinteger(L, (lua_Integer)(state->sid));
	lua_rawset(L, LUA_REGISTRYINDEX);

	return L;
}

void protocol_lua_closestate(lua_State* L)
{
	unsigned int sid;
	PROTOCOL_LUA_STATE* state;

	lua_pushstring(L, "PROTOCOL_LUA_ID");
	lua_rawget(L, LUA_REGISTRYINDEX);
	if(!lua_isnumber(L, -1)) {
		lua_pop(L, -1);
		return;
	}
	sid = lua_tointeger(L, -1);
	lua_pop(L, -1);


	assert((sid&0xffff)>=0 && (sid&0xffff)<sizeof(state_list)/sizeof(state_list[0]));
	assert(state_list[sid&0xffff].sid==sid);
	assert(state_list[sid&0xffff].L==L);

	os_mutex_lock(&state_mutex);
	state = state_get(sid);
	state_del(state);
	os_mutex_unlock(&state_mutex);

	lua_close(L);
}

void protocol_lua_debugbreak(lua_State* L)
{
	unsigned int sid;
	PROTOCOL_LUA_STATE* state;

	lua_pushstring(L, "PROTOCOL_LUA_ID");
	lua_rawget(L, LUA_REGISTRYINDEX);
	if(!lua_isnumber(L, -1)) {
		lua_pop(L, -1);
		return;
	}

	sid = lua_tointeger(L, -1);
	lua_pop(L, -1);

	assert((sid&0xffff)<sizeof(state_list)/sizeof(state_list[0]));
	assert(state_list[sid&0xffff].sid==sid);

	os_mutex_lock(&state_mutex);
	state = state_get(sid);
	state_list[sid].debug_host = state_list[sid].host;
	state_list[sid].debug_cid = state_list[sid].cid;
	state_list[sid].dbgL = L;
	os_mutex_unlock(&state_mutex);

	if(state_list[sid].debug_host) {
		threadpool_s();
		LuaDebugClientRpc_BreakPoint(state_list[sid].debug_host, state_list[sid].cid);
		threadpool_e();
		state_list[sid].debug_host = NULL;
	}
}

void protocol_lua_debugmsg(lua_State* L, int type, const char* msg)
{
	unsigned int sid;
	PROTOCOL_LUA_STATE* state;

	lua_pushstring(L, "PROTOCOL_LUA_ID");
	lua_rawget(L, LUA_REGISTRYINDEX);
	if(!lua_isnumber(L, -1)) {
		lua_pop(L, -1);
		return;
	}
	sid = lua_tointeger(L, -1);
	lua_pop(L, -1);

	assert((sid&0xffff)<sizeof(state_list)/sizeof(state_list[0]));
	assert(state_list[sid&0xffff].sid==sid);

	os_mutex_lock(&state_mutex);
	state = state_get(sid);
	state_list[sid].debug_host = state_list[sid].host;
	state_list[sid].debug_cid = state_list[sid].cid;
	os_mutex_unlock(&state_mutex);

	if(state_list[sid].debug_host) {
		threadpool_s();
		LuaDebugClientRpc_DebugMsg(state_list[sid].debug_host, state_list[sid].debug_cid, type, msg);
		threadpool_e();
		state_list[sid].debug_host = NULL;
	}

}

int LuaDebugHostRpc_Attach_impl(RPCNET_GROUP* group, os_dword sid, os_dword cid)
{
	PROTOCOL_LUA_STATE* state;
	int ret;
	RPCNET_GROUP* thost;
	unsigned int tcid;

	os_mutex_lock(&state_mutex);
	state = state_get(sid);
	if(!state) {
		os_mutex_unlock(&state_mutex);
		return ERR_NOT_FOUND;
	}
	thost = state->host;
	tcid = state->cid;
	state->host = group;
	state->cid = cid;
	os_mutex_unlock(&state_mutex);

	if(thost) {
		LuaDebugClientRpc_Detach(thost, tcid, state->sid);
	}

	ret = LuaDebugClientRpc_Attach(group, cid, sid);
	if(ret!=ERR_NOERROR) {
		state->host = NULL;
		state->cid = 0;
	}

	return ret;
}

int LuaDebugHostRpc_Detach_impl(RPCNET_GROUP* group, os_dword sid, os_dword cid)
{
	PROTOCOL_LUA_STATE* state;
	RPCNET_GROUP* thost;
	unsigned int tcid;

	os_mutex_lock(&state_mutex);
	state = state_get(sid);
	if(!state) {
		os_mutex_unlock(&state_mutex);
		return ERR_NOT_FOUND;
	}
	thost = state->host;
	tcid = state->cid;
	if(thost==group && tcid==cid) {
		state->host = NULL;
		state->cid = 0;
	}
	os_mutex_unlock(&state_mutex);

	if(thost!=group || tcid!=cid) {
		return ERR_NOT_FOUND;
	}

	return LuaDebugClientRpc_Detach(group, cid, sid);
}

int LuaDebugHostRpc_GetStateList_impl(RPCNET_GROUP* group, LUADEBUG_STATEINFO* infos, int* count)
{
	int i, m;

	if(*count>sizeof(state_list)/sizeof(state_list[0])) {
		*count = sizeof(state_list)/sizeof(state_list[0]);
	}

	os_mutex_lock(&state_mutex);
	for(m=i=0; i<*count; i++) {
		RPCNET_GROUP* grp;

		if(!state_list[i].L) continue;
		grp = state_list[i].host;

		infos[m].sid = state_list[i].sid;
		strcpy(infos[m].name, state_list[i].name);
		if(grp) {
			SOCK_ADDR sa;
			sock_addr2str(rpcnet_group_get_endpoint(grp, &sa), infos[m].client_ep);
		} else {
			infos[m].client_ep[0] = '\0';
		}
		m++;
	}
	os_mutex_unlock(&state_mutex);

	*count = m;
	return ERR_NOERROR;
}

int LuaDebugHostRpc_GetCallStack_impl(RPCNET_GROUP* group, os_dword sid, LUADEBUG_CALLSTACK* stacks, int* depth)
{
	PROTOCOL_LUA_STATE* state;
	RPCNET_GROUP* host;
	lua_State* L;
	int i;
	lua_Debug ar;

	os_mutex_lock(&state_mutex);
	state = state_get(sid);
	if(!state) {
		os_mutex_unlock(&state_mutex);
		return ERR_NOT_FOUND;
	}
	host = state->debug_host;
	L = state->dbgL;
	os_mutex_unlock(&state_mutex);

	if(host!=group || !state->dbgL) return ERR_OPT_FORBIDDEN;

	for(i=1; i<*depth; i++) {
		memset(&ar, 0, sizeof(ar));
		if(!lua_getstack(state->dbgL, i, &ar)) break;
		if(!lua_getinfo(state->dbgL, "flnSu", &ar)) {
			assert(0);
			break;
		}

		if(ar.name) strcpy(stacks[i-1].name, ar.name);				else strcpy(stacks[i-1].name, "");
		if(ar.namewhat) strcpy(stacks[i-1].namewhat, ar.namewhat);	else strcpy(stacks[i-1].namewhat, "");
		if(ar.what) strcpy(stacks[i-1].what, ar.what);				else strcpy(stacks[i-1].what, "");
		if(ar.source) strcpy(stacks[i-1].source, ar.source);		else strcpy(stacks[i-1].source, "");
		stacks[i-1].currentline = ar.currentline;
		stacks[i-1].nups = ar.nups;
		stacks[i-1].linedefined = ar.linedefined;
		stacks[i-1].lastlinedefined = ar.lastlinedefined;
		strcpy(stacks[i-1].short_src, ar.short_src);
	}

	*depth = i - 1;
	return ERR_NOERROR;
}

int LuaDebugHostRpc_RunCmd_impl(RPCNET_GROUP* group, os_dword sid, const char* Cmd)
{
	PROTOCOL_LUA_STATE* state;
	int ret;

	state = state_get(sid);
	if(!state) return ERR_NOT_FOUND;
	if(!state->dbgL) return ERR_OPT_FORBIDDEN;

	ret = luaL_dostring(state->dbgL, Cmd);
	if(!ret) {
		lua_pop(state->dbgL, 1);
		return ERR_UNKNOWN;
	} else {
		return ERR_NOERROR;
	}
}

PROTOCOL_LUA_STATE* state_add(const char* name, lua_State* L)
{
	unsigned int idx;

	for(idx=0; idx<sizeof(state_list)/sizeof(state_list[0]); idx++) {
		if(!state_list[idx].L) break;
	}
	if(idx==sizeof(state_list)/sizeof(state_list[0])) return NULL;

	state_list[idx].sid		= (((state_seq++)&0x7fff)<<16) | idx;
	state_list[idx].L		= L;
	strcpy(state_list[idx].name, name);
	state_list[idx].host	= NULL;
	state_list[idx].cid		= 0;
	state_list[idx].debug_host	= NULL;
	state_list[idx].debug_cid	= 0;
	state_list[idx].dbgL		= NULL;

	return &state_list[idx];
}

PROTOCOL_LUA_STATE* state_get(unsigned int sid)
{
	unsigned int idx;
	idx = sid & 0xffff;
	if(idx>=sizeof(state_list)/sizeof(state_list[0])) return NULL;
	if(state_list[idx].sid!=sid) return NULL;
	return &state_list[idx];
}

void state_del(PROTOCOL_LUA_STATE* state)
{
	memset(state, 0, sizeof(*state));
}

void* protocol_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	(void)ud;
	(void)osize;
	if(nsize==0) {
		dymempool_free(ptr);
		return NULL;
	} else
		return dymempool_realloc(ptr, nsize);
}

int protocol_panic(lua_State *L)
{
	(void)L;  /* to avoid warnings */
	fprintf(stderr, "PANIC: unprotected error in call to Lua API (%s)\n", lua_tostring(L, -1));
	return 0;
}
