#pragma once

//class COutputWindowDlg :
//	public CDialogImpl<COutputWindowDlg>,
//	public CDialogLayout<COutputWindowDlg>
//{
//public:
//	enum { IDD = IDD_COMMAND_WINDOW };
//
//	BEGIN_MSG_MAP(COutputWindowDlg)
//		MSG_WM_INITDIALOG(OnInitDialog)
//		CHAIN_MSG_MAP(CDialogLayout<COutputWindowDlg>)
//	END_MSG_MAP()
//
//	BEGIN_LAYOUT_MAP()
//		LAYOUT_CONTROL(IDC_CONSOLE, LAYOUT_ANCHOR_ALL)
//		LAYOUT_CONTROL(IDC_RUN, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_BOTTOM)
//		LAYOUT_CONTROL(IDC_COMMAND, LAYOUT_ANCHOR_HORIZONTAL | LAYOUT_ANCHOR_BOTTOM)
//	END_LAYOUT_MAP()
//
//	BOOL OnInitDialog(HWND, LPARAM)
//	{
//		// ...
//
//		SetMsgHandled(FALSE);
//		return TRUE;
//	}
//
//};
//
//class CCommandWindow
//	: public dockwins::CTitleDockingWindowImpl<CCommandWindow, COutputWindowDlg, dockwins::COutlookLikeTitleDockingWindowTraits>
//{
//	typedef CCommandWindow	thisClass;
//	typedef dockwins::CTitleDockingWindowImpl< CCommandWindow, COutputWindowDlg, dockwins::COutlookLikeTitleDockingWindowTraits> baseClass;
//public:
//
//	BEGIN_MSG_MAP(thisClass)
//		CHAIN_MSG_MAP(baseClass)
////		CHAIN_MSG_MAP(COutputWindowDlg)
//	END_MSG_MAP()
//
//};
