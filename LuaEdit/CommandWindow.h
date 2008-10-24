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

class CCommandWindow : public CDialogWindow<CCommandWindow, CCommandDlg>
{
public:
};
