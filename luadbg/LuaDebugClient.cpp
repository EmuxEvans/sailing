#include "StdAfx.h"

#include <skates\skates.h>

#include "LuaDebugClient.h"

static void attach(PROTOCOL_LUA_CLIENT* pClient);
static void detach(PROTOCOL_LUA_CLIENT* pClient);
static void debugbreak(PROTOCOL_LUA_CLIENT* pClient);
static void debugmsg(PROTOCOL_LUA_CLIENT* pClient, int type, const char* msg);

class CLuaDebugClient : public ILuaDebugClient
{
public:
	CLuaDebugClient();
	virtual ~CLuaDebugClient();

	virtual BOOL Connect(LPCSTR pHostEP, unsigned int nSId, ILuaDebugHooker* pHooker);
	virtual BOOL Disconnect();

	virtual BOOL RunCmd(LPCSTR pCmd);
	virtual BOOL GetCallStack(LUADEBUG_CALLSTACK* pStacks, int nSize, int &nDepth);

	virtual BOOL IsConnected();
	virtual BOOL IsStop();
	virtual BOOL Continue();

	virtual void Release() {
		delete this;
	}

	void OnRPCBreakPoint();
	void OnRPCDebugMessage(int nType, LPCSTR pMsg);
	void OnAttched();
	void OnDetched();

protected:
	BOOL m_bIsStop;
	PROTOCOL_LUA_CLIENT* m_pClient;
	ILuaDebugHooker* m_pHooker;
	HANDLE m_hBPEvent;
};

BOOL InitializeLuaDebugClient(const char* addr)
{
	SOCK_ADDR sa;

	if(sock_str2addr(addr, &sa)==NULL) {
		return FALSE;
	}

	mempool_init();
	sock_init();
	fdwatch_init();
	threadpool_init(1);
	rpcnet_init();
	rpcfun_init();
	dymempool_init(50, 2048);

	if(rpcnet_bind(&sa)==ERR_NOERROR) {
		protocol_lua_init();
		protocol_luaclt_init();
		return TRUE;
	}

	dymempool_final();
	rpcfun_final();
	rpcnet_final();
	threadpool_final();
	fdwatch_final();
	sock_final();
	mempool_final();
	return FALSE;
}

BOOL FinalizeLuaDebugClient()
{
	protocol_luaclt_final();
	protocol_lua_final();
	rpcnet_unbind();
	dymempool_final();
	rpcfun_final();
	rpcnet_final();
	threadpool_final();
	fdwatch_final();
	sock_final();
	mempool_final();
	return TRUE;
}

ILuaDebugClient* CreateLuaDebugClient()
{
	return new CLuaDebugClient;
}

CLuaDebugClient::CLuaDebugClient()
{
	m_bIsStop = FALSE;
	m_pClient = NULL;
	m_pHooker = NULL;
	m_hBPEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CLuaDebugClient::~CLuaDebugClient()
{
	DeleteObject(m_hBPEvent);
}

BOOL CLuaDebugClient::Connect(LPCSTR pHostEP, unsigned int nSId, ILuaDebugHooker* pHooker)
{
	RPCNET_GROUP* pHost;
	SOCK_ADDR sa;
	PROTOCOL_LUA_DEBUG_CALLBACK callback;

	if(sock_str2addr(pHostEP, &sa)==NULL)
		return FALSE;

	if(m_pClient)
		return FALSE;

	pHost = rpcnet_getgroup(&sa);
	if(pHost==NULL)
		return FALSE;

	m_pHooker = pHooker;

	callback.attach		= attach;
	callback.detach		= detach;
	callback.debugbreak	= debugbreak;
	callback.debugmsg	= debugmsg;
	callback.userptr	= this;
	m_pClient = protocol_luaclt_attach(pHost, nSId, &callback);
	if(!m_pClient) {
		m_pHooker = NULL;
		return FALSE;
	} else {
		return TRUE;
	}
}

BOOL CLuaDebugClient::Disconnect()
{
	if(!m_pClient)
		return FALSE;

	protocol_luaclt_detach(m_pClient);

	return TRUE;
}

BOOL CLuaDebugClient::RunCmd(LPCSTR pCmd)
{
	int ret;

	if(!m_pClient) return FALSE;

	ret = protocol_luaclt_runcmd(m_pClient, pCmd);
	return ret==ERR_NOERROR?TRUE:FALSE;
}

BOOL CLuaDebugClient::GetCallStack(LUADEBUG_CALLSTACK* pStacks, int nSize, int &nDepth)
{
	int ret;

	if(!m_pClient) return FALSE;

	nDepth = nSize;
	ret = protocol_luaclt_getstack(m_pClient, pStacks, &nDepth);

	return ret==ERR_NOERROR?TRUE:FALSE;
}

BOOL CLuaDebugClient::IsConnected()
{
	return m_pClient?TRUE:FALSE;
}

BOOL CLuaDebugClient::IsStop()
{
	return m_bIsStop;
}

BOOL CLuaDebugClient::Continue()
{
	if(!m_bIsStop)
		return FALSE;

	SetEvent(m_hBPEvent);
	return TRUE;
}

void CLuaDebugClient::OnRPCBreakPoint()
{
	m_bIsStop = TRUE;
	m_pHooker->OnBreakPoint(this);
	WaitForSingleObject(m_hBPEvent, INFINITE);
	m_bIsStop = FALSE;
}

void CLuaDebugClient::OnRPCDebugMessage(int nType, LPCSTR pMsg)
{
	m_pHooker->OnDebugMessage(this, nType, pMsg);
}

void CLuaDebugClient::OnAttched()
{
	m_pHooker->OnConnect(this);
}

void CLuaDebugClient::OnDetched()
{
	m_pHooker->OnDisconnect(this);
	m_pClient = NULL;
	m_pHooker = NULL;
}

void attach(PROTOCOL_LUA_CLIENT* clt)
{
	CLuaDebugClient* pClient;
	pClient = (CLuaDebugClient*)protocol_luaclt_userptr(clt);
	pClient->OnAttched();
}

void detach(PROTOCOL_LUA_CLIENT* clt)
{
	CLuaDebugClient* pClient;
	pClient = (CLuaDebugClient*)protocol_luaclt_userptr(clt);
	pClient->OnDetched();
}

void debugbreak(PROTOCOL_LUA_CLIENT* clt)
{
	CLuaDebugClient* pClient;
	pClient = (CLuaDebugClient*)protocol_luaclt_userptr(clt);
	pClient->OnRPCBreakPoint();
}

void debugmsg(PROTOCOL_LUA_CLIENT* clt, int type, const char* msg)
{
	CLuaDebugClient* pClient;
	pClient = (CLuaDebugClient*)protocol_luaclt_userptr(clt);
	pClient->OnRPCDebugMessage(type, msg);
}
