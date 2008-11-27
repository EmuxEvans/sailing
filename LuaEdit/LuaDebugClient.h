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
	virtual BOOL Connect(LPCSTR pHostEP, unsigned int nSId, ILuaDebugHooker* pHooker) = NULL;
	virtual BOOL Disconnect() = NULL;

	virtual BOOL RunCmd(LPCSTR pCmd) = NULL;
	virtual BOOL GetCallStack(LUADEBUG_CALLSTACK* pStacks, int nSize, int &nDepth) = NULL;

	virtual BOOL IsConnected() = NULL;
	virtual BOOL IsStop() = NULL;
	virtual BOOL Continue() = NULL;

	virtual void Release() = NULL;
};

BOOL InitializeLuaDebugClient(const char* addr);
BOOL FinalizeLuaDebugClient();
ILuaDebugClient* CreateLuaDebugClient();
