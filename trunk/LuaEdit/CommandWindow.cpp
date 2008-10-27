#include "StdAfx.h"

#include "resource.h"

#include "DialogWindow.h"
#include "CommandWindow.h"

#include "LuaDebugClient.h"
extern ILuaDebugClient* m_pClient;

LRESULT CCommandDlg::OnCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	TCHAR szCmd[1000];
	int nRetCode;
	::GetWindowText(GetDlgItem(IDC_COMMAND), szCmd, sizeof(szCmd));

	if(!m_pClient) {
		MessageBox(_T("Not Attach"), _T("ERROR"), MB_OK);
		return 0;
	}

	if(!m_pClient->RunCmd(szCmd, nRetCode)) {
	}
	::SetWindowText(GetDlgItem(IDC_COMMAND), _T(""));

	return 0;
}
