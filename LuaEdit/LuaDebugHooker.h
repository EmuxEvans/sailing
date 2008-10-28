#pragma once

class CDebugHostWindow;
class CCommandWindow;
class CFileManager;
class CMainFrame;

class CLuaDebugHooker;
class CLuaDebugManager;

class CLuaDebugHooker : public ILuaDebugHooker
{
public:
	CLuaDebugHooker(CLuaDebugManager* pManager);
	~CLuaDebugHooker();

	virtual void OnConnect(ILuaDebugClient* pClient);
	virtual void OnDisconnect(ILuaDebugClient* pClient);
	virtual void OnBreakPoint(ILuaDebugClient* pClient);
	virtual void OnDebugMessage(ILuaDebugClient* pClient, int nType, const char* pMsg);

	ILuaDebugClient* GetLuaDebugClient() {
		return m_pClient;
	}
	LUADEBUG_CALLSTACK* GetStack(int nIndex) {
		if(nIndex<0 || nIndex>=m_nDepth) NULL;
		return &m_Stack[nIndex];
	}
	int GetStackDepth() {
		return m_nDepth;
	}

	BOOL Continue();
	BOOL GoStack(int nIndex);

private:
	CLuaDebugManager*	m_pManager;
	ILuaDebugClient*	m_pClient;
	LUADEBUG_CALLSTACK	m_Stack[30];
	int					m_nDepth;
};

class CLuaDebugManager
{
protected:
	CLuaDebugManager();
	~CLuaDebugManager();

public:
	CLuaDebugHooker* GetDebugHooker();
	CLuaDebugHooker* NewHooker(CMainFrame* pMainFrame);
	void DeleteHooker();

	static CLuaDebugManager* GetDefault();

};
