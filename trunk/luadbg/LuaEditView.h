// LuaEditView.h : interface of the CLuaEditView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CLuaEditView : public CWindowImpl<CLuaEditView, CSciLexerEdit>
{
public:
	CLuaEditView();
	~CLuaEditView();

	BOOL PreTranslateMessage(MSG* pMsg);

	void Init();

	BEGIN_MSG_MAP(CLuaEditView)
	END_MSG_MAP()

	void SetFileName(LPCTSTR pFileName, BOOL bNewFile=FALSE) {
		m_FileName = pFileName;
		m_bNewFile = bNewFile;
	}
	LPCTSTR GetFileName() {
		return m_FileName;
	}
	BOOL IsNewFile() {
		return m_bNewFile;
	}

protected:
	CString m_FileName;
	BOOL m_bNewFile;

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

};
