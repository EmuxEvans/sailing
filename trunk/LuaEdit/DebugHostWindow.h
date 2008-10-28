#pragma once

class CDebugHostDlg :
	public CDialogImpl<CDebugHostDlg>,
	public CDialogLayout<CDebugHostDlg>
{
public:
	enum { IDD = IDD_DEBUGHOST_WINDOW };

	BEGIN_MSG_MAP(CDebugHostDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(CDialogLayout<CDebugHostDlg>)
	END_MSG_MAP()


	CListViewCtrl m_CallStack;

	BEGIN_LAYOUT_MAP()
		LAYOUT_CONTROL(ID_DEBUG_CONTINUE, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_TOP)
		LAYOUT_CONTROL(IDC_HOSTLIST, LAYOUT_ANCHOR_HORIZONTAL | LAYOUT_ANCHOR_TOP)
		LAYOUT_CONTROL(IDC_CALLSTACK, LAYOUT_ANCHOR_ALL)
	END_LAYOUT_MAP()

	BOOL OnInitDialog(HWND, LPARAM)
	{
		// ...
		m_CallStack.m_hWnd = GetDlgItem(IDC_CALLSTACK);
		m_CallStack.AddColumn(_T("a1"), 0);
		m_CallStack.AddColumn(_T("a2"), 1);
		m_CallStack.AddColumn(_T("a3"), 2);
		m_CallStack.AddColumn(_T("a4"), 3);
		m_CallStack.AddColumn(_T("a5"), 4);
		m_CallStack.AddColumn(_T("a6"), 5);
		m_CallStack.AddColumn(_T("a7"), 6);
		m_CallStack.AddItem(0, 0, "i1");
		m_CallStack.AddItem(0, 1, "i2");
		m_CallStack.AddItem(0, 2, "i3");
		m_CallStack.AddItem(0, 3, "i4");
		m_CallStack.AddItem(0, 4, "i5");


		SetMsgHandled(FALSE);
		return TRUE;
	}

};

class CDebugHostWindow : public CDialogWindow<CDebugHostWindow, CDebugHostDlg>
{
public:
};
