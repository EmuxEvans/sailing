#include "StdAfx.h"

#include "resource.h"

#include "DialogWindow.h"
#include "CommandWindow.h"

#include "LuaHost\LuaDebugInfo.h"
#include "LuaDebugClient.h"
#include "LuaDebugHooker.h"

LRESULT CCommandDlg::OnCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	TCHAR szCmd[1000];
	int nRetCode;
	::GetWindowText(GetDlgItem(IDC_COMMAND), szCmd, sizeof(szCmd));

	if(!CLuaDebugManager::GetDefault()->GetDebugHooker()) {
		MessageBox(_T("Not Attach"), _T("ERROR"), MB_OK);
		return 0;
	}

	if(!CLuaDebugManager::GetDefault()->GetDebugHooker()->GetLuaDebugClient()->RunCmd(szCmd, nRetCode)) {
		MessageBox(_T("RunCmd Error"), _T("ERROR"), MB_OK);
		return 0;
	}

	::SetWindowText(GetDlgItem(IDC_COMMAND), _T(""));
	return 0;
}
