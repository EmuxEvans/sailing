#include "StdAfx.h"

#include "resource.h"

#include "LuaHost\LuaDebugInfo.h"
#include "LuaDebugClient.h"

#include "DialogWindow.h"
#include "DebugHostWindow.h"
#include "FileManager.h"
#include "LuaDebugHooker.h"

BOOL CDebugHostDlg::OnInitDialog(HWND, LPARAM)
{
	// ...
	m_CallStack.m_hWnd = GetDlgItem(IDC_CALLSTACK);
	m_CallStack.AddColumn(_T(""), 0);
	m_CallStack.AddColumn(_T("Name"), 1);
	m_CallStack.AddColumn(_T("Source"), 2);
	m_CallStack.SetColumnWidth(0, 40);
	m_CallStack.SetColumnWidth(1, 100);
	m_CallStack.SetColumnWidth(2, 100);
	
	ListView_SetExtendedListViewStyle(GetDlgItem(IDC_CALLSTACK), ListView_GetExtendedListViewStyle(GetDlgItem(IDC_CALLSTACK))|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

	// 

	SetMsgHandled(FALSE);
	return TRUE;
}

LRESULT CDebugHostDlg::OnDebugContinue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(!CLuaDebugManager::GetDefault()->GetDebugHooker()) {
		MessageBox("Not Attach");
		return 0;
	}
	if(!CLuaDebugManager::GetDefault()->GetDebugHooker()->GetLuaDebugClient()->IsStop()) {
		MessageBox("Not Stop");
		return 0;
	}
	CLuaDebugManager::GetDefault()->GetDebugHooker()->Continue();
	return 0;
}

LRESULT CDebugHostDlg::OnCallStack_DlbClk(int wID, LPNMHDR pNM, BOOL &bHandled)
{
	LPNMITEMACTIVATE pNMItem = (LPNMITEMACTIVATE)pNM;
	bHandled = TRUE;
	if(!CLuaDebugManager::GetDefault()->GetDebugHooker()) return 0;
	if(!CLuaDebugManager::GetDefault()->GetDebugHooker()->GetLuaDebugClient()->IsStop()) return 0;
	if(pNMItem->iItem<0 || pNMItem->iItem>=CLuaDebugManager::GetDefault()->GetDebugHooker()->GetStackDepth()) return 0;
	CLuaDebugManager::GetDefault()->GetDebugHooker()->GoStack(pNMItem->iItem);
	return 0;
}

void CDebugHostDlg::Update(LUADEBUG_CALLSTACK* pStack, int nCount)
{
	m_CallStack.DeleteAllItems();
	if(pStack==NULL) return;

	for(int i=0; i<nCount; i++) {
		char szFlag[5];
		char szSource[100];
		memset(szFlag, 0, sizeof(szFlag));
		if(strcmp(pStack[i].namewhat, "global")==0) szFlag[1] = 'G';
		if(strcmp(pStack[i].namewhat, "local")==0) szFlag[1] = 'L';
		if(strcmp(pStack[i].namewhat, "field")==0) szFlag[1] = 'F';
		if(strcmp(pStack[i].namewhat, "method")==0) szFlag[1] = 'M';
		if(strcmp(pStack[i].what, "Lua")==0) szFlag[2] = 'L';
		if(strcmp(pStack[i].what, "C")==0) szFlag[2] = 'C';
		if(strcmp(pStack[i].what, "main")==0) szFlag[2] = 'M';
		if(strcmp(pStack[i].what, "tail")==0) szFlag[2] = 'T';
		if(szFlag[1]==0) szFlag[1] = 'U';
		if(szFlag[2]==0) szFlag[2] = 'U';
		szFlag[0] = '[';
		szFlag[3] = ']';
		sprintf(szSource, "%s:%d", pStack[i].source, pStack[i].currentline);
		m_CallStack.AddItem(i, 0, szFlag);
		m_CallStack.AddItem(i, 1, pStack[i].name);
		m_CallStack.AddItem(i, 2, szSource);
	}
}
