// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "SGClient.h"
#include "AboutDlg.h"
#include "MainView.h"
#include "MainFrm.h"

CMainFrame::CMainFrame()
{
	m_nConnection = 0;
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();

	MSG msg;
	while(!::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		SwitchToThread();
		m_view.Tick();
	}

	return FALSE;
}

void CMainFrame::SetConnection(int nNumber)
{
	for(int i=0; i<10; i++) {
		UISetCheck(ID_CONN_00+i, i==nNumber?TRUE:FALSE);
	}
	m_nConnection = nNumber;
}

int CMainFrame::GetConnection()
{
	return m_nConnection;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateSimpleStatusBar();

	m_hWndClient = m_view.Create(m_hWnd);

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	SetConnection(m_nConnection);
	return 0;
}

LRESULT CMainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	m_view.Clear();
	bHandled = FALSE;
	return 0L;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: add code to initialize document

	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnConnClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SetConnection(wID-ID_CONN_00);
	return 0;
}

#include "..\Engine\CmdInfo.h"
#include "..\SGCommon\SGCmdSet.h"

CSGCmdSetManage sets;

LRESULT CMainFrame::OnGenHFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog a(FALSE, _T("*.h"), _T("out.h"), 0, _T("Include File (*.h)\0*.h\0"));
	if(a.DoModal()!=IDOK) return 0;
	FILE* fp;
	fp = _tfopen(a.m_szFileName, _T("wt"));
	if(!fp) return 0;

	_ftprintf(fp, _T("\n"));
	_ftprintf(fp, _T("#ifdef __OUT_NETHOOK_CALL\n"));
	for(int cmd=0; cmd<sets.GetClientCmdSet().GetCmdCount(); cmd++) {
		_ftprintf(fp, _T("	virtual void %s("), sets.GetClientCmdSet().GetCmd(cmd)->m_Name);
		for(int arg=0; arg<(int)sets.GetClientCmdSet().GetCmd(cmd)->m_Args.size(); arg++) {
			if(arg>0) _ftprintf(fp, _T(", "));
			switch(sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Type&0xfff) {
			case CMDARG_TYPE_CHAR:		_ftprintf(fp, _T("%s"), _T("char")); break;
			case CMDARG_TYPE_SHORT:		_ftprintf(fp, _T("%s"), _T("short")); break;
			case CMDARG_TYPE_INT:		_ftprintf(fp, _T("%s"), _T("int")); break;
			case CMDARG_TYPE_BYTE:		_ftprintf(fp, _T("%s"), _T("unsigned char")); break;
			case CMDARG_TYPE_WORD:		_ftprintf(fp, _T("%s"), _T("unsigned short")); break;
			case CMDARG_TYPE_DWORD:		_ftprintf(fp, _T("%s"), _T("unsigned int")); break;
			case CMDARG_TYPE_FLOAT:		_ftprintf(fp, _T("%s"), _T("float")); break;
			case CMDARG_TYPE_STRING:	_ftprintf(fp, _T("%s"), _T("const char*")); break;
			case CMDARG_TYPE_STRUCT:	_ftprintf(fp, _T("const %s*"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_StructName); break;
			}
			_ftprintf(fp, _T(" %s"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Name);
			if(sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Type&CMDARG_TYPE_ARRAY) {
				if(sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Type!=CMDARG_TYPE_STRUCT) {
					_ftprintf(fp, _T("[]"));
				}
			}
		}
		_ftprintf(fp, _T(") {\n"));
		_ftprintf(fp, _T("		char __buf[12*1024];\n"));
		_ftprintf(fp, _T("		unsigned int __len = 0;\n"));
		for(int arg=0; arg<(int)sets.GetClientCmdSet().GetCmd(cmd)->m_Args.size(); arg++) {
			if(sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Type&CMDARG_TYPE_ARRAY) {
				assert(0);
				return 0l;
			}
			switch(sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Type) {
			case CMDARG_TYPE_CHAR:
				_ftprintf(fp, _T("		memcpy(__buf+__len, &%s, sizeof(char));\n"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Name);
				_ftprintf(fp, _T("		__len += sizeof(char);\n"));
				break;
			case CMDARG_TYPE_SHORT:
				_ftprintf(fp, _T("		memcpy(__buf+__len, &%s, sizeof(short));\n"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Name);
				_ftprintf(fp, _T("		__len += sizeof(short);\n"));
				break;
			case CMDARG_TYPE_INT:
				_ftprintf(fp, _T("		memcpy(__buf+__len, &%s, sizeof(int));\n"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Name);
				_ftprintf(fp, _T("		__len += sizeof(int);\n"));
				break;
			case CMDARG_TYPE_BYTE:
				_ftprintf(fp, _T("		memcpy(__buf+__len, &%s, sizeof(unsigned char));\n"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Name);
				_ftprintf(fp, _T("		__len += sizeof(unsigned char);\n"));
				break;
			case CMDARG_TYPE_WORD:
				_ftprintf(fp, _T("		memcpy(__buf+__len, &%s, sizeof(unsigned short));\n"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Name);
				_ftprintf(fp, _T("		__len += sizeof(unsigned short);\n"));
				break;
			case CMDARG_TYPE_DWORD:
				_ftprintf(fp, _T("		memcpy(__buf+__len, &%s, sizeof(unsigned int));\n"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Name);
				_ftprintf(fp, _T("		__len += sizeof(unsigned int);\n"));
				break;
			case CMDARG_TYPE_FLOAT:
				_ftprintf(fp, _T("		memcpy(__buf+__len, &%s, sizeof(float));\n"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Name);
				_ftprintf(fp, _T("		__len += sizeof(float);\n"));
				break;
			case CMDARG_TYPE_STRING:
				_ftprintf(fp, _T("		memcpy(__buf+__len, %s, strlen(%s)+1);\n"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Name, sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Name);
				_ftprintf(fp, _T("		__len += (unsigned int)strlen(%s) + 1;\n"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Name);
				break;
			case CMDARG_TYPE_STRUCT:
				_ftprintf(fp, _T("		memcpy(__buf+__len, &%s, sizeof(%s));\n"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_Name, sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_StructName);
				_ftprintf(fp, _T("		__len += sizeof(%s);\n"), sets.GetClientCmdSet().GetCmd(cmd)->m_Args[arg].m_StructName);
				break;
			}
		}
		_ftprintf(fp, _T("		Send(%d, __buf, __len);\n"), sets.GetClientCmdSet().GetCmd(cmd)->m_Code);
		_ftprintf(fp, _T("	}\n"));
	}
	_ftprintf(fp, _T("#endif\n"));

	_ftprintf(fp, _T("\n"));
	_ftprintf(fp, _T("#ifdef __OUT_NETHOOK_CALLBACK\n"));
	for(int cmd=0; cmd<sets.GetServerCmdSet().GetCmdCount(); cmd++) {
		_ftprintf(fp, _T("	virtual void %s("), sets.GetServerCmdSet().GetCmd(cmd)->m_Name);
		for(int arg=0; arg<(int)sets.GetServerCmdSet().GetCmd(cmd)->m_Args.size(); arg++) {
			if(arg>0) _ftprintf(fp, _T(", "));
			if(sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_Type&CMDARG_TYPE_ARRAY) {
				_ftprintf(fp, _T("const "));
			} else if(sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_Type==CMDARG_TYPE_STRUCT) {
				_ftprintf(fp, _T("const "));
			}
			switch(sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_Type&0xfff) {
			case CMDARG_TYPE_CHAR:		_ftprintf(fp, _T("%s"), _T("char")); break;
			case CMDARG_TYPE_SHORT:		_ftprintf(fp, _T("%s"), _T("short")); break;
			case CMDARG_TYPE_INT:		_ftprintf(fp, _T("%s"), _T("int")); break;
			case CMDARG_TYPE_BYTE:		_ftprintf(fp, _T("%s"), _T("unsigned char")); break;
			case CMDARG_TYPE_WORD:		_ftprintf(fp, _T("%s"), _T("unsigned short")); break;
			case CMDARG_TYPE_DWORD:		_ftprintf(fp, _T("%s"), _T("unsigned int")); break;
			case CMDARG_TYPE_FLOAT:		_ftprintf(fp, _T("%s"), _T("float")); break;
			case CMDARG_TYPE_STRING:	_ftprintf(fp, _T("%s"), _T("const char*")); break;
			case CMDARG_TYPE_STRUCT:	_ftprintf(fp, _T("%s*"), sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_StructName); break;
			}
			if(sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_Type&CMDARG_TYPE_ARRAY) {
				if((sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_Type&0xfff)!=CMDARG_TYPE_STRUCT) {
					_ftprintf(fp, _T("*"));
				}
			}
			_ftprintf(fp, _T(" %s"), sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_Name);
		}
		_ftprintf(fp, _T(") = 0;\n"));
	}
	_ftprintf(fp, _T("#endif\n"));

	_ftprintf(fp, _T("\n"));
	_ftprintf(fp, _T("#ifdef __OUT_NETHOOK_DISPATCHER\n"));
	_ftprintf(fp, _T("void OnData(const void* __buf, unsigned int __len) {\n"));
	_ftprintf(fp, _T("	unsigned int __cur = 2;\n"));
	for(int cmd=0; cmd<sets.GetServerCmdSet().GetCmdCount(); cmd++) {
		_ftprintf(fp, _T("	if(*((const unsigned short*)__buf)==%d) {\n"), sets.GetServerCmdSet().GetCmd(cmd)->m_Code);

		for(int arg=0; arg<(int)sets.GetServerCmdSet().GetCmd(cmd)->m_Args.size(); arg++) {
			switch(sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_Type) {
			case CMDARG_TYPE_CHAR:
				_ftprintf(fp, _T("\t\tchar a%d = get_%s(__buf, __len, __cur);\n"), arg, _T("char"));
				break;
			case CMDARG_TYPE_SHORT:
				_ftprintf(fp, _T("\t\tshort a%d = get_%s(__buf, __len, __cur);\n"), arg, _T("short"));
				break;
			case CMDARG_TYPE_INT:
				_ftprintf(fp, _T("\t\tint a%d = get_%s(__buf, __len, __cur);\n"), arg, _T("int"));
				break;
			case CMDARG_TYPE_BYTE:
				_ftprintf(fp, _T("\t\tunsigned char a%d = get_%s(__buf, __len, __cur);\n"), arg, _T("byte"));
				break;
			case CMDARG_TYPE_WORD:
				_ftprintf(fp, _T("\t\tunsigned short a%d = get_%s(__buf, __len, __cur);\n"), arg, _T("word"));
				break;
			case CMDARG_TYPE_DWORD:
				_ftprintf(fp, _T("\t\tunsigned int a%d = get_%s(__buf, __len, __cur);\n"), arg, _T("dword"));
				break;
			case CMDARG_TYPE_FLOAT:
				_ftprintf(fp, _T("\t\tfloat a%d = get_%s(__buf, __len, __cur);\n"), arg, _T("float"));
				break;
			case CMDARG_TYPE_STRING:
				_ftprintf(fp, _T("\t\tconst char* a%d = get_string(__buf, __len, __cur);\n"), arg);
				break;
			case CMDARG_TYPE_STRUCT:
				_ftprintf(fp, _T("\t\tconst %s* a%d = (const %s*)get_struct(__buf, __len, __cur, sizeof(%s));\n"),
					sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_StructName,
					arg,
					sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_StructName,
					sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_StructName);
				break;
			case CMDARG_TYPE_CHAR|CMDARG_TYPE_ARRAY:
				_ftprintf(fp, _T("\t\tconst char* a%d = get_array_%s(__buf, __len, __cur);\n"), arg, _T("char"));
				break;
			case CMDARG_TYPE_SHORT|CMDARG_TYPE_ARRAY:
				_ftprintf(fp, _T("\t\tconst short* a%d = get_array_%s(__buf, __len, __cur);\n"), arg, _T("short"));
				break;
			case CMDARG_TYPE_INT|CMDARG_TYPE_ARRAY:
				_ftprintf(fp, _T("\t\tconst int* a%d = get_array_%s(__buf, __len, __cur);\n"), arg, _T("int"));
				break;
			case CMDARG_TYPE_BYTE|CMDARG_TYPE_ARRAY:
				_ftprintf(fp, _T("\t\tconst unsigned char* a%d = get_array_%s(__buf, __len, __cur);\n"), arg, _T("byte"));
				break;
			case CMDARG_TYPE_WORD|CMDARG_TYPE_ARRAY:
				_ftprintf(fp, _T("\t\tconst unsigned short* a%d = get_array_%s(__buf, __len, __cur);\n"), arg, _T("word"));
				break;
			case CMDARG_TYPE_DWORD|CMDARG_TYPE_ARRAY:
				_ftprintf(fp, _T("\t\tconst unsigned int* a%d = get_array_%s(__buf, __len, __cur);\n"), arg, _T("dword"));
				break;
			case CMDARG_TYPE_FLOAT|CMDARG_TYPE_ARRAY:
				_ftprintf(fp, _T("\t\tconst float* a%d = get_array_%s(__buf, __len, __cur);\n"), arg, _T("float"));
				break;
			case CMDARG_TYPE_STRUCT|CMDARG_TYPE_ARRAY:
				_ftprintf(fp, _T("\t\tconst %s* a%d = (const %s*)get_array_struct(__buf, __len, __cur, sizeof(%s));\n"),
					sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_StructName,
					arg,
					sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_StructName,
					sets.GetServerCmdSet().GetCmd(cmd)->m_Args[arg].m_StructName);
				break;
			}
		}

		_ftprintf(fp, _T("		%s("), sets.GetServerCmdSet().GetCmd(cmd)->m_Name);
		for(int arg=0; arg<(int)sets.GetServerCmdSet().GetCmd(cmd)->m_Args.size(); arg++) {
			if(arg==0) {
				_ftprintf(fp, _T("a%d"), arg);
			} else {
				_ftprintf(fp, _T(", a%d"), arg);
			}
		}
		_ftprintf(fp, _T(");\n"));
		_ftprintf(fp, _T("	}\n"));
	}
	_ftprintf(fp, _T("}\n"));
	_ftprintf(fp, _T("#endif\n"));

	fclose(fp);
	return 0;
}

#include "..\Engine\PropertySet.h"
#include "..\Engine\PropertyDB.h"

#include "..\SGCommon\SGData.h"

extern int SGDataDef_GetPropertySetCount();
extern IPropertySet* SGDataDef_GetPropertySet(int nIndex);
extern IPropertySet* SGDataDef_GetPropertySet(const char* pName);

LRESULT CMainFrame::OnGenDBFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog a(FALSE, _T("*.sql"), _T("out.sql"), 0, _T("Sql File (*.sql)\0*.sql\0"));
	if(a.DoModal()!=IDOK) return 0;

	FILE* fp;
	char SqlText[1000];
	IPropertyDBConnection* pConnection = CreatePropertyDBConnection("");
	CPropertyDBTable Table(SGDataDef_GetPropertySet("SGPLAYER_INFO"), "SGPLAYER_INFO", CPropertyDBTable::SINGLE);
	fp = _tfopen(a.m_szFileName, _T("wt"));
	if(!fp) return 0;

	pConnection->CreateTable(&Table, SqlText, sizeof(SqlText));
	_fputts(SqlText, fp);

	fclose(fp);

	SGPLAYER_INFO info;
	memset(&info, 0, sizeof(info));
	strcpy(info.nick, "welcome");
	info.sex = 0;

	pConnection->Insert(&Table, 10980, &info);
	pConnection->Delete(&Table, 10980);
	pConnection->Read(&Table, 10980, &info);
	pConnection->Write(&Table, 10980, &info);

	pConnection->Release();
	return 0;
}
