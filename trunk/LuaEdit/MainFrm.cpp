// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "LuaHost\LuaDebugInfo.h"
#include "LuaDebugClient.h"

#include "AboutDlg.h"
#include "DropFileHandler.h"
#include "FileManager.h"
#include "DialogWindow.h"
#include "DebugHostWindow.h"
#include "CommandWindow.h"
#include "MainFrm.h"

#include "SciLexerEdit.h"
#include "LuaEditView.h"
#include "LuaDebugHooker.h"

#include <skates\skates.h>

CMainFrame::CMainFrame() : m_FileManager(&m_view)
{
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(dockwins::CDockingFrameImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();
	UISetCheck(ID_VIEW_DEBUGHOST, m_DebugHostWindow.IsVisible());
	UISetCheck(ID_VIEW_COMMAND, m_CommandWindow.IsVisible());
	m_FileManager.UpdateUI();
	return FALSE;
}

BOOL CMainFrame::IsReadyForDrop()
{
	return TRUE;
}

BOOL CMainFrame::HandleDroppedFile(LPCTSTR szBuff)
{
	LPCTSTR pDot;
	pDot = _tcsrchr(szBuff, _T('.'));
	if(pDot==NULL || _tcsicmp(pDot+1, _T("lua"))!=0) return TRUE;
	m_FileManager.Open(szBuff);
	return TRUE;
}

void CMainFrame::EndDropFiles()
{
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateSimpleStatusBar();

	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	UISetCheck(ID_VIEW_OUTPUT, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenuHandle menuMain = m_CmdBar.GetMenu();
	m_view.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));

	RegisterDropHandler();

	InitializeDockingFrame();
	DWORD dwStyle=WS_OVERLAPPEDWINDOW | WS_POPUP| WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	CRect rcBar(0, 0, 100, 200);
	m_DebugHostWindow.Create(m_hWnd, rcBar, _T("Debug Host"), dwStyle);
	DockWindow(m_DebugHostWindow, dockwins::CDockingSide(dockwins::CDockingSide::sBottom),
		0/*nBar*/,float(0.5)/*fPctPos*/, 100 /*nWidth*/,200/* nHeight*/);
	m_CommandWindow.Create(m_hWnd, rcBar, _T("Command"), dwStyle);
	DockWindow(m_CommandWindow, dockwins::CDockingSide(dockwins::CDockingSide::sBottom),
		0/*nBar*/,float(0.5)/*fPctPos*/, 100 /*nWidth*/,200/* nHeight*/);

	SendMessage(CWM_INITIALIZE, 0, 0);

	return 0;
}

LRESULT CMainFrame::OnInitialize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	sstate::CDockWndMgr mgrDockWnds;
	mgrDockWnds.Add(sstate::CDockingWindowStateAdapter<CDebugHostWindow>(m_DebugHostWindow));
	mgrDockWnds.Add(sstate::CDockingWindowStateAdapter<CCommandWindow>(m_CommandWindow));

	m_stateMgr.Initialize(m_hWnd);
	m_stateMgr.Add(sstate::CRebarStateAdapter(m_hWndToolBar));
	m_stateMgr.Add(sstate::CToggleWindowAdapter(m_hWndStatusBar));
	m_stateMgr.Add(mgrDockWnds);
	CRegKey key;
	if(key.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\Sailing\\LuaEdit"), KEY_READ)==ERROR_SUCCESS)
	{
		sstate::CStgRegistry reg(key.Detach());
		m_stateMgr.Restore(reg);
	}
	else
		m_stateMgr.RestoreDefault();

	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnNCDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	//CRegKey key;
	//if(key.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\Sailing\\LuaEdit"), KEY_WRITE)==ERROR_SUCCESS || key.Create(HKEY_CURRENT_USER,_T("SOFTWARE\\Sailing\\LuaEdit"))==ERROR_SUCCESS)
	//{
	//	sstate::CStgRegistry reg(key.Detach());
	//	m_stateMgr.Store(reg);
	//}

	if(CLuaDebugManager::GetDefault()->GetDebugHooker()) {
		if(CLuaDebugManager::GetDefault()->GetDebugHooker()->GetLuaDebugClient()->IsStop()) {
			CLuaDebugManager::GetDefault()->GetDebugHooker()->GetLuaDebugClient()->Continue();
		}
		CLuaDebugManager::GetDefault()->DeleteHooker();
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_FileManager.New();
	return 0;
}

LRESULT CMainFrame::OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog a(TRUE, _T("*.lua"), NULL, OFN_FILEMUSTEXIST, _T("Lua Source File (*.lua)\0*.lua\0"));
	if(a.DoModal()!=IDOK) return 0;
	HandleDroppedFile(a.m_szFileName);
	return 0;
}

LRESULT CMainFrame::OnFileClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()>=0)
		m_view.RemovePage(m_view.GetActivePage());
	return 0;
}

LRESULT CMainFrame::OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nIndex = m_view.GetActivePage();
	if(nIndex>=0) {
		if(m_FileManager.IsNewFile(nIndex)) {
			BOOL bHandled = TRUE;
			return OnFileSaveAs(0, 0, NULL, bHandled);
		}
		m_FileManager.Save(nIndex);
	}
	return 0;
}

LRESULT CMainFrame::OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nIndex = m_view.GetActivePage();
	if(nIndex>=0) {
		CFileDialog a(FALSE, _T("*.lua"), m_FileManager.GetFileName(nIndex), 0, _T("Lua Source File (*.lua)\0*.lua\0"));
		if(a.DoModal()!=IDOK) return 0;
		m_FileManager.Save(nIndex, a.m_szFileName);
		return 0;
	}
	return 0;
}

LRESULT CMainFrame::OnFileCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	for(int i=m_view.GetPageCount()-1; i>=0; i--) {
		m_FileManager.Close(i);
	}
	return 0;
}

LRESULT CMainFrame::OnFileSaveAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	for(int nIndex=m_view.GetPageCount()-1; nIndex>=0; nIndex--) {
		if(m_FileManager.IsNewFile(nIndex)) {
			CFileDialog a(FALSE, _T("*.lua"), m_FileManager.GetFileName(nIndex), 0, _T("Lua Source File (*.lua)\0*.lua\0"));
			if(a.DoModal()!=IDOK) break;
			m_FileManager.Save(nIndex, a.m_szFileName);
		}
		m_FileManager.Save(nIndex);
	}
	return 0;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnEditUndo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	CLuaEditView* pView;
	pView = (CLuaEditView*)m_view.GetPageData(m_view.GetActivePage());
	pView->Undo();
	return 0;
}

LRESULT CMainFrame::OnEditRedo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	CLuaEditView* pView;
	pView = (CLuaEditView*)m_view.GetPageData(m_view.GetActivePage());
	pView->Redo();
	return 0;
}

LRESULT CMainFrame::OnEditCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	CLuaEditView* pView;
	pView = (CLuaEditView*)m_view.GetPageData(m_view.GetActivePage());
	pView->Cut();
	return 0;
}

LRESULT CMainFrame::OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	CLuaEditView* pView;
	pView = (CLuaEditView*)m_view.GetPageData(m_view.GetActivePage());
	pView->Copy();
	return 0;
}

LRESULT CMainFrame::OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	CLuaEditView* pView;
	pView = (CLuaEditView*)m_view.GetPageData(m_view.GetActivePage());
	pView->Paste();
	return 0;
}

LRESULT CMainFrame::OnEditFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	CFindDlg Dlg;
	Dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT CMainFrame::OnEditFindNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	return 0;
}

LRESULT CMainFrame::OnEditReplace(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	CRelaceDlg Dlg;
	Dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT CMainFrame::OnEditGoTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	int nLineNumber, nLineMax;
	CLuaEditView* pView;
	pView = (CLuaEditView*)m_view.GetPageData(m_view.GetActivePage());

	nLineMax = (int)pView->GetLineCount();
	nLineNumber = (int)pView->GetCurrentLine();

	CGoToLineDlg Dlg(nLineMax, nLineNumber);
	if(Dlg.DoModal(m_hWnd)!=IDOK) return 0;
	pView->GotoLine(Dlg.m_nLineNumber);
	return 0;
}

LRESULT CMainFrame::OnBookmarkToggle(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	CLuaEditView* pView;
	pView = (CLuaEditView*)m_view.GetPageData(m_view.GetActivePage());
	long lLine = pView->GetCurrentLine();
	if(pView->HasBookmark(lLine)) {
		pView->DeleteBookmark(lLine);
	} else {
		pView->AddBookmark(lLine);
	}
	return 0;
}

LRESULT CMainFrame::OnBookmarkNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	CLuaEditView* pView;
	pView = (CLuaEditView*)m_view.GetPageData(m_view.GetActivePage());
	pView->FindNextBookmark();
	return 0;
}

LRESULT CMainFrame::OnBookmarkPrev(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	CLuaEditView* pView;
	pView = (CLuaEditView*)m_view.GetPageData(m_view.GetActivePage());
	pView->FindPreviousBookmark();
	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnDebugAttachHost(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(CLuaDebugManager::GetDefault()->GetDebugHooker()) {
		MessageBox("Already Attached");
		return 0;
	}

	CAttachHostDlg Dlg;
	if(Dlg.DoModal(m_hWnd)!=IDOK) return 0;

	if(!CLuaDebugManager::GetDefault()->NewHooker(this)) {
		MessageBox("Already Attached");
		return 0;
	}

	if(!CLuaDebugManager::GetDefault()->GetDebugHooker()->Connect(Dlg.m_szAddress)) {
		MessageBox("Connect Error");
		CLuaDebugManager::GetDefault()->DeleteHooker();
		return 0;
	}

	return 0;
}

LRESULT CMainFrame::OnDebugDetachHost(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(!CLuaDebugManager::GetDefault()->GetDebugHooker()) {
		MessageBox("Not Attach");
		return 0;
	}
	CLuaDebugManager::GetDefault()->GetDebugHooker()->Disconnect();
	CLuaDebugManager::GetDefault()->DeleteHooker();
	return 0;
}

LRESULT CMainFrame::OnDebugBeginHost(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CMainFrame::OnDebugHostSetting(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CHostSettingDlg Dlg;
	Dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT CMainFrame::OnDebugContinue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(!CLuaDebugManager::GetDefault()->GetDebugHooker()) {
		MessageBox("Not Attach");
		return 0;
	}
	if(!CLuaDebugManager::GetDefault()->GetDebugHooker()->GetLuaDebugClient()->IsStop()) {
		MessageBox("Not Stop");
		return 0;
	}
	CLuaDebugManager::GetDefault()->GetDebugHooker()->GetLuaDebugClient()->Continue();
	m_DebugHostWindow.m_Dlg.Update(NULL, 0);
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnWindowActivate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nPage = wID - ID_WINDOW_TABFIRST;
	m_view.SetActivePage(nPage);
	return 0;
}

LRESULT CMainFrame::OnWindowNext(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()>=0)
		m_view.SetActivePage((m_view.GetActivePage()+1+m_view.GetPageCount()) % m_view.GetPageCount());
	return 0;
}

LRESULT CMainFrame::OnWindowPrev(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()>=0)
		m_view.SetActivePage((m_view.GetActivePage()-1+m_view.GetPageCount()) % m_view.GetPageCount());
	return 0;
}
