// SGClientView.cpp : implementation of the CSGClientView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "..\Engine\CmdData.h"
#include "..\SGGame\SGCmdCode.h"

#include "SGClient.h"
#include "SGCmdBuilder.h"
#include "MainView.h"
#include "MainFrm.h"

extern "C" {
#include <skates/lua/lua.h>
#include <skates/lua/lauxlib.h>
#include <skates/lua/lualib.h>
#include <skates/lua/tolua++.h>
};

static lua_State* L = NULL;
static CMainView* g_pMainView = NULL;
static CSGClientCmdSet myClientCmdSet;
static CSGServerCmdSet myServerCmdSet;

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

static int on_luacall(lua_State* L)
{
	return g_pMainView->LuaCallback();
}

CMainView::CMainView()
{
	L = lua_open();

	lua_pushstring(L, "connect");
	lua_pushcfunction(L, on_luacall);
	lua_rawset(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "disconnect");
	lua_pushcfunction(L, on_luacall);
	lua_rawset(L, LUA_GLOBALSINDEX);

	for(int l=0; l<myClientCmdSet.GetCmdCount(); l++) {
		lua_pushstring(L, myClientCmdSet.GetCmd(l)->m_Name.c_str());
		lua_pushcfunction(L, on_luacall);
		lua_rawset(L, LUA_GLOBALSINDEX);
	}
	g_pMainView = this;
}

CMainView::~CMainView()
{
	g_pMainView = NULL;
	lua_close(L);
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

	if(luaL_dostring(L, szText)==0) {
		m_Command.SetWindowText("");
	} else {
		m_Console.AppendText(lua_tostring(L, -1));
		m_Console.AppendText("\n");
		lua_pop(L, 1);
	}

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
	CDataReader data(pData, nSize);
	const CmdInfo* pCmdInfo = myServerCmdSet.GetCmd(data.GetValue<unsigned short>());
	if(!pCmdInfo) {
		MessageBox("invalid data");
		return;
	}

	lua_newtable(L);
	for(int l=0; l<(int)pCmdInfo->m_Args.size(); l++) {
		if(pCmdInfo->m_Args[l].m_Type&CMDARG_TYPE_ARRAY) {
		}

		lua_pushstring(L, pCmdInfo->m_Name.c_str());
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
		}
	}
}

void CMainView::OnDisconnect()
{
	m_Console.AppendText("Disconnect!\n");
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
		if(!GetClient()->Connect("127.0.0.1:1980")) {
			tolua_error(L, "failed to connect", NULL);
		}
		return 0;
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
				if(!lua_isnumber(L, -(l+1))) {
					char szTxt[1000];
					sprintf(szTxt, "invalid parameter type in %s(%d)@%s, BYTE\n", pCmdInfo->m_Args[l].m_Name, l+1, pCmdInfo->m_Name);
					tolua_error(L, szTxt, NULL);
					return 0;
				}
				data.PutValue<unsigned char>((unsigned char)lua_tointeger(L, -(l+1)));
				break;
			}
		case CMDARG_TYPE_DWORD:
			{
				if(!lua_isnumber(L, -(l+1))) {
					char szTxt[1000];
					sprintf(szTxt, "invalid parameter type in %s(%d)@%s, DWORD\n", pCmdInfo->m_Args[l].m_Name, l+1, pCmdInfo->m_Name);
					tolua_error(L, szTxt, NULL);
					return 0;
				}
				data.PutValue<unsigned int>((unsigned int)lua_tointeger(L, -(l+1)));
				break;
			}
		case CMDARG_TYPE_FLOAT:
			{
				if(!lua_isnumber(L, -(l+1))) {
					char szTxt[1000];
					sprintf(szTxt, "invalid parameter type in %s(%d)@%s, FLOAT\n", pCmdInfo->m_Args[l].m_Name, l+1, pCmdInfo->m_Name);
					tolua_error(L, szTxt, NULL);
					return 0;
				}
				data.PutValue<float>((float)lua_tointeger(L, -(l+1)));
				break;
			}
		case CMDARG_TYPE_STRING:
			{
				if(!lua_isstring(L, -(l+1))) {
					char szTxt[1000];
					sprintf(szTxt, "invalid parameter type in %s(%d)@%s, STRING\n", pCmdInfo->m_Args[l].m_Name, l+1, pCmdInfo->m_Name);
					tolua_error(L, szTxt, NULL);
					return 0;
				}
				data.PutString(lua_tostring(L, -(l+1)));
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