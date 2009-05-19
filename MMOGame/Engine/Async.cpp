#include <windows.h>
#include <process.h>

#include "Async.h"

#define WM_SG_DATABASE		(WM_USER+12)

static void adb_thread_proc(void* arg);
static DWORD g_dwADBThreadId = 0;
static DWORD WINAPI CAPC_Procedure(LPVOID lpParameter);

// Async Procedure Call
DWORD WINAPI CAPC_Procedure(LPVOID lpParameter)
{
	((CAPC*)lpParameter)->Execute();
	return 0;
}

CAPC::CAPC(IMsgLoop* pMsgLoop)
{
	m_pMsgLoop = pMsgLoop;
}

CAPC::~CAPC()
{
}

bool CAPC::Queue()
{
	return QueueUserWorkItem(CAPC_Procedure, this, WT_EXECUTEDEFAULT)?true:false;
}

// Async Database Write OR Read
CAsyncDB::CAsyncDB(IMsgLoop* pMsgLoop)
{
	m_pMsgLoop = pMsgLoop;
}

CAsyncDB::~CAsyncDB()
{
}

bool CAsyncDB::Queue()
{
	return PostThreadMessage(g_dwADBThreadId, WM_SG_DATABASE, (WPARAM)this, 0)?true:false;
}

void adb_thread_proc(void* arg)
{
	MSG msg;
	::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

	g_dwADBThreadId = GetCurrentThreadId();

	while(::GetMessage(&msg, NULL, 0, 0)) {
		if(msg.message==WM_QUIT) break;

		switch(msg.message) {
		case WM_SG_DATABASE:
			((CAsyncDB*)msg.wParam)->Execute();
			break;
		default:
			break;
		}
	}
}

bool Async_Init()
{
	_beginthread(adb_thread_proc, 0, NULL);
	while(!g_dwADBThreadId) SwitchToThread();
	return true;
}

bool Async_Final()
{
	while(!::PostThreadMessage(g_dwADBThreadId, WM_QUIT, 0, 0)) SwitchToThread();
	HANDLE hThread = OpenThread(SYNCHRONIZE , FALSE, g_dwADBThreadId);
	if(hThread) {
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
	return true;
}

