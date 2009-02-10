// LuaEditView.cpp : implementation of the CLuaEditView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "SciLexerEdit.h"
#include "LuaEditView.h"

CLuaEditView::CLuaEditView()
{
}

CLuaEditView::~CLuaEditView()
{
}

BOOL CLuaEditView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

void CLuaEditView::Init()
{
	CWindowImpl<CLuaEditView, CSciLexerEdit>::Init();
	SetFontname(STYLE_DEFAULT, "Lucida Console");
	SetFontheight(STYLE_DEFAULT, 9);
	SetLexer(SCLEX_LUA);
	SetKeyWords(0, "break do end else elseif function if local nil not or repeat return then until while");
}
