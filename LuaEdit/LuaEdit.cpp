// LuaEdit.cpp : main source file for LuaEdit.exe
//

#include "stdafx.h"

#include "resource.h"

#include <skates/skates.h>
#include "LuaDebugClient.h"

#include "AboutDlg.h"
#include "DropFileHandler.h"
#include "FileManager.h"
#include "DialogWindow.h"
#include "DebugHostWindow.h"
#include "CommandWindow.h"
#include "MainFrm.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;

	if(wndMain.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

TCHAR g_szLuaHostPath[MAX_PATH] = _T(".\\");
TCHAR g_szLuaHostArgs[200] = "127.0.0.1:1980";

//ILuaDebugClient* CreateLuaDebugClient();
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	if(!InitializeLuaDebugClient("127.0.0.1:1980")) {
		::MessageBox(NULL, _T("InitializeLuaDebugClient"), _T("ERROR"), MB_OK);
		return 0;
	}

	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));
	Scintilla_RegisterClasses(hInstance);


	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	Scintilla_ReleaseResources();
	::CoUninitialize();
	FinalizeLuaDebugClient();

	return nRet;
}

#include "include\DockImpl.cpp"
#include "include\DialogItemTemplate.cpp"
#include "include\DialogTemplate.cpp"
#include "include\DialogLayout.cpp"
