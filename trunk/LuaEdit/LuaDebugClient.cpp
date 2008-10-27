#include "StdAfx.h"
#include "LuaDebugClient.h"

#include <skates\skates.h>
#include "LuaHost\LuaDebugClientRpc.h"
#include "LuaHost\LuaDebugHostRpc.h"

class CLuaDebugClient;

static map<RPCNET_GROUP*, CLuaDebugClient*> client_map;
static os_mutex_t client_mtx;

class CLuaDebugClient : public ILuaDebugClient
{
public:
	CLuaDebugClient();
	virtual ~CLuaDebugClient();

	virtual BOOL Connect(LPCSTR pHostEP, ILuaDebugHooker* pHooker);
	virtual BOOL Disconnect();

	virtual BOOL RunCmd(LPCSTR pCmd);

	virtual BOOL IsRuning();
	virtual BOOL Continue();

	virtual void Release() {
		delete this;
	}

	void OnRPCBreakPoint();
	void OnRPCDebugMessage(int nType, LPCSTR pMsg);
	void OnAttched(RPCNET_GROUP* grp);
	void OnDetched(RPCNET_GROUP* grp);

protected:
	RPCNET_GROUP* m_pHost;
	ILuaDebugHooker* m_pHooker;
	HANDLE m_hBPEvent;
};

BOOL InitializeLuaDebugClient(const char* addr)
{
	SOCK_ADDR sa;

	if(sock_str2addr(addr, &sa)==NULL) {
		return FALSE;
	}

	os_mutex_init(&client_mtx);
	mempool_init();
	sock_init();
	fdwatch_init();
	threadpool_init(10);
	rpcnet_init();
	rpcfun_init();

	if(rpcnet_bind(&sa)==ERR_NOERROR) {
		rpcfun_register(__LuaDebugClientRpc_desc, 0);
		return TRUE;
	}

	rpcfun_final();
	rpcnet_final();
	threadpool_final();
	fdwatch_final();
	sock_final();
	mempool_final();
	os_mutex_destroy(&client_mtx);
	return FALSE;
}

BOOL FinalizeLuaDebugClient()
{
	rpcfun_unregister(__LuaDebugClientRpc_desc, 0);
	rpcnet_unbind();
	rpcfun_final();
	rpcnet_final();
	threadpool_final();
	fdwatch_final();
	sock_final();
	mempool_final();
	os_mutex_destroy(&client_mtx);
	return TRUE;
}

ILuaDebugClient* CreateLuaDebugClient()
{
	return new CLuaDebugClient;
}

CLuaDebugClient::CLuaDebugClient()
{
	m_pHost = NULL;
}

CLuaDebugClient::~CLuaDebugClient()
{
}

BOOL CLuaDebugClient::Connect(LPCSTR pHostEP, ILuaDebugHooker* pHooker)
{
	RPCNET_GROUP* pHost;
	int ret;
	SOCK_ADDR sa;

	if(sock_str2addr(pHostEP, &sa)==NULL)
		return FALSE;
	pHost = rpcnet_getgroup(&sa);
	if(pHost==NULL)
		return FALSE;

	threadpool_s();
	ret = LuaDebugHostRpc_Attach(pHost);
	threadpool_e();
	if(ret!=ERR_NOERROR)
		return FALSE;

	m_pHost = pHost;
	m_pHooker = pHooker;
	return TRUE;
}

BOOL CLuaDebugClient::Disconnect()
{
	threadpool_s();
	LuaDebugHostRpc_Detach(m_pHost);
	threadpool_e();
	m_pHost = NULL;
	return TRUE;
}

BOOL CLuaDebugClient::RunCmd(LPCSTR pCmd)
{
	return TRUE;
}

BOOL CLuaDebugClient::IsRuning()
{
	return TRUE;
}

BOOL CLuaDebugClient::Continue()
{
	return TRUE;
}

void CLuaDebugClient::OnRPCBreakPoint()
{
	m_pHooker->OnBreakPoint(this);
	WaitForSingleObject(m_hBPEvent, INFINITE);
}

void CLuaDebugClient::OnRPCDebugMessage(int nType, LPCSTR pMsg)
{
	m_pHooker->OnDebugMessage(this, nType, pMsg);
}

void CLuaDebugClient::OnAttched(RPCNET_GROUP* grp)
{
	assert(m_pHost==NULL);
	m_pHost = grp;
	m_pHooker->OnConnect(this);
}

void CLuaDebugClient::OnDetched(RPCNET_GROUP* grp)
{
	m_pHooker->OnDisconnect(this);
	assert(m_pHost==grp);
	m_pHost = NULL;
}

int LuaDebugClientRpc_Attach_impl(RPCNET_GROUP* group)
{
	CLuaDebugClient* pClient;
	map<RPCNET_GROUP*, CLuaDebugClient*>::iterator i;

	os_mutex_lock(&client_mtx);
	i = client_map.find(group);
	if(i==client_map.end()) {
		pClient = NULL;
	} else {
		pClient = i->second;
	}
	os_mutex_unlock(&client_mtx);

	if(pClient!=NULL) return ERR_UNKNOWN;
	pClient->OnAttched(group);
	return ERR_NOERROR;
}

int LuaDebugClientRpc_Detach_impl(RPCNET_GROUP* group)
{
	CLuaDebugClient* pClient;
	map<RPCNET_GROUP*, CLuaDebugClient*>::iterator i;

	os_mutex_lock(&client_mtx);
	i = client_map.find(group);
	if(i==client_map.end()) {
		pClient = NULL;
	} else {
		pClient = i->second;
	}
	os_mutex_unlock(&client_mtx);

	if(pClient==NULL) return ERR_UNKNOWN;
	pClient->OnDetched(group);
	return ERR_NOERROR;
}

int LuaDebugClientRpc_BreakPoint_impl(RPCNET_GROUP* group)
{
	CLuaDebugClient* pClient;
	map<RPCNET_GROUP*, CLuaDebugClient*>::iterator i;

	os_mutex_lock(&client_mtx);
	i = client_map.find(group);
	if(i==client_map.end()) {
		pClient = NULL;
	} else {
		pClient = i->second;
	}
	os_mutex_unlock(&client_mtx);

	if(pClient==NULL) return ERR_UNKNOWN;
	pClient->OnRPCBreakPoint();
	return ERR_NOERROR;
}

int LuaDebugClientRpc_DebugMsg_impl(RPCNET_GROUP* group, int Type, const char* Msg)
{
	CLuaDebugClient* pClient;
	map<RPCNET_GROUP*, CLuaDebugClient*>::iterator i;

	os_mutex_lock(&client_mtx);
	i = client_map.find(group);
	if(i==client_map.end()) {
		pClient = NULL;
	} else {
		pClient = i->second;
	}
	os_mutex_unlock(&client_mtx);

	if(pClient==NULL) return ERR_UNKNOWN;
	pClient->OnRPCDebugMessage(Type, Msg);
	return ERR_NOERROR;
}
