#pragma once

template<class T, class TDialog>
class CDialogWindow
	: public dockwins::CTitleDockingWindowImpl<T, CWindow, dockwins::COutlookLikeTitleDockingWindowTraits>
{
	typedef T thisClass;
	typedef dockwins::CTitleDockingWindowImpl< T, CWindow, dockwins::COutlookLikeTitleDockingWindowTraits> baseClass;
public:

	BEGIN_MSG_MAP(thisClass)
		CHAIN_MSG_MAP(baseClass)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize);
	END_MSG_MAP()

	TDialog m_Dlg;

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		BOOL bHandle = TRUE;
		m_Dlg.Create(m_hWnd, NULL);
		m_Dlg.SetParent(m_hWnd);
		return 0;
	}
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		RECT rcClient;
		GetClientRect(&rcClient);
		m_Dlg.SetWindowPos(NULL, &rcClient, SWP_NOACTIVATE | SWP_NOOWNERZORDER);
		m_Dlg.ShowWindow(SW_SHOWNOACTIVATE);
		return 0;
	}

};
