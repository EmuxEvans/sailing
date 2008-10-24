#pragma once

class CCommandDlg :
	public CDialogImpl<CCommandDlg>,
	public CDialogLayout<CCommandDlg>
{
public:
	enum { IDD = IDD_COMMAND_WINDOW };

	BEGIN_MSG_MAP(CCommandDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(CDialogLayout<CCommandDlg>)
	END_MSG_MAP()

	BEGIN_LAYOUT_MAP()
		LAYOUT_CONTROL(IDC_CONSOLE, LAYOUT_ANCHOR_ALL)
		LAYOUT_CONTROL(IDC_RUN, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_BOTTOM)
		LAYOUT_CONTROL(IDC_COMMAND, LAYOUT_ANCHOR_HORIZONTAL | LAYOUT_ANCHOR_BOTTOM)
	END_LAYOUT_MAP()

	BOOL OnInitDialog(HWND, LPARAM)
	{
		// ...

		SetMsgHandled(FALSE);
		return TRUE;
	}

};

class CCommandWindow
	: public dockwins::CTitleDockingWindowImpl<CCommandWindow, CWindow, dockwins::COutlookLikeTitleDockingWindowTraits>
{
	typedef CCommandWindow	thisClass;
	typedef dockwins::CTitleDockingWindowImpl< CCommandWindow, CWindow, dockwins::COutlookLikeTitleDockingWindowTraits> baseClass;
public:

	BEGIN_MSG_MAP(thisClass)
		CHAIN_MSG_MAP(baseClass)
//		CHAIN_MSG_MAP(CCommandWindowDlg)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize);
	END_MSG_MAP()

	CCommandDlg m_CommandDlg;

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		BOOL bHandle = TRUE;
		m_CommandDlg.Create(m_hWnd, NULL);
		m_CommandDlg.SetParent(m_hWnd);
//		OnSize(0, 0, 0, bHandle);
		return 0;
	}
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		RECT rcClient;
		GetClientRect(&rcClient);
		m_CommandDlg.SetWindowPos(NULL, &rcClient, SWP_NOACTIVATE | SWP_NOOWNERZORDER);
		m_CommandDlg.ShowWindow(SW_SHOWNOACTIVATE);
		return 0;
	}

};
