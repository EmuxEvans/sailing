#include "StdAfx.h"
#include "LuaDebugClient.h"

class CLuaDebugClient;

static map<string, CLuaDebugClient*> client_map;

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

protected:
	ILuaDebugHooker* m_pHooker;
	HANDLE m_hBPEvent;
};

BOOL InitializeLuaDebugClient()
{
	return TRUE;
}

BOOL FinalizeLuaDebugClient()
{
	return TRUE;
}

ILuaDebugClient* CreateLuaDebugClient()
{
	return new CLuaDebugClient;
}

CLuaDebugClient::CLuaDebugClient()
{
}

CLuaDebugClient::~CLuaDebugClient()
{
}

BOOL CLuaDebugClient::Connect(LPCSTR pHostEP, ILuaDebugHooker* pHooker)
{
	return TRUE;
}

BOOL CLuaDebugClient::Disconnect()
{
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
