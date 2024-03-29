// SGClientView.cpp : implementation of the CSGClientView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

extern "C" {
#include <skates/lua/lua.h>
#include <skates/lua/lauxlib.h>
#include <skates/lua/lualib.h>
#include <skates/lua/tolua++.h>
};
#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/misc.h>

#include "..\Engine\CmdData.h"
#include "..\Engine\CmdInfo.h"
#include "..\SGCommon\SGCode.h"
#include "..\SGCommon\SGData.h"
#include "..\SGCommon\SGCmdSet.h"
#include "..\SGCommon\SGLua.h"

#include "SGClient.h"
#include "MainView.h"
#include "MainFrm.h"

extern CMainFrame* g_pMainFrm;

static lua_State* L = NULL;
static CMainView* g_pMainView = NULL;
static CSGCmdSetManage myCmdSet;
static const char* lua_script = 
	"function onconnect(conn)" "\n"
	"	output(\"OnConnect \"..conn..\"\\n\")" "\n"
	"end" "\n"
	"function ondisconnect(conn)" "\n"
	"	output(\"OnDisconnect \"..conn..\"\\n\")" "\n"
	"end" "\n"
	"function ondata(conn, args)" "\n"
	"	output(\"OnData \" ..conn..\" \".. args.CmdName .. \"\\n\")" "\n"
	"end" "\n"
;
static ISGClient* myClients[10] = { NULL };

ISGClient* GetClient()
{
	return myClients[g_pMainFrm->GetConnection()];
}

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

static int lua_switchto(lua_State* L)
{
	if(!lua_isnumber(L, -1)) {
		tolua_error(L, "output invalid parameter", NULL);
		return 0;		
	}
	int conn = lua_tointeger(L, -1);
	if(conn<0) conn = 0;
	if(conn>=sizeof(myClients)/sizeof(myClients[0])) conn = sizeof(myClients)/sizeof(myClients[0]) - 1;
	g_pMainFrm->SetConnection(conn);
	return 0;
}

static int lua_clear(lua_State* L)
{
	g_pMainView->m_Console.SetText("");
	return 0;
}

static int lua_connect(lua_State* L)
{
	int nConn = GetClient()->GetIndex();
	char address[100] = "";
	switch(lua_gettop(L)) {
	case 0:
		break;
	case 1:
		if(lua_isnumber(L, -1)) {
			nConn = lua_tointeger(L, -1);
		} else if(lua_isstring(L, -1)) {
			strcpy(address, lua_tostring(L, -1));
		} else {
			tolua_error(L, "", NULL);
			return 0;
		}
		break;
	case 2:
		if(!lua_isnumber(L, -2)) {
			tolua_error(L, "invalid parameter", NULL);
			return 0;
		}
		if(!lua_isstring(L, -1)) {
			tolua_error(L, "invalid parameter", NULL);
			return 0;
		}
		nConn = lua_tointeger(L, -2);
		strcpy(address, lua_tostring(L, -1));
		break;
	default:
		tolua_error(L, "invalid parameter", NULL);
		return 0;
	}
	if(address[0]=='\0') {
		lua_getglobal(L, "default_server");
		if(lua_isstring(L, -1)) {
			strcpy(address, lua_tostring(L, -1));
		} else {
			strcpy(address, "127.0.0.1:1980");
		}
		lua_pop(L, -1);
	}
	if(myClients[nConn]->Available()) myClients[nConn]->Disconnect();
	lua_pushboolean(L, myClients[nConn]->Connect(address)?1:0);
	return 1;
}

static int lua_disconnect(lua_State* L)
{
	int nConn = GetClient()->GetIndex();
	char address[100] = "";
	switch(lua_gettop(L)) {
	case 0:
		break;
	case 1:
		if(lua_isnumber(L, -1)) {
			nConn = lua_tointeger(L, -1);
		} else {
			tolua_error(L, "", NULL);
			return 0;
		}
		break;
	default:
		tolua_error(L, "invalid parameter", NULL);
		return 0;
	}
	myClients[nConn]->Disconnect();
	return 0;
}

static int lua_wait(lua_State* L)
{
	return 0;
}

static int lua_oncall(lua_State* L)
{
	return g_pMainView->LuaCallback();
}

static int lua_output(lua_State* L)
{
	if(!lua_isstring(L, -1)) {
		tolua_error(L, "output invalid parameter", NULL);
		return 0;		
	}
	g_pMainView->Output(lua_tostring(L, -1));
	return 0;
}

static std::list<std::string> m_CommandList;

CMainView::CMainView()
{
	L = lua_open();
	luaopen_string(L);
	luaopen_base(L);
	luaopen_table(L);
//	luaopen_io(L);
	luaopen_os(L);
	luaopen_math(L);
	luaopen_debug(L);
//	luaopen_package(L);
	tolua_sglua_open(L);

	lua_pushstring(L, "switchto");
	lua_pushcfunction(L, lua_switchto);
	lua_rawset(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "clear");
	lua_pushcfunction(L, lua_clear);
	lua_rawset(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "connect");
	lua_pushcfunction(L, lua_connect);
	lua_rawset(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "disconnect");
	lua_pushcfunction(L, lua_disconnect);
	lua_rawset(L, LUA_GLOBALSINDEX);

	for(int l=0; l<myCmdSet.GetClientCmdSet().GetCmdCount(); l++) {
		lua_pushstring(L, myCmdSet.GetClientCmdSet().GetCmd(l)->m_Name);
		lua_pushcfunction(L, lua_oncall);
		lua_rawset(L, LUA_GLOBALSINDEX);
	}

	lua_pushstring(L, "output");
	lua_pushcfunction(L, lua_output);
	lua_rawset(L, LUA_GLOBALSINDEX);


	GetCurrentDirectory(sizeof(m_szCurrentDir), m_szCurrentDir);
	char szTxt[10000];

	if(load_textfile("SGClient.lua", szTxt, sizeof(szTxt))>=0) {
		lua_script = szTxt;
	}

	if(luaL_dostring(L, lua_script)!=0) {
		::MessageBox(NULL, lua_tostring(L, -1), "failed to load script", MB_OK);
	}

	g_pMainView = this;
	m_bCommandMode = TRUE;
	m_szFileName[0] = '\0';
	GetCurrentDirectory(sizeof(m_szFileName), m_szFileName);

	for(int l=0; l<sizeof(myClients)/sizeof(myClients[0]); l++) {
		myClients[l] = CreateSGClient(this, l);
	}

	FILE* fp = fopen("SGClient.History.txt", "rt");
	if(fp) {
		char szLine[1000];
		for(;;) {
			if(fgets(szLine, sizeof(szLine), fp)==NULL) break;
			for(int i=0; i<sizeof(szLine); i++) {
				if(szLine[i]<' ' && szLine[i]!='\0') {
					memmove(szLine+i, szLine+i+1, strlen(szLine+i+1)+1);
				}
				if(szLine[i]=='\0') break;
			}
			if(szLine[0]=='\0') continue;

			m_CommandList.push_back(szLine);
		}
		m_CommandList.sort();
		fclose(fp);
	}
}

CMainView::~CMainView()
{
	for(int l=0; l<sizeof(myClients)/sizeof(myClients[0]); l++) {
		myClients[l]->Release();
	}

	g_pMainView = NULL;
	lua_close(L);

	SetCurrentDirectory(m_szCurrentDir);
	FILE* fp = fopen("SGClient.History.txt", "wt");
	if(fp) {
		std::list<std::string>::iterator i;
		for(i=m_CommandList.begin(); i!=m_CommandList.end(); i++) {
			fprintf(fp, "%s\n", (*i).c_str());
		}
		fclose(fp);
	}

}

BOOL CMainView::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainView::OnInitDialog(HWND, LPARAM)
{
	SetMsgHandled(FALSE);
	m_Command.m_hWnd = GetDlgItem(IDC_COMMAND);
	m_Console.m_hWnd = GetDlgItem(IDC_CONSOLE);
	m_Script.m_hWnd = GetDlgItem(IDC_SCRIPT);

	CRect rctDlg, rctCtl;
	GetClientRect(&rctDlg);

	CPoint LT(0, 0);
	::ClientToScreen(m_hWnd, &LT);

	::GetWindowRect(GetDlgItem(IDC_CONSOLE), &rctCtl);
	rctCtl -= LT;
	m_nConsoleWidth = rctDlg.right - rctCtl.Width();
	m_nConsoleHeight = rctDlg.bottom - rctCtl.Height();

	::GetWindowRect(GetDlgItem(IDC_SCRIPT), &rctCtl);
	rctCtl -= LT;
	m_nScriptTop = rctDlg.bottom - rctCtl.top;
	m_nScriptWidth = rctDlg.right - rctCtl.Width();

	::GetWindowRect(GetDlgItem(IDC_COMMAND), &rctCtl);
	rctCtl -= LT;
	m_nBottomTop = rctDlg.bottom - rctCtl.top;
	m_nCommandWidth = rctDlg.right - rctCtl.Width();

	::GetWindowRect(GetDlgItem(IDC_RUNCMD), &rctCtl);
	rctCtl -= LT;
	m_nRunCmdRight = rctDlg.right - rctCtl.left;
	::GetWindowRect(GetDlgItem(IDC_CLRLOG), &rctCtl);
	rctCtl -= LT;
	m_nClrLogRight = rctDlg.right - rctCtl.left;
	::GetWindowRect(GetDlgItem(IDC_MODCHG), &rctCtl);
	rctCtl -= LT;
	m_nModChgRight = rctDlg.right - rctCtl.left;

	std::list<std::string>::iterator i;
	for(i=m_CommandList.begin(); i!=m_CommandList.end(); i++) {
		m_Command.AddString((*i).c_str());
	}
	m_CommandList.sort();

	m_Console.Init();
	m_Console.SetFontname(STYLE_DEFAULT, "Lucida Console");
	m_Console.SetFontheight(STYLE_DEFAULT, 9);
	m_Console.SetLexer(SCLEX_CPP);
	m_Console.SetKeyWords(0, "OnConnect OnData OnDisconnect");
	m_Console.SetDisplayLinenumbers(FALSE);
	m_Console.SetDisplayFolding(FALSE);
	m_Console.SendMessage(SCI_SETINDENTATIONGUIDES, TRUE, 0);
//	m_Console.SetReadOnly(TRUE);

	m_Script.Init();
	m_Script.SetFontname(STYLE_DEFAULT, "Lucida Console");
	m_Script.SetFontheight(STYLE_DEFAULT, 9);
	m_Script.SetLexer(SCLEX_LUA);
	m_Script.SetKeyWords(0, "break do end else elseif function if local nil not or repeat return then until while");
	m_Script.SetDisplayLinenumbers(FALSE);
	//m_Script.SetDisplayFolding(FALSE);


	return TRUE;
}

LRESULT CMainView::OnSize(UINT, WPARAM, LPARAM , BOOL& bHandled)
{
	CRect rctDlg, rctCtl;
	CPoint LT(0, 0);

	GetClientRect(&rctDlg);
	::ClientToScreen(m_hWnd, &LT);

	if(m_bCommandMode) {
		::GetWindowRect(GetDlgItem(IDC_CONSOLE), &rctCtl);
		rctCtl -= LT;
		::SetWindowPos(GetDlgItem(IDC_CONSOLE), NULL,
			0, 0,
			rctDlg.Width() - m_nConsoleWidth, rctDlg.Height() - m_nConsoleHeight + (m_nScriptTop - m_nBottomTop),
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);


		::GetWindowRect(GetDlgItem(IDC_SCRIPT), &rctCtl);
		rctCtl -= LT;
		::SetWindowPos(GetDlgItem(IDC_SCRIPT), NULL, rctCtl.left, 0, 100, rctCtl.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		::ShowWindow(GetDlgItem(IDC_SCRIPT), SW_HIDE);

		::GetWindowRect(GetDlgItem(IDC_LOADTXT), &rctCtl);
		rctCtl -= LT;
		::SetWindowPos(GetDlgItem(IDC_LOADTXT), NULL,
			rctCtl.left, rctDlg.bottom - m_nBottomTop,
			0, 0,
			SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
		::ShowWindow(GetDlgItem(IDC_LOADTXT), SW_HIDE);

		::GetWindowRect(GetDlgItem(IDC_SAVETXT), &rctCtl);
		rctCtl -= LT;
		::SetWindowPos(GetDlgItem(IDC_SAVETXT), NULL,
			rctCtl.left, rctDlg.bottom - m_nBottomTop,
			0, 0,
			SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
		::ShowWindow(GetDlgItem(IDC_SAVETXT), SW_HIDE);

		::GetWindowRect(GetDlgItem(IDC_COMMAND), &rctCtl);
		rctCtl -= LT;
		::SetWindowPos(GetDlgItem(IDC_COMMAND), NULL,
			rctCtl.left, rctDlg.bottom - m_nBottomTop,
			rctDlg.right - m_nCommandWidth, rctCtl.Height(),
			SWP_NOACTIVATE | SWP_NOZORDER);
		::ShowWindow(GetDlgItem(IDC_COMMAND), SW_SHOWNOACTIVATE);

		::SetWindowPos(GetDlgItem(IDC_RUNCMD), NULL,
			rctDlg.right - m_nRunCmdRight, rctDlg.bottom - m_nBottomTop,
			0, 0,
			SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
		::SetWindowPos(GetDlgItem(IDC_CLRLOG), NULL,
			rctDlg.right - m_nClrLogRight, rctDlg.bottom - m_nBottomTop,
			0, 0,
			SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

		::SetWindowText(GetDlgItem(IDC_MODCHG), "<<");
		::SetWindowPos(GetDlgItem(IDC_MODCHG), NULL,
			rctDlg.right - m_nModChgRight, rctDlg.bottom - m_nBottomTop,
			0, 0,
			SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	} else {
		::GetWindowRect(GetDlgItem(IDC_CONSOLE), &rctCtl);
		rctCtl -= LT;
		::SetWindowPos(GetDlgItem(IDC_CONSOLE), NULL,
			0, 0,
			rctDlg.Width() - m_nConsoleWidth, rctDlg.Height() - m_nConsoleHeight,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

		::GetWindowRect(GetDlgItem(IDC_SCRIPT), &rctCtl);
		rctCtl -= LT;
		::SetWindowPos(GetDlgItem(IDC_SCRIPT), NULL,
			rctCtl.left, rctDlg.bottom - m_nScriptTop,
			rctDlg.right - m_nScriptWidth, rctCtl.Height(),
			SWP_NOACTIVATE | SWP_NOZORDER);
		::ShowWindow(GetDlgItem(IDC_SCRIPT), SW_SHOWNOACTIVATE);

		::GetWindowRect(GetDlgItem(IDC_LOADTXT), &rctCtl);
		rctCtl -= LT;
		::SetWindowPos(GetDlgItem(IDC_LOADTXT), NULL,
			rctCtl.left, rctDlg.bottom - m_nBottomTop,
			0, 0,
			SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
		::ShowWindow(GetDlgItem(IDC_LOADTXT), SW_SHOWNOACTIVATE);

		::GetWindowRect(GetDlgItem(IDC_SAVETXT), &rctCtl);
		rctCtl -= LT;
		::SetWindowPos(GetDlgItem(IDC_SAVETXT), NULL,
			rctCtl.left, rctDlg.bottom - m_nBottomTop,
			0, 0,
			SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
		::ShowWindow(GetDlgItem(IDC_SAVETXT), SW_SHOWNOACTIVATE);

		::GetWindowRect(GetDlgItem(IDC_COMMAND), &rctCtl);
		rctCtl -= LT;
		::SetWindowPos(GetDlgItem(IDC_COMMAND), NULL,
			rctCtl.left, rctDlg.bottom - m_nBottomTop,
			rctDlg.right - m_nCommandWidth, rctCtl.Height(),
			SWP_NOACTIVATE | SWP_NOZORDER);
		::ShowWindow(GetDlgItem(IDC_COMMAND), SW_HIDE);

		::SetWindowPos(GetDlgItem(IDC_RUNCMD), NULL,
			rctDlg.right - m_nRunCmdRight, rctDlg.bottom - m_nBottomTop,
			0, 0,
			SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
		::SetWindowPos(GetDlgItem(IDC_CLRLOG), NULL,
			rctDlg.right - m_nClrLogRight, rctDlg.bottom - m_nBottomTop,
			0, 0,
			SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

		::SetWindowText(GetDlgItem(IDC_MODCHG), ">>");
		::SetWindowPos(GetDlgItem(IDC_MODCHG), NULL,
			rctDlg.right - m_nModChgRight, rctDlg.bottom - m_nBottomTop,
			0, 0,
			SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CMainView::OnRunCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	char szText[10000];

	if(m_bCommandMode) {
		::GetWindowText(GetDlgItem(IDC_COMMAND), szText, sizeof(szText));
		for(int i=0; i<sizeof(szText); i++) {
			if(szText[i]<' ' && szText[i]!='\0') {
				memmove(szText+i, szText+i+1, strlen(szText+i+1)+1);
			}
			if(szText[i]=='\0') break;
		}
	} else {
		strcpy(szText, m_Script.GetText());
	}

	std::string Text = szText;
	if(trim(Text).size()==0) {
		return 0L;
	}

	if(luaL_dostring(L, szText)==0) {
		if(m_bCommandMode) {
			::SetWindowText(GetDlgItem(IDC_COMMAND), "");
			if(m_Command.FindString(-1, szText)==-1) {
				m_Command.AddString(szText);
				m_CommandList.push_back(szText);
			}
		}
	} else {
		Output(lua_tostring(L, -1));
		Output("\n");
		lua_pop(L, 1);
	}

	return 0L;
}

LRESULT CMainView::OnClearLog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	m_Console.SetText("");
	return 0L;
}

LRESULT CMainView::OnModeChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	m_bCommandMode = !m_bCommandMode;
	BOOL bHandled = FALSE;
	OnSize(0, 0, 0, bHandled);
	return 0L;
}

LRESULT CMainView::OnLoadText(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	CFileDialog a(TRUE, _T("*.lua"), NULL, OFN_FILEMUSTEXIST, _T("Lua Source File (*.lua)\0*.lua\0"));
	if(a.DoModal()!=IDOK) return 0;

	char szText[100000];

	if(load_textfile(a.m_szFileName, szText, sizeof(szText))<0) {
		MessageBox(a.m_szFileName, "failed to load file");
		return 0L;
	}

	strcpy(m_szFileName, a.m_szFileName);
	m_Script.SetText(szText);
	return 0L;
}

LRESULT CMainView::OnSaveText(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	CFileDialog a(FALSE, _T("*.lua"), m_szFileName, 0, _T("Lua Source File (*.lua)\0*.lua\0"));
	if(a.DoModal()!=IDOK) return 0;

	strcpy(m_szFileName, a.m_szFileName);
	if(save_textfile(a.m_szFileName, m_Script.GetText(), strlen(m_Script.GetText()))<0) {
		MessageBox(a.m_szFileName, "failed to save file");
	}

	return 0L;
}

void CMainView::Tick()
{
	for(int l=0; l<sizeof(myClients)/sizeof(myClients[0]); l++) {
		myClients[l]->Wait();
	}
}

void CMainView::Clear()
{
	for(int l=0; l<sizeof(myClients)/sizeof(myClients[0]); l++) {
		if(myClients[l]->Available()) myClients[l]->Disconnect();
	}
}

void CMainView::OnConnect(ISGClient* pClient)
{
	lua_getglobal(L, "onconnect");
	lua_pushinteger(L, pClient->GetIndex());
	if(lua_pcall(L, 1, 0, 0)!=0) {
		Output(lua_tostring(L, -1));
		Output("\n");
		lua_pop(L, 1);
	}
}

#include "..\Engine\PropertySet.h"

int SGDataDef_GetPropertySetCount();
IPropertySet* SGDataDef_GetPropertySet(int nIndex);
IPropertySet* SGDataDef_GetPropertySet(const char* pName);

bool pushstruct(lua_State* L, const void* pData, const char* pTypeName)
{
	IPropertySet* pSet = SGDataDef_GetPropertySet(pTypeName);
	assert(pSet);
	if(pSet==NULL) return false;

	lua_newtable(L);
	for(int n=0; n<pSet->PropertyCount(); n++) {
		lua_pushstring(L, pSet->GetProperty(n)->GetName());
		switch(pSet->GetProperty(n)->GetType()) {
		case PROPERTY_TYPE_CHAR:
			lua_pushinteger(L, (lua_Integer)(*((const char*)((const char*)pData + pSet->GetProperty(n)->GetOffset()))));
			break;
		case PROPERTY_TYPE_SHORT:
			lua_pushinteger(L, (lua_Integer)(*((const short*)((const char*)pData + pSet->GetProperty(n)->GetOffset()))));
			break;
		case PROPERTY_TYPE_INT:
			lua_pushinteger(L, (lua_Integer)(*((const int*)((const char*)pData + pSet->GetProperty(n)->GetOffset()))));
			break;
		case PROPERTY_TYPE_BYTE:
			lua_pushinteger(L, (lua_Integer)(*((const unsigned char*)((const char*)pData + pSet->GetProperty(n)->GetOffset()))));
			break;
		case PROPERTY_TYPE_WORD:
			lua_pushinteger(L, (lua_Integer)(*((const unsigned short*)((const char*)pData + pSet->GetProperty(n)->GetOffset()))));
			break;
		case PROPERTY_TYPE_DWORD:
			lua_pushinteger(L, (lua_Integer)(*((const unsigned int*)((const char*)pData + pSet->GetProperty(n)->GetOffset()))));
			break;
		case PROPERTY_TYPE_FLOAT:
			lua_pushnumber(L, (lua_Number)(*((const float*)((const char*)pData + pSet->GetProperty(n)->GetOffset()))));
			break;
		case PROPERTY_TYPE_STRING:
			lua_pushstring(L, (const char*)pData + pSet->GetProperty(n)->GetOffset());
			break;
		case PROPERTY_TYPE_STRUCT:
			pushstruct(L, (const char*)pData + pSet->GetProperty(n)->GetOffset(), pSet->GetProperty(n)->GetTypeDefine()->GetName());
			break;
		}
		lua_rawset(L, -3);
	}

	return true;
}

void CMainView::OnData(ISGClient* pClient, const void* pData, unsigned int nSize)
{
	CDataReader data(pData, nSize);
	const CmdInfo* pCmdInfo = myCmdSet.GetServerCmdSet().GetCmd(data.GetValue<unsigned short>());
	if(!pCmdInfo) {
		MessageBox("invalid data");
		return;
	}

	lua_getglobal(L, "ondata");

	lua_pushinteger(L, pClient->GetIndex());
	lua_newtable(L);

	lua_pushstring(L, "CmdName");
	lua_pushstring(L, pCmdInfo->m_Name);
	lua_rawset(L, -3);
	lua_pushstring(L, "CmdCode");
	lua_pushinteger(L, (lua_Integer)pCmdInfo->m_Code);
	lua_rawset(L, -3);

	for(int l=0; l<(int)pCmdInfo->m_Args.size(); l++) {
		lua_pushstring(L, pCmdInfo->m_Args[l].m_Name);

		if(pCmdInfo->m_Args[l].m_Type&CMDARG_TYPE_ARRAY) {
			unsigned short nCount = data.GetValue<unsigned short>();
			lua_newtable(L);
			for(int n=0; n<(int)nCount; n++) {
				lua_pushinteger(L, (lua_Integer)n);
				switch(pCmdInfo->m_Args[l].m_Type) {
				case CMDARG_TYPE_BYTE|CMDARG_TYPE_ARRAY:
					lua_pushinteger(L, (lua_Integer)data.GetValue<unsigned char>());
					break;
				case CMDARG_TYPE_DWORD|CMDARG_TYPE_ARRAY:
					lua_pushinteger(L, (lua_Integer)data.GetValue<unsigned int>());
					break;
				case CMDARG_TYPE_FLOAT|CMDARG_TYPE_ARRAY:
					lua_pushnumber(L, (lua_Number)data.GetValue<float>());
					break;
				case CMDARG_TYPE_STRUCT|CMDARG_TYPE_ARRAY:
					pushstruct(L, data.GetStruct(pCmdInfo->m_Args[l].m_StructSize), pCmdInfo->m_Args[l].m_StructName);
					break;
				default:
					assert(0);
					return;
				}
				lua_rawset(L, -3);
			}
		} else {
			switch(pCmdInfo->m_Args[l].m_Type) {
			case CMDARG_TYPE_BYTE:
				lua_pushinteger(L, (lua_Integer)data.GetValue<unsigned char>());
				break;
			case CMDARG_TYPE_DWORD:
				lua_pushinteger(L, (lua_Integer)data.GetValue<unsigned int>());
				break;
			case CMDARG_TYPE_FLOAT:
				lua_pushnumber(L, (lua_Number)data.GetValue<float>());
				break;
			case CMDARG_TYPE_STRING:
				lua_pushstring(L, data.GetString());
				break;
			case CMDARG_TYPE_STRUCT:
				pushstruct(L, data.GetStruct(pCmdInfo->m_Args[l].m_StructSize), pCmdInfo->m_Args[l].m_StructName);
				break;
			default:
				assert(0);
				return;
			}
		}
		lua_rawset(L, -3);
	}

	if(lua_pcall(L, 2, 0, 0)!=0) {
		Output(lua_tostring(L, -1));
		Output("\n");
		lua_pop(L, 1);
	}
}

void CMainView::OnDisconnect(ISGClient* pClient)
{
	lua_getglobal(L, "ondisconnect");
	lua_pushinteger(L, pClient->GetIndex());
	if(lua_pcall(L, 1, 0, 0)!=0) {
		Output(lua_tostring(L, -1));
		Output("\n");
		lua_pop(L, 1);
	}
}

int CMainView::LuaCallback()
{
	lua_Debug ar;
	memset(&ar, 0, sizeof(ar));
	if(!lua_getstack(L, 0, &ar)) return 0;
	if(!lua_getinfo(L, "n", &ar)) {
		assert(0);
	}
	assert(ar.name);

	const CmdInfo* pCmdInfo = myCmdSet.GetClientCmdSet().GetCmd(ar.name);
	assert(pCmdInfo);
	if(!pCmdInfo) {
		char szTxt[1000];
		sprintf(szTxt, "invalid command \"%s\"", ar.name);
		tolua_error(L, szTxt, NULL);
		return 0;
	}

	int nConn;
	if(lua_gettop(L)==(int)pCmdInfo->m_Args.size()) {
		nConn = GetClient()->GetIndex();
	} else if(lua_gettop(L)==(int)pCmdInfo->m_Args.size()+1) {
		if(!lua_isnumber(L, ((int)pCmdInfo->m_Args.size())*(-1) - 1)) {
			tolua_error(L, "", NULL);
			return 0;
		}
		nConn = lua_tointeger(L, ((int)pCmdInfo->m_Args.size())*(-1) - 1);
		if(nConn<0) nConn = 0;
		if(nConn>=sizeof(myClients)/sizeof(myClients[0])) nConn = sizeof(myClients)/sizeof(myClients[0]) - 1;
	} else {
		char szTxt[1000];
		sprintf(szTxt, "invalid parameter count", ar.name);
		tolua_error(L, szTxt, NULL);
		return 0;
	}

	CDataBuffer<1000> data;
	data.PutValue(pCmdInfo->m_Code);

	for(int l=0; l<(int)pCmdInfo->m_Args.size(); l++) {
		switch(pCmdInfo->m_Args[l].m_Type) {
		case CMDARG_TYPE_BYTE:
			{
				if(!lua_isnumber(L, (((int)pCmdInfo->m_Args.size())*(-1) + l))) {
					char szTxt[1000];
					sprintf(szTxt, "invalid parameter type in %s(%d)@%s, BYTE\n", pCmdInfo->m_Args[l].m_Name, l+1, pCmdInfo->m_Name);
					tolua_error(L, szTxt, NULL);
					return 0;
				}
				data.PutValue<unsigned char>((unsigned char)lua_tointeger(L, (((int)pCmdInfo->m_Args.size())*(-1) + l)));
				break;
			}
		case CMDARG_TYPE_DWORD:
			{
				if(!lua_isnumber(L, (((int)pCmdInfo->m_Args.size())*(-1) + l))) {
					char szTxt[1000];
					sprintf(szTxt, "invalid parameter type in %s(%d)@%s, DWORD\n", pCmdInfo->m_Args[l].m_Name, l+1, pCmdInfo->m_Name);
					tolua_error(L, szTxt, NULL);
					return 0;
				}
				data.PutValue<unsigned int>((unsigned int)lua_tointeger(L, (((int)pCmdInfo->m_Args.size())*(-1) + l)));
				break;
			}
		case CMDARG_TYPE_FLOAT:
			{
				if(!lua_isnumber(L, (((int)pCmdInfo->m_Args.size())*(-1) + l))) {
					char szTxt[1000];
					sprintf(szTxt, "invalid parameter type in %s(%d)@%s, FLOAT\n", pCmdInfo->m_Args[l].m_Name, l+1, pCmdInfo->m_Name);
					tolua_error(L, szTxt, NULL);
					return 0;
				}
				data.PutValue<float>((float)lua_tointeger(L, (((int)pCmdInfo->m_Args.size())*(-1) + l)));
				break;
			}
		case CMDARG_TYPE_STRING:
			{
				if(!lua_isstring(L, (((int)pCmdInfo->m_Args.size())*(-1) + l))) {
					char szTxt[1000];
					sprintf(szTxt, "invalid parameter type in %s(%d)@%s, STRING\n", pCmdInfo->m_Args[l].m_Name, l+1, pCmdInfo->m_Name);
					tolua_error(L, szTxt, NULL);
					return 0;
				}
				data.PutString(lua_tostring(L, (((int)pCmdInfo->m_Args.size())*(-1) + l)));
				break;
			}
		default:
			tolua_error(L, "internal error", NULL);
			return 0;
		}
	}

	if(!myClients[nConn]->Available()) {
		tolua_error(L, "connection not available", NULL);
		return 0;
	}

	myClients[nConn]->SendData(data.GetBuffer(), data.GetLength());	

	return 0;
}

void CMainView::Output(const char* pLine)
{
	OutputDebugString(pLine);
	bool bEndOfText = m_Console.GetFirstVisableLine()>=m_Console.GetLineCount() - m_Console.LinesOnScreen() - 1;
	m_Console.AppendText(pLine, strlen(pLine));
	if(bEndOfText) {
		int p = m_Console.GetLineCount() - m_Console.LinesOnScreen();
		if(p<0) p = 0;
		m_Console.LineScroll(p-m_Console.GetCurrentLine(), 0);
	}
}
