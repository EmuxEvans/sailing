#pragma once

class CCommandDlg :
	public CDialogImpl<CCommandDlg>,
	public CDialogLayout<CCommandDlg>
{
public:
	enum { IDD = IDD_COMMAND_WINDOW };

	CEdit m_Console;

	BEGIN_MSG_MAP(CCommandDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_RUN, OnCommand)
		CHAIN_MSG_MAP(CDialogLayout<CCommandDlg>)
	END_MSG_MAP()

	BEGIN_LAYOUT_MAP()
		LAYOUT_CONTROL(IDC_CONSOLE, LAYOUT_ANCHOR_ALL)
		LAYOUT_CONTROL(IDC_RUN, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_BOTTOM)
		LAYOUT_CONTROL(IDC_COMMAND, LAYOUT_ANCHOR_HORIZONTAL | LAYOUT_ANCHOR_BOTTOM)
	END_LAYOUT_MAP()

	CCommandDlg() {
		m_szMsg[0] = _T('\0');
	}

	void ClearScreen()
	{
		m_Console.SetWindowText(_T(""));
		m_szMsg[0] = _T('\0');
	}
	void Print(LPCTSTR pMsg)
	{
		m_Console.AppendText(pMsg);
	}

	BOOL OnInitDialog(HWND, LPARAM)
	{
		SetMsgHandled(FALSE);
		m_Console.m_hWnd = GetDlgItem(IDC_CONSOLE);
		return TRUE;
	}

	LRESULT OnCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);

	TCHAR m_szMsg[20*1024];
};

class CCommandWindow : public CDialogWindow<CCommandWindow, CCommandDlg>
{
public:
};
