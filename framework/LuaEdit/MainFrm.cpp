// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "AboutDlg.h"
#include "SciLexerEdit.h"
#include "LuaEditView.h"
#include "DropFileHandler.h"
#include "FileManager.h"
#include "MainFrm.h"

CMainFrame::CMainFrame() : m_FileManager(&m_view)
{
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();
	return FALSE;
}

BOOL CMainFrame::IsReadyForDrop()
{
	return TRUE;
}

BOOL CMainFrame::HandleDroppedFile(LPCTSTR szBuff)
{
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

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenuHandle menuMain = m_CmdBar.GetMenu();
	m_view.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));

	RegisterDropHandler();

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
	CLuaEditView* pView = new CLuaEditView;
	pView->Create(m_view, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL, 0);
	pView->SetFont(AtlGetDefaultGuiFont());
	m_view.AddPage(pView->m_hWnd, _T("Document"), -1, pView);
	pView->Init();

	// TODO: add code to initialize document

	return 0;
}

LRESULT CMainFrame::OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
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
	if(m_view.GetActivePage()<0)
		return 0;

	CLuaEditView* pView;
	pView = (CLuaEditView*)m_view.GetPageData(m_view.GetActivePage());
	return 0;
}

LRESULT CMainFrame::OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_view.GetActivePage()<0)
		return 0;

	CLuaEditView* pView;
	pView = (CLuaEditView*)m_view.GetPageData(m_view.GetActivePage());
	return 0;
}

LRESULT CMainFrame::OnFileCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_view.RemoveAllPages();
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
