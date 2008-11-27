#include "StdAfx.h"

#include <skates\skates.h>

#include "LuaDebugClient.h"

void attach(PROTOCOL_LUA_CLIENT* pClient);
void detach(PROTOCOL_LUA_CLIENT* pClient);
void debugbreak(PROTOCOL_LUA_CLIENT* pClient);
void debugmsg(PROTOCOL_LUA_CLIENT* pClient, int type, const char* msg);


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
	void OnAttched(RPCNET_GROUP* grp);
	void OnDetched(RPCNET_GROUP* grp);

protected:
	BOOL m_bIsStop;
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

	mempool_init();
	sock_init();
	fdwatch_init();
	threadpool_init(1);
	rpcnet_init();
	rpcfun_init();
	dymempool_init(50, 2048);

	if(rpcnet_bind(&sa)==ERR_NOERROR) {
		protocol_lua_init();
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
	protocol_lua_final();
	rpcnet_unbind();
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
	m_pHost = NULL;
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
	int ret;
	SOCK_ADDR sa;
	PROTOCOL_LUA_DEBUG_CALLBACK callback;

	if(sock_str2addr(pHostEP, &sa)==NULL)
		return FALSE;

	if(m_pHost)
		return FALSE;

	pHost = rpcnet_getgroup(&sa);
	if(pHost==NULL)
		return FALSE;

	m_pHooker = pHooker;
	callback.attach		= NULL;
	callback.detach		= NULL;
	callback.debugbreak	= NULL;
	callback.debugmsg	= NULL;
	callback.userptr	= this;
	ret = protocol_lua_attach(pHost, nSId, &callback);
	if(ret!=ERR_NOERROR) {
		m_pHooker = NULL;
		return FALSE;
	} else {
		return TRUE;
	}
}

BOOL CLuaDebugClient::Disconnect()
{
	if(!m_pHost)
		return FALSE;

	protocol_lua_detach(NULL);

	return TRUE;
}

BOOL CLuaDebugClient::RunCmd(LPCSTR pCmd)
{
	int ret;

	if(!m_pHost) return FALSE;

	ret = protocol_lua_runcmd(NULL, pCmd);
	return ret==ERR_NOERROR?TRUE:FALSE;
}

BOOL CLuaDebugClient::GetCallStack(LUADEBUG_CALLSTACK* pStacks, int nSize, int &nDepth)
{
	int ret;

	nDepth = nSize;
	ret = protocol_lua_getstack(NULL, pStacks, &nDepth);

	return ret==ERR_NOERROR?TRUE:FALSE;
}

BOOL CLuaDebugClient::IsConnected()
{
	return m_pHooker?TRUE:FALSE;
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
