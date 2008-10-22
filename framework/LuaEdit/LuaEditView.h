// LuaEditView.h : interface of the CLuaEditView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CLuaEditView : public CWindowImpl<CLuaEditView, CEdit>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CEdit::GetWndClassName())

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CLuaEditView)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
};
