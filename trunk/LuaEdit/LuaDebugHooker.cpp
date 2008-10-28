#include "StdAfx.h"

#include "LuaHost\LuaDebugInfo.h"
#include "LuaDebugClient.h"
#include "LuaDebugHooker.h"
#include "DropFileHandler.h"

#include "resource.h"
#include "DialogWindow.h"
#include "CommandWindow.h"
#include "DebugHostWindow.h"
#include "FileManager.h"
#include "MainFrm.h"

static CLuaDebugManager*	pDefaultManager = NULL;
static CLuaDebugHooker*		pDefaultHooker = NULL;
static CDebugHostWindow*	m_pDebugHostWindow = NULL;
static CCommandWindow*		m_pCommandWindow = NULL;
static CFileManager*		m_pFileManager = NULL;

CLuaDebugHooker::CLuaDebugHooker(CLuaDebugManager* pManager)
{
	m_pManager = pManager;
	m_pClient = CreateLuaDebugClient();
}

CLuaDebugHooker::~CLuaDebugHooker()
{
	if(m_pClient->IsConnected()) m_pClient->Disconnect();
	m_pClient->Release();
}

void CLuaDebugHooker::OnConnect(ILuaDebugClient* pClient)
{
}

void CLuaDebugHooker::OnDisconnect(ILuaDebugClient* pClient)
{
}

void CLuaDebugHooker::OnBreakPoint(ILuaDebugClient* pClient)
{
	if(pClient->GetCallStack(m_Stack, sizeof(m_Stack)/sizeof(m_Stack[0]), m_nDepth)) {
		m_pDebugHostWindow->m_Dlg.Update(m_Stack, m_nDepth);
	}
}

void CLuaDebugHooker::OnDebugMessage(ILuaDebugClient* pClient, int nType, const char* pMsg)
{
	m_pCommandWindow->m_Dlg.Print(pMsg);
	m_pCommandWindow->m_Dlg.Print("\n");
}

BOOL CLuaDebugHooker::Continue()
{
	BOOL bRet;
	bRet = m_pClient->Continue();
	m_pDebugHostWindow->m_Dlg.Update(NULL, 0);
	return bRet;
}

BOOL CLuaDebugHooker::GoStack(int nIndex)
{
	if(m_Stack[nIndex].source[0]=='@') {
		if(m_pFileManager->Open(m_Stack[nIndex].source+1)) {
			m_pFileManager->GotoLine(m_Stack[nIndex].currentline);
		}
	}
	return TRUE;
}

CLuaDebugManager::CLuaDebugManager()
{
}

CLuaDebugManager::~CLuaDebugManager()
{
}

CLuaDebugHooker* CLuaDebugManager::GetDebugHooker()
{
	return pDefaultHooker;
}

CLuaDebugHooker* CLuaDebugManager::NewHooker(CMainFrame* pMainFrame)
{
	if(pDefaultHooker) return NULL;
	pDefaultHooker = new CLuaDebugHooker(this);
	m_pDebugHostWindow = &pMainFrame->m_DebugHostWindow;
	m_pCommandWindow = &pMainFrame->m_CommandWindow;
	m_pFileManager = &pMainFrame->m_FileManager;
	return pDefaultHooker;
}

void CLuaDebugManager::DeleteHooker()
{
	if(!pDefaultHooker) return;
	delete pDefaultHooker;
	pDefaultHooker = NULL;
}

CLuaDebugManager* CLuaDebugManager::GetDefault()
{
	if(!pDefaultManager) pDefaultManager = new CLuaDebugManager;
	return pDefaultManager;
}
