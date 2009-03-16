// SGClientView.h : interface of the CSGClientView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CMainView : public CDialogImpl<CMainView>, public CDialogLayout<CMainView>, public ISGClientCallback
{
public:
	enum { IDD = IDD_MAINVIEW };

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CMainView)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnRunCommand)
		COMMAND_ID_HANDLER(IDC_RUNCMD, OnRunCommand)
		COMMAND_ID_HANDLER(IDC_CLRLOG, OnClearLog)
		
		CHAIN_MSG_MAP(CDialogLayout<CMainView>)
	END_MSG_MAP()

	BEGIN_LAYOUT_MAP()
//		LAYOUT_CONTROL(IDC_SCRIPT, LAYOUT_ANCHOR_HORIZONTAL | LAYOUT_ANCHOR_BOTTOM)
		LAYOUT_CONTROL(IDC_COMMAND, LAYOUT_ANCHOR_HORIZONTAL | LAYOUT_ANCHOR_BOTTOM)
		LAYOUT_CONTROL(IDC_RUNCMD, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_BOTTOM)
		LAYOUT_CONTROL(IDC_CLRLOG, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_BOTTOM)
//		LAYOUT_CONTROL(IDC_MODECHANGE, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_BOTTOM)
		LAYOUT_CONTROL(IDC_CONSOLE, LAYOUT_ANCHOR_ALL)
	END_LAYOUT_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	CEdit m_Console;
	CEdit m_Command;

	CMainView();
	~CMainView();

	BOOL OnInitDialog(HWND, LPARAM);
	LRESULT OnRunCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnClearLog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);

	virtual void OnConnect();
	virtual void OnData(const void* pData, unsigned int nSize);
	virtual void OnDisconnect();

	int LuaCallback();

};
