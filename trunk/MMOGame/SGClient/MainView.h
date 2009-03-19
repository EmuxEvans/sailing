// SGClientView.h : interface of the CSGClientView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CMainView : public CDialogImpl<CMainView>, public ISGClientCallback
{
public:
	enum { IDD = IDD_MAINVIEW };

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CMainView)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		COMMAND_ID_HANDLER(IDOK, OnRunCommand)
		COMMAND_ID_HANDLER(IDC_RUNCMD, OnRunCommand)
		COMMAND_ID_HANDLER(IDC_CLRLOG, OnClearLog)
		COMMAND_ID_HANDLER(IDC_MODCHG, OnModeChange)

		COMMAND_ID_HANDLER(IDC_LOADTXT, OnLoadText)
		COMMAND_ID_HANDLER(IDC_SAVETXT, OnSaveText)

	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	CComboBox m_Command;
	CSciLexerEdit m_Script;
	CSciLexerEdit m_Console;

	CMainView();
	~CMainView();

	BOOL OnInitDialog(HWND, LPARAM);
	LRESULT OnSize(UINT, WPARAM, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRunCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnClearLog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnModeChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnLoadText(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnSaveText(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);

	void Tick();
	void Clear();

	virtual void OnConnect(ISGClient* pClient);
	virtual void OnData(ISGClient* pClient, const void* pData, unsigned int nSize);
	virtual void OnDisconnect(ISGClient* pClient);

	int LuaCallback();
	void Output(const char* pLine);

private:
	BOOL m_bCommandMode;
	char m_szFileName[MAX_PATH];
	int m_nConsoleWidth, m_nConsoleHeight;
	int m_nScriptTop, m_nScriptWidth;
	int m_nBottomTop;
	int m_nCommandWidth;
	int m_nModChgRight, m_nClrLogRight, m_nRunCmdRight;

};
