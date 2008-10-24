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

	BEGIN_LAYOUT_MAP()
		LAYOUT_CONTROL(ID_DEBUG_CONTINUE, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_TOP)
		LAYOUT_CONTROL(IDC_HOSTLIST, LAYOUT_ANCHOR_HORIZONTAL | LAYOUT_ANCHOR_TOP)
		LAYOUT_CONTROL(IDC_CALLSTACK, LAYOUT_ANCHOR_ALL)
	END_LAYOUT_MAP()

	BOOL OnInitDialog(HWND, LPARAM)
	{
		// ...

		SetMsgHandled(FALSE);
		return TRUE;
	}

};

class CDebugHostWindow : public CDialogWindow<CDebugHostWindow, CDebugHostDlg>
{
public:
};
