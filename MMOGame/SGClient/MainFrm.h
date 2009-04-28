// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CUpdateUI<CMainFrame>,
		public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	CMainView m_view;

	CCommandBarCtrl m_CmdBar;

	CMainFrame();
	~CMainFrame();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	void SetConnection(int nNumber);
	int GetConnection();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_CONN_00, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_CONN_01, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_CONN_02, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_CONN_03, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_CONN_04, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_CONN_05, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_CONN_06, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_CONN_07, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_CONN_08, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_CONN_09, UPDUI_TOOLBAR)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)

		COMMAND_ID_HANDLER(ID_GENERATE_HFILE,	OnGenHFile)
		COMMAND_ID_HANDLER(ID_GENERATE_DBFILE,	OnGenDBFile)

		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)

		COMMAND_ID_HANDLER(ID_CONN_00, OnConnClick)
		COMMAND_ID_HANDLER(ID_CONN_01, OnConnClick)
		COMMAND_ID_HANDLER(ID_CONN_02, OnConnClick)
		COMMAND_ID_HANDLER(ID_CONN_03, OnConnClick)
		COMMAND_ID_HANDLER(ID_CONN_04, OnConnClick)
		COMMAND_ID_HANDLER(ID_CONN_05, OnConnClick)
		COMMAND_ID_HANDLER(ID_CONN_06, OnConnClick)
		COMMAND_ID_HANDLER(ID_CONN_07, OnConnClick)
		COMMAND_ID_HANDLER(ID_CONN_08, OnConnClick)
		COMMAND_ID_HANDLER(ID_CONN_09, OnConnClick)

		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGenHFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGenDBFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnConnClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	int m_nConnection;
};
