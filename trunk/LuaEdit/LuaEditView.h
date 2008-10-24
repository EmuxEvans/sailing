// LuaEditView.h : interface of the CLuaEditView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CLuaEditView : public CWindowImpl<CLuaEditView, CSciLexerEdit>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CSciLexerEdit::GetWndClassName())

	CLuaEditView();
	~CLuaEditView();

	BOOL PreTranslateMessage(MSG* pMsg);

	void Init();

	BEGIN_MSG_MAP(CLuaEditView)
	END_MSG_MAP()

	void SetFileName(LPCTSTR pFileName) {
		m_FileName = pFileName;
	}
	LPCTSTR GetFileName() {
		return m_FileName;
	}
protected:
	CString m_FileName;

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

};
