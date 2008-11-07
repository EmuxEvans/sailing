// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "AboutDlg.h"
#include <skates\skates.h>

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());
	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

CAttachHostDlg::CAttachHostDlg()
{
	_tcscpy(m_szAddress, "127.0.0.1:2000");
}

LRESULT CAttachHostDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	::SetWindowText(GetDlgItem(IDC_HOST_ADDRESS), m_szAddress);
	return 0;
}

LRESULT CAttachHostDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(wID==IDOK) {
		::GetWindowText(GetDlgItem(IDC_HOST_ADDRESS), m_szAddress, sizeof(m_szAddress));
		SOCK_ADDR sa;
		if(!sock_str2addr(m_szAddress, &sa)) return 0;
	}
	EndDialog(wID);
	return 0;
}

LRESULT CFindDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CFindDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CFindDlg::OnFindNext(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CReplaceDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CReplaceDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CReplaceDlg::OnFindNext(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CReplaceDlg::OnReplace(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CReplaceDlg::OnReplaceAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

CGoToLineDlg::CGoToLineDlg(int nLineMax, int nLineNumber)
{
	m_nLineMax = nLineMax;
	m_nLineNumber = nLineNumber;
}

LRESULT CGoToLineDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	char szTmp[100];
	sprintf(szTmp, "&Line number (1-%d):", m_nLineMax);
	::SetWindowTextA(GetDlgItem(IDC_GOTOLINE_STATIC), szTmp);
	sprintf(szTmp, "%d", m_nLineNumber);
	::SetWindowTextA(GetDlgItem(IDC_LINE_NUMBER), szTmp);
	return 0;
}

LRESULT CGoToLineDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(wID==IDOK) {
		char szTmp[100];
		::GetWindowTextA(GetDlgItem(IDC_LINE_NUMBER), szTmp, sizeof(szTmp));
		m_nLineNumber = atoi(szTmp);
		if(m_nLineNumber<1 || m_nLineNumber>m_nLineMax) {
			MessageBox(_T("Invalid line number"), _T("Error"), MB_OK);
			return 0;
		}
	}
	EndDialog(wID);
	return 0;
}

LRESULT CHostSettingDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CHostSettingDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
