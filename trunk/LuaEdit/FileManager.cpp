#include "StdAfx.h"
#include "SciLexerEdit.h"
#include "LuaEditView.h"
#include "FileManager.h"

CFileManager::CFileManager(CTabView* pTabView)
{
	m_pTabView = pTabView;
}

CFileManager::~CFileManager()
{
}

BOOL CFileManager::New()
{
	CLuaEditView* pView = new CLuaEditView;
	pView->Create(m_pTabView->m_hWnd, CWindow::rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL, 0);
	pView->SetFont(AtlGetDefaultGuiFont());
	m_pTabView->AddPage(pView->m_hWnd, _T(""), -1, pView);
	pView->Init();
	pView->SetFileName("NoName.lua");
	return FALSE;
}

BOOL CFileManager::Open(LPCTSTR pFileName)
{
	CLuaEditView* pView = new CLuaEditView;
	pView->Create(m_pTabView->m_hWnd, CWindow::rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL, 0);
	pView->SetFont(AtlGetDefaultGuiFont());
	m_pTabView->AddPage(pView->m_hWnd, _T(""), -1, pView);
	pView->Init();
	pView->LoadFile(pFileName);
	LPCTSTR pName = pFileName;
	if(pName<_tcsrchr(pFileName, _T('\\'))) pName = _tcsrchr(pFileName, _T('\\'));
	if(pName<_tcsrchr(pFileName, _T('/'))) pName = _tcsrchr(pFileName, _T('/'));
	pView->SetFileName("NoName.lua");
	return FALSE;
}

BOOL CFileManager::Save(LPCTSTR pFileName)
{
	CLuaEditView* pView;
	pView = (CLuaEditView*)m_pTabView->GetPageData(m_pTabView->GetActivePage());
	return FALSE;
}

BOOL CFileManager::SaveAll()
{
	return FALSE;
}

BOOL CFileManager::CloseAll()
{
	m_pTabView->RemoveAllPages();
	return FALSE;
}

void CFileManager::UpdateUI()
{
}
