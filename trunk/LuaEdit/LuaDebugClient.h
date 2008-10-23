#pragma once

class ILuaDebugHooker;
class ILuaDebugClient;

class ILuaDebugHooker
{
public:
	virtual void OnConnect(ILuaDebugClient* pClient) = NULL;
	virtual void OnDisconnect(ILuaDebugClient* pClient) = NULL;

	virtual void OnBreakPoint(ILuaDebugClient* pClient) = NULL;
	virtual void OnDebugMessage(ILuaDebugClient* pClient, int nType, LPCSTR pMsg) = NULL;

};

class ILuaDebugClient
{
public:
	virtual BOOL Connect(LPCSTR pHostEP, ILuaDebugHooker* pHooker) = NULL;
	virtual BOOL Disconnect() = NULL;

	virtual BOOL RunCmd(LPCSTR pCmd) = NULL;

	virtual BOOL IsRuning() = NULL;
	virtual BOOL Continue() = NULL;

	virtual void Release() = NULL;
};

BOOL InitializeLuaDebugClient();
BOOL FinalizeLuaDebugClient();
ILuaDebugClient* CreateLuaDebugClient();
