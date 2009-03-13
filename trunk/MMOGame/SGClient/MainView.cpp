// SGClientView.cpp : implementation of the CSGClientView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "SGClient.h"
#include "SGCmdBuilder.h"
#include "MainView.h"
#include "MainFrm.h"

inline std::string& lTrim(std::string &ss)  
{
	std::string::iterator p=std::find_if(ss.begin(), ss.end(), std::not1(std::ptr_fun(isspace)));
	ss.erase(ss.begin(), p);
	return  ss;
}

inline std::string& rTrim(std::string &ss)
{
	std::string::reverse_iterator p=std::find_if(ss.rbegin(), ss.rend(), std::not1(std::ptr_fun(isspace)));
	ss.erase(p.base(), ss.end());
	return ss;
}

inline std::string& trim(std::string &st)
{
	lTrim(rTrim(st));
	return st;
}

BOOL CMainView::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainView::OnInitDialog(HWND, LPARAM)
{
	SetMsgHandled(FALSE);
	m_Console.m_hWnd = GetDlgItem(IDC_CONSOLE);
	m_Command.m_hWnd = GetDlgItem(IDC_COMMAND);
	return TRUE;
}

void InvalidParameter(const char* pName)
{
	MessageBox(NULL, pName, "Invalid Parameter", MB_OK);
}

LRESULT CMainView::OnRunCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	char szText[1000];
	m_Command.GetWindowText(szText, sizeof(szText));

	{
		std::string Text = szText;
		Text = trim(Text);
		if(Text.size()==0) {
			m_Command.SetWindowText("");
			return 0L;
		}
	}

	char szCmdName[100];
	const char* pString = GetCmdBuilder().GetTokenIdentity(szText, szCmdName, sizeof(szCmdName));

	if(!pString) {
		MessageBox("invalid command");
		return 0L;
	}

	if(strcmp(szCmdName, "connect")==0) {
		char szAddress[100];
		pString = GetCmdBuilder().GetTokenString(pString, szAddress, sizeof(szAddress));
		if(!pString) {
			InvalidParameter(szCmdName);
		} else {
			if(GetClient()->Available()) GetClient()->Disconnect();
			if(!GetClient()->Connect(szAddress)) {
				MessageBox("Failed to connect server");
			} else {
				m_Command.SetWindowText("");
			}
		}
		return 0L;
	}
	if(strcmp(szCmdName, "disconnect")==0) {
		if(GetClient()->Available()) GetClient()->Disconnect();
		m_Command.SetWindowText("");
		return 0L;
	}

	const void* pData;
	unsigned int nLength;
	pData = GetCmdBuilder().ParseString(szText, nLength);
	if(!pData) {
		InvalidParameter(szCmdName);
		return 0L;
	}
	if(!GetClient()->Available()) {
		MessageBox("not connect");
		return 0L;
	}
	GetClient()->SendData(pData, nLength);

	m_Command.SetWindowText("");
	//m_Console.AppendText(szText);
	//m_Console.AppendText("\n");
	return 0L;
}

LRESULT CMainView::OnClearLog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	m_Console.SetWindowText("");
	return 0L;
}

void CMainView::OnConnect()
{
	m_Console.AppendText("Connected!\n");
}

void CMainView::OnData(const void* pData, unsigned int nSize)
{
	m_Console.AppendText("OnData\n");
}

void CMainView::OnDisconnect()
{
	m_Console.AppendText("Disconnect!\n");
}
