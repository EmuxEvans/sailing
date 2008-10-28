#pragma once

class CDebugHostDlg :
	public CDialogImpl<CDebugHostDlg>,
	public CDialogLayout<CDebugHostDlg>
{
public:
	enum { IDD = IDD_DEBUGHOST_WINDOW };

	BEGIN_MSG_MAP(CDebugHostDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		NOTIFY_HANDLER(IDC_CALLSTACK, NM_DBLCLK, OnCallStack_DlbClk)
		COMMAND_ID_HANDLER(ID_DEBUG_CONTINUE, OnDebugContinue)
		COMMAND_ID_HANDLER(ID_DEBUG_DETACHHOST, OnDebugDetachHost)
		COMMAND_ID_HANDLER(ID_DEBUG_ATTACHHOST, OnDebugAttachHost)
		CHAIN_MSG_MAP(CDialogLayout<CDebugHostDlg>)
	END_MSG_MAP()

	CListViewCtrl	m_CallStack;
	CComboBox		m_HostList;

	BEGIN_LAYOUT_MAP()
		LAYOUT_CONTROL(ID_DEBUG_ATTACHHOST, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_TOP)
		LAYOUT_CONTROL(ID_DEBUG_DETACHHOST, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_TOP)
		LAYOUT_CONTROL(ID_DEBUG_CONTINUE, LAYOUT_ANCHOR_RIGHT | LAYOUT_ANCHOR_TOP)
		LAYOUT_CONTROL(IDC_HOSTLIST, LAYOUT_ANCHOR_HORIZONTAL | LAYOUT_ANCHOR_TOP)
		LAYOUT_CONTROL(IDC_CALLSTACK, LAYOUT_ANCHOR_ALL)
	END_LAYOUT_MAP()

	BOOL OnInitDialog(HWND, LPARAM);
	LRESULT OnDebugContinue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDebugDetachHost(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDebugAttachHost(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCallStack_DlbClk(int wID, LPNMHDR pNM, BOOL &bHandled);

	void Update(LUADEBUG_CALLSTACK* pStack, int nCount);

};

class CDebugHostWindow : public CDialogWindow<CDebugHostWindow, CDebugHostDlg>
{
public:
};
