
#include <winsock2.h>
#include <windows.h>

#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <assert.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <queue>

#include "CmdData.h"
#include "GameLoop.h"

// Game FES
class CWinGameFES;

static CWinGameFES* epmap[100] = { NULL };
static unsigned int epmap_count = 0;
static CRITICAL_SECTION epmap_cs;

#define WM_SG_SENDDATA		(WM_USER+10)
#define WM_SG_DISCONNECT	(WM_USER+11)
#define WM_SG_DATABASE		(WM_USER+12)

typedef struct SendBuf {
	unsigned int nSize;
	char aData[1];
} SendBuf;

static void output_thread_proc(void* arg);
static void gameadb_thread_proc(void* arg);
static DWORD g_dwGameADBThreadId = 0;
static void gameloop_thread_proc(void* arg);
static DWORD WINAPI CGameAPC_Procedure(LPVOID lpParameter);

// Game FES Thread
class CWinGameFES : public IGameFES
{
public:
	CWinGameFES(unsigned int nIndex, unsigned int nIp, unsigned short nPort) {
		m_nThreadId = 0;
		m_nIndex = nIndex;
		m_nIp = nIp;
		m_nPort = nPort;
	}
	virtual ~CWinGameFES() {
	}

	virtual bool SendData(unsigned int nFESSeq, const void* pData, unsigned int nSize) {
		SendBuf* pBuf;
		pBuf = (SendBuf*)malloc(sizeof(SendBuf) + nSize);
		if(!pBuf) return false;
		pBuf->nSize = nSize;
		memcpy(pBuf->aData, pData, nSize);
		while(!::PostThreadMessage(m_nThreadId, WM_SG_SENDDATA, (WPARAM)nFESSeq, (LPARAM)pBuf)) SwitchToThread();
		return true;
	}
	virtual bool Disconnect(unsigned int nFESSeq) {
		while(!::PostThreadMessage(m_nThreadId, WM_SG_DISCONNECT, (WPARAM)nFESSeq, 0)) SwitchToThread();
		return true;
	}

	void Loop() {
		MSG msg;
		::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

		in_addr addr;
		RPC_BINDING_HANDLE hBinding = NULL;
		char szProtocol[20] = "ncacn_ip_tcp";
		char szHost[100];
		char szPort[100];

		addr.S_un.S_addr = GetIP();
		strcpy(szHost, inet_ntoa(addr));
		sprintf(szPort, "%u", GetPort());
		RPC_CSTR strBinding;
		RpcStringBindingComposeA(NULL, (RPC_CSTR)szProtocol, (RPC_CSTR)szHost, (RPC_CSTR)szPort, NULL, &strBinding);
		RpcBindingFromStringBindingA(strBinding, &hBinding);
		RpcStringFreeA(&strBinding);

		while(::GetMessage(&msg, NULL, 0, 0)) {
			if(msg.message==WM_QUIT) break;

			if(msg.message==WM_SG_SENDDATA) {
				// senddata(xxx, uid, ((SendBuf*)lParam)->aData, ((SendBuf*)lParam)->nSize);
				free((void*)msg.lParam);
			}

			if(msg.message==WM_SG_DISCONNECT) {
				// disconnect(xxx, uid);
			}
		}

		if(hBinding) {
			RpcBindingFree(&hBinding);
		}
	}
	void Quit() {
		HANDLE hThread;
		hThread = OpenThread(SYNCHRONIZE, FALSE, m_nThreadId);
		while(!::PostThreadMessage(m_nThreadId, WM_QUIT, 0, 0)) SwitchToThread();
		if(hThread) {
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}
	}

	DWORD GetThreadId() {
		return m_nThreadId;
	}
	unsigned int GetIndex() {
		return m_nIndex;
	}
	unsigned int GetIP() {
		return m_nIp;
	}
	unsigned short GetPort() {
		return m_nPort;
	}

private:
	DWORD m_nThreadId;
	unsigned int m_nIndex;
	unsigned int m_nIp;
	unsigned short m_nPort;
};

void output_thread_proc(void* arg)
{
	((CWinGameFES*)arg)->Loop();
}


CGameLoopRecorder::CGameLoopRecorder(IGameLoopCallback* pCallback)
{
	m_pCallback = pCallback;
	m_hLogHandle = NULL;
}

CGameLoopRecorder::~CGameLoopRecorder()
{
}

bool CGameLoopRecorder::Open(const char* pLogPath)
{
	if(m_hLogHandle) return false;
	m_hLogHandle = fopen(pLogPath, "wb");
	return m_hLogHandle!=NULL;
}

bool CGameLoopRecorder::Close()
{
	if(!m_hLogHandle) return false;
	fclose(m_hLogHandle);
	m_hLogHandle = NULL;
	return true;
}

bool CGameLoopRecorder::Play(const char* pLogPath)
{
	FILE* hLog;
	hLog = fopen(pLogPath, "rb");
	if(hLog==NULL) return false;

	unsigned int nCmd, nWho, nSize;
	static char szBuf[10*1024];
	for(;;) {
		fread(&nCmd, sizeof(nCmd), 1, hLog);
		fread(&nWho, sizeof(nWho), 1, hLog);
		fread(&nSize, sizeof(nSize), 1, hLog);
		if(feof(hLog)) break;
		if(nSize>0) fread(szBuf, nSize, 1, hLog);

		printf("nCmd=%u nWho=%u nSize=%u\n", nCmd, nWho, nSize);

		if(nCmd==0) {
			assert(nSize==sizeof(unsigned int) + sizeof(unsigned int));
			m_pCallback->Tick(*((unsigned int*)szBuf + 0), *((unsigned int*)szBuf + 1));
		} else {
			CmdData cmd = { nCmd, nWho, nSize, szBuf };
			m_pCallback->Process(&cmd);
		}
	}

	fclose(hLog);
	return true;
}

void CGameLoopRecorder::Process(const CmdData* pCmdData)
{
	assert(pCmdData->nCmd!=0);

	fwrite(&pCmdData->nCmd, sizeof(pCmdData->nCmd), 1, m_hLogHandle);
	fwrite(&pCmdData->nWho, sizeof(pCmdData->nWho), 1, m_hLogHandle);
	fwrite(&pCmdData->nSize, sizeof(pCmdData->nSize), 1, m_hLogHandle);
	fwrite(pCmdData->pData, pCmdData->nSize, 1, m_hLogHandle);

	printf("nCmd=%u nWho=%u nSize=%u\n", pCmdData->nCmd, pCmdData->nWho, pCmdData->nSize);

	m_pCallback->Process(pCmdData);
}

void CGameLoopRecorder::Tick(unsigned int nCurrent, unsigned int nDelta)
{
	unsigned int nCmd = 0;
	unsigned int nWho = 0;
	unsigned int nSize = sizeof(nCurrent) + sizeof(nDelta);
	fwrite(&nCmd, sizeof(nCmd), 1, m_hLogHandle);
	fwrite(&nWho, sizeof(nWho), 1, m_hLogHandle);
	fwrite(&nSize, sizeof(nSize), 1, m_hLogHandle);
	fwrite(&nCurrent, sizeof(nCurrent), 1, m_hLogHandle);
	fwrite(&nDelta, sizeof(nDelta), 1, m_hLogHandle);

	printf("nCmd=%u nWho=%u nSize=%u\n", nCmd, nWho, nSize);

	m_pCallback->Tick(nCurrent, nDelta);
}

void CGameLoopRecorder::OnStart()
{
	m_pCallback->OnStart();
}

void CGameLoopRecorder::OnShutdown()
{
	m_pCallback->OnShutdown();
}

// GameLoop
class CWinGameLoop : public IGameLoop
{
public:
	CWinGameLoop(IGameLoopCallback* pCallback) {
		m_pCallback = pCallback;
		m_dwThreadId = 0;
		m_nMsgQ = 0;
		InitializeCriticalSection(&m_csMsgQ);
		m_nGameAPC = 0;
		m_nGameADB = 0;
	}
	virtual ~CWinGameLoop() {
		DeleteCriticalSection(&m_csMsgQ);
	}
	virtual void Release() {
		delete this;
	}

	void OnGameAPCCreate(CGameAPC* pAPC) {
		InterlockedIncrement((LONG*)&m_nGameAPC);
	}
	void OnGameAPCDestroy(CGameAPC* pAPC) {
		InterlockedDecrement((LONG*)&m_nGameAPC);
	}
	void OnGameADBCreate(CGameAsyncDB* pADB) {
		InterlockedIncrement((LONG*)&m_nGameADB);
	}
	void OnGameADBDestroy(CGameAsyncDB* pADB) {
		InterlockedDecrement((LONG*)&m_nGameADB);
	}

	virtual bool OpenOutputFile(const char* pLogFile) {
		return true;
	}

	virtual bool CloseOutputFile() {
		return true;
	}

	virtual bool Start(unsigned int nMinTime) {
		m_nMinTime = nMinTime;
		_beginthread(gameloop_thread_proc, 0, this);
		while(!m_dwThreadId) SwitchToThread();
		return true;
	}

	virtual bool Stop() {
		return PushMsg(0, 0, NULL, 0);
	}

	virtual void Wait() {
		HANDLE hThread;
		hThread = OpenThread(SYNCHRONIZE, FALSE, m_dwThreadId);
		if(hThread) {
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}
		while(m_nGameAPC>0 || m_nGameADB>0) {
			SwitchToThread();
		}
	}

	virtual bool PushMsg(unsigned int nCmd, unsigned int nWho, const void* pData, unsigned int nSize) {
		CmdData Data = { nCmd, nWho, nSize, NULL};
		if(nSize) {
			Data.pData = malloc(nSize);
			if(!Data.pData) return false;
			memcpy(Data.pData, pData, nSize);
		}

		EnterCriticalSection(&m_csMsgQ);
		m_MsgQ[m_nMsgQ?1:0].push(Data);
		LeaveCriticalSection(&m_csMsgQ);

		return true;
	}

	void Run() {
		unsigned int nMsgQ;
		unsigned int nTimeE, nTimeS;
		bool bQuit = false;

		m_dwThreadId = GetCurrentThreadId();

		m_pCallback->OnStart();

		nTimeE = GetTickCount();
		nTimeS = nTimeE;
		for(;;) {
			nTimeE = GetTickCount();
			if(nTimeE-nTimeS>=m_nMinTime) {
				m_pCallback->Tick(nTimeE, nTimeE-nTimeS);
				nTimeS = nTimeE;
			} else {
				SwitchToThread();
			}

			EnterCriticalSection(&m_csMsgQ);
			nMsgQ = m_nMsgQ;
			m_nMsgQ = nMsgQ?0:1;
			LeaveCriticalSection(&m_csMsgQ);

			while(!m_MsgQ[nMsgQ?1:0].empty()) {
				if(m_MsgQ[nMsgQ?1:0].front().nCmd==0) {
					bQuit = true;
					break;
				}

				m_pCallback->Process(&m_MsgQ[nMsgQ?1:0].front());
				if(m_MsgQ[nMsgQ?1:0].front().pData) {
					free((void*)(m_MsgQ[nMsgQ?1:0].front().pData));
				}
				m_MsgQ[nMsgQ?1:0].pop();
			}
			if(bQuit) break;
		}

		m_pCallback->OnShutdown();
	}

private:
	IGameLoopCallback* m_pCallback;
	DWORD m_dwThreadId;
	unsigned int m_nMinTime;
	CRITICAL_SECTION m_csMsgQ;
	std::queue<CmdData> m_MsgQ[2];
	unsigned int m_nMsgQ;
	unsigned int m_nGameAPC, m_nGameADB;
};

bool GameLoop_Init()
{
	InitializeCriticalSectionAndSpinCount(&epmap_cs, 0x80000400);
	_beginthread(gameadb_thread_proc, 0, NULL);
	while(!g_dwGameADBThreadId) SwitchToThread();
	return true;
}

bool GameLoop_Final()
{
	for(int i=0; i<sizeof(epmap)/sizeof(epmap[0]); i++) {
		if(!epmap[i]) continue;
		epmap[i]->Quit();
		delete epmap[i];
	}

	while(!::PostThreadMessage(g_dwGameADBThreadId, WM_QUIT, 0, 0)) SwitchToThread();

	HANDLE hThread = OpenThread(SYNCHRONIZE , FALSE, g_dwGameADBThreadId);
	if(hThread) {
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
	DeleteCriticalSection(&epmap_cs);
	return true;
}

IGameLoop* GameLoop_Create(IGameLoopCallback* pCallback)
{
	return new CWinGameLoop(pCallback);
}

IGameFES* GameLoop_GetFES(unsigned int nIp, unsigned int short nPort)
{
	IGameFES* pFES = NULL;
	EnterCriticalSection(&epmap_cs);
	for(unsigned int i=0; i<sizeof(epmap)/sizeof(epmap[0]); i++) {
		if(epmap[i]->GetIP()==nIp && epmap[i]->GetPort()==nPort)
			pFES = epmap[i];
	}
	if(!pFES) {
		epmap[epmap_count] = new CWinGameFES(epmap_count, nIp, nPort);
		_beginthread(output_thread_proc, 0, epmap[epmap_count]);
		while(!epmap[epmap_count]->GetThreadId()) SwitchToThread();
		pFES = epmap[epmap_count];
		epmap_count++;
	}
	LeaveCriticalSection(&epmap_cs);
	return pFES;
}

// Async Procedure Call
DWORD WINAPI CGameAPC_Procedure(LPVOID lpParameter)
{
	((CGameAPC*)lpParameter)->Execute();
	return 0;
}

CGameAPC::CGameAPC(IGameLoop* pGameLoop)
{
	m_pGameLoop = pGameLoop;
	((CWinGameLoop*)m_pGameLoop)->OnGameAPCCreate(this);
}

CGameAPC::~CGameAPC()
{
	((CWinGameLoop*)m_pGameLoop)->OnGameAPCDestroy(this);
}

bool CGameAPC::Queue()
{
	return QueueUserWorkItem(CGameAPC_Procedure, this, WT_EXECUTEDEFAULT)?true:false;
}

// Async Database Write OR Read
CGameAsyncDB::CGameAsyncDB(IGameLoop* pGameLoop)
{
	m_pGameLoop = pGameLoop;
	((CWinGameLoop*)m_pGameLoop)->OnGameADBCreate(this);
}

CGameAsyncDB::~CGameAsyncDB()
{
	((CWinGameLoop*)m_pGameLoop)->OnGameADBDestroy(this);
}

bool CGameAsyncDB::Queue()
{
	return PostThreadMessage(g_dwGameADBThreadId, WM_SG_DATABASE, (WPARAM)this, 0)?true:false;
}

void gameloop_thread_proc(void* arg)
{
	CWinGameLoop* pGameLoop = (CWinGameLoop*)arg;
	pGameLoop->Run();
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
