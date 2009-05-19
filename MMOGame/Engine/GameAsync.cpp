#include <windows.h>
#include <process.h>

#include "GameAsync.h"

#define WM_SG_DATABASE		(WM_USER+12)

static void gameadb_thread_proc(void* arg);
static DWORD g_dwGameADBThreadId = 0;
static DWORD WINAPI CGameAPC_Procedure(LPVOID lpParameter);

// Async Procedure Call
DWORD WINAPI CGameAPC_Procedure(LPVOID lpParameter)
{
	((CGameAPC*)lpParameter)->Execute();
	return 0;
}

CGameAPC::CGameAPC(IGameLoop* pGameLoop)
{
	m_pGameLoop = pGameLoop;
}

CGameAPC::~CGameAPC()
{
}

bool CGameAPC::Queue()
{
	return QueueUserWorkItem(CGameAPC_Procedure, this, WT_EXECUTEDEFAULT)?true:false;
}

// Async Database Write OR Read
CGameAsyncDB::CGameAsyncDB(IGameLoop* pGameLoop)
{
	m_pGameLoop = pGameLoop;
}

CGameAsyncDB::~CGameAsyncDB()
{
}

bool CGameAsyncDB::Queue()
{
	return PostThreadMessage(g_dwGameADBThreadId, WM_SG_DATABASE, (WPARAM)this, 0)?true:false;
}

void gameadb_thread_proc(void* arg)
{
	MSG msg;
	::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

	g_dwGameADBThreadId = GetCurrentThreadId();

	while(::GetMessage(&msg, NULL, 0, 0)) {
		if(msg.message==WM_QUIT) break;

		switch(msg.message) {
		case WM_SG_DATABASE:
			((CGameAsyncDB*)msg.wParam)->Execute();
			break;
		default:
			break;
		}
	}
}

bool GameAsync_Init()
{
	_beginthread(gameadb_thread_proc, 0, NULL);
	while(!g_dwGameADBThreadId) SwitchToThread();
	return true;
}

bool GameAsync_Final()
{
	while(!::PostThreadMessage(g_dwGameADBThreadId, WM_QUIT, 0, 0)) SwitchToThread();
	HANDLE hThread = OpenThread(SYNCHRONIZE , FALSE, g_dwGameADBThreadId);
	if(hThread) {
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
	return true;
}

