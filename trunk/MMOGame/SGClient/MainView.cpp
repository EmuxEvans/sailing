// SGClientView.cpp : implementation of the CSGClientView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "..\Engine\CmdData.h"
#include "..\SGGame\SGCmdCode.h"

#include "SGClient.h"
#include "MainView.h"
#include "MainFrm.h"

extern "C" {
#include <skates/lua/lua.h>
#include <skates/lua/lauxlib.h>
#include <skates/lua/lualib.h>
#include <skates/lua/tolua++.h>
};
#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/misc.h>

#include "SGClientCmdSet.h"
#include "SGServerCmdSet.h"

static lua_State* L = NULL;
static CMainView* g_pMainView = NULL;
static CSGClientCmdSet myClientCmdSet;
static CSGServerCmdSet myServerCmdSet;
static const char* lua_script = 
	"function onconnect()" "\n"
	"	output(\"OnConnect\\n\")" "\n"
	"end" "\n"
	"function ondisconnect()" "\n"
	"	output(\"OnDisconnect\\n\")" "\n"
	"end" "\n"
	"function ondata(args)" "\n"
	"	output(\"OnData \" .. args.CmdName .. \"\\n\")" "\n"
	"end" "\n"
;

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

static int lua_wait(lua_State* L)
{
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
	//luaopen_io(L);
	//luaopen_os(L);
	//luaopen_math(L);
	//luaopen_debug(L);
	//luaopen_package(L);

	lua_pushstring(L, "connect");
	lua_pushcfunction(L, lua_oncall);
	lua_rawset(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "disconnect");
	lua_pushcfunction(L, lua_oncall);
	lua_rawset(L, LUA_GLOBALSINDEX);

	for(int l=0; l<myClientCmdSet.GetCmdCount(); l++) {
		lua_pushstring(L, myClientCmdSet.GetCmd(l)->m_Name.c_str());
		lua_pushcfunction(L, lua_oncall);
		lua_rawset(L, LUA_GLOBALSINDEX);
	}

	lua_pushstring(L, "output");
	lua_pushcfunction(L, lua_output);
	lua_rawset(L, LUA_GLOBALSINDEX);

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
	g_pMainView = NULL;
	lua_close(L);

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
		::SetWindowText(GetDlgItem(IDC_COMMAND), "");
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

void CMainView::OnConnect()
{
	lua_getglobal(L, "onconnect");
	if(lua_pcall(L, 0, 0, 0)!=0) {
		Output(lua_tostring(L, -1));
		Output("\n");
		lua_pop(L, 1);
	}
}

void CMainView::OnData(const void* pData, unsigned int nSize)
{
	CDataReader data(pData, nSize);
	const CmdInfo* pCmdInfo = myServerCmdSet.GetCmd(data.GetValue<unsigned short>());
	if(!pCmdInfo) {
		MessageBox("invalid data");
		return;
	}

	lua_getglobal(L, "ondata");

	lua_newtable(L);

	lua_pushstring(L, "CmdName");
	lua_pushstring(L, pCmdInfo->m_Name.c_str());
	lua_rawset(L, -3);
	lua_pushstring(L, "CmdCode");
	lua_pushinteger(L, (lua_Integer)pCmdInfo->m_Code);
	lua_rawset(L, -3);

	for(int l=0; l<(int)pCmdInfo->m_Args.size(); l++) {
		lua_pushstring(L, pCmdInfo->m_Args[l].m_Name.c_str());

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
				lua_pushstring(L, "this is a struct");
				break;
			default:
				assert(0);
				return;
			}
		}
		lua_rawset(L, -3);
	}

	if(lua_pcall(L, 1, 0, 0)!=0) {
		Output(lua_tostring(L, -1));
		Output("\n");
		lua_pop(L, 1);
	}
}

void CMainView::OnDisconnect()
{
	lua_getglobal(L, "ondisconnect");
	if(lua_pcall(L, 0, 0, 0)!=0) {
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

	if(strcmp(ar.name, "connect")==0) {
		if(GetClient()->Available()) GetClient()->Disconnect();
		char address[100] = "127.0.0.1:1980";
		if(lua_isstring(L, -1)) {
			strcpy(address, lua_tostring(L, -1));
		} else {
			lua_getglobal(L, "default_server");
			if(lua_isstring(L, -1)) {
				strcpy(address, lua_tostring(L, -1));
			}
			lua_pop(L, -1);
		}
		lua_pushboolean(L, GetClient()->Connect(address)?1:0);
		return 1;
	}
	if(strcmp(ar.name, "disconnect")==0) {
		if(GetClient()->Available()) GetClient()->Disconnect();
		return 0;
	}

	if(!GetClient()->Available()) {
		tolua_error(L, "connection not available", NULL);
		return 0;
	}

	const CmdInfo* pCmdInfo = myClientCmdSet.GetCmd(ar.name);
	assert(pCmdInfo);
	if(!pCmdInfo) {
		char szTxt[1000];
		sprintf(szTxt, "invalid command \"%s\"", ar.name);
		tolua_error(L, szTxt, NULL);
		return 0;
	}

	if(lua_gettop(L)!=(int)pCmdInfo->m_Args.size()) {
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

	GetClient()->SendData(data.GetBuffer(), data.GetLength());	

	return 0;
}

void CMainView::Output(const char* pLine)
{
	bool bEndOfText = m_Console.GetFirstVisableLine()>=m_Console.GetLineCount() - m_Console.LinesOnScreen() - 1;
	m_Console.AppendText(pLine, strlen(pLine));
	if(bEndOfText) {
		int p = m_Console.GetLineCount() - m_Console.LinesOnScreen();
		if(p<0) p = 0;
		m_Console.LineScroll(p-m_Console.GetCurrentLine(), 0);
	}
}
