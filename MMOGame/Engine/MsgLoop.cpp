
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
#include "MsgLoop.h"

class CMsgLoop;
static CMsgLoop* msgloop_list[100] = { NULL };
static unsigned int msgloop_count = 0;
static void msgloop_thread_proc(void* arg);

class CMsgLoopRecorder : public IMsgLoopCallback
{
public:
	CMsgLoopRecorder(IMsgLoopCallback* pCallback);
	virtual ~CMsgLoopRecorder();

	bool Open(const char* pLogPath);
	bool Close();

	virtual void Process(const CmdData* pCmdData);
	virtual void Tick(unsigned int nCurrent, unsigned int nDelta);

	virtual void OnStart();
	virtual void OnShutdown();

private:
	IMsgLoopCallback* m_pCallback;
	FILE* m_hLogHandle;
};

CMsgLoopRecorder::CMsgLoopRecorder(IMsgLoopCallback* pCallback)
{
	m_pCallback = pCallback;
	m_hLogHandle = NULL;
}

CMsgLoopRecorder::~CMsgLoopRecorder()
{
}

bool CMsgLoopRecorder::Open(const char* pLogPath)
{
	if(m_hLogHandle) return false;
	m_hLogHandle = fopen(pLogPath, "wb");
	return m_hLogHandle!=NULL;
}

bool CMsgLoopRecorder::Close()
{
	if(!m_hLogHandle) return false;
	fclose(m_hLogHandle);
	m_hLogHandle = NULL;
	return true;
}

void CMsgLoopRecorder::Process(const CmdData* pCmdData)
{
	assert(pCmdData->nCmd!=0);

	fwrite(&pCmdData->nCmd, sizeof(pCmdData->nCmd), 1, m_hLogHandle);
	fwrite(&pCmdData->nWho, sizeof(pCmdData->nWho), 1, m_hLogHandle);
	fwrite(&pCmdData->nSize, sizeof(pCmdData->nSize), 1, m_hLogHandle);
	fwrite(pCmdData->pData, pCmdData->nSize, 1, m_hLogHandle);
	m_pCallback->Process(pCmdData);
}

void CMsgLoopRecorder::Tick(unsigned int nCurrent, unsigned int nDelta)
{
	unsigned int nCmd = 0;
	unsigned int nWho = 0;
	unsigned int nSize = sizeof(nCurrent) + sizeof(nDelta);
	fwrite(&nCmd, sizeof(nCmd), 1, m_hLogHandle);
	fwrite(&nWho, sizeof(nWho), 1, m_hLogHandle);
	fwrite(&nSize, sizeof(nSize), 1, m_hLogHandle);
	fwrite(&nCurrent, sizeof(nCurrent), 1, m_hLogHandle);
	fwrite(&nDelta, sizeof(nDelta), 1, m_hLogHandle);
	m_pCallback->Tick(nCurrent, nDelta);
}

void CMsgLoopRecorder::OnStart()
{
	m_pCallback->OnStart();
}

void CMsgLoopRecorder::OnShutdown()
{
	m_pCallback->OnShutdown();
}

// GameLoop
class CMsgLoop : public IMsgLoop
{
public:
	CMsgLoop() {
		m_pCallback = NULL;
		m_pRecorder = NULL;
		m_dwThreadId = 0;
		m_nMsgQ = 0;
		InitializeCriticalSection(&m_csMsgQ);
		msgloop_list[msgloop_count++] = this;
	}
	virtual ~CMsgLoop() {
		while(!m_MsgQ[0].empty()) {
			if(m_MsgQ[0].front().nSize>0) {
				free(m_MsgQ[0].front().pData);
			}
			m_MsgQ[0].pop();
		}
		while(!m_MsgQ[1].empty()) {
			if(m_MsgQ[1].front().nSize>0) {
				free(m_MsgQ[1].front().pData);
			}
			m_MsgQ[1].pop();
		}
		DeleteCriticalSection(&m_csMsgQ);
	}

	virtual bool Playback(IMsgLoopCallback* pCallback, const char* pLogFileName) {
		FILE* hLog;
		hLog = fopen(pLogFileName, "rb");
		if(hLog==NULL) return false;

		m_pCallback = pCallback;

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

	virtual bool Start(IMsgLoopCallback* pCallback, unsigned int nMinTime, const char* pLogFileName) {
		if(pLogFileName) {
			m_pRecorder = new CMsgLoopRecorder(pCallback);
			if(!m_pRecorder->Open(pLogFileName)) {
				delete m_pRecorder;
				return false;
			}
			m_pCallback = m_pRecorder;
		} else {
			m_pCallback = pCallback;
		}
		m_nMinTime = nMinTime;
		_beginthread(msgloop_thread_proc, 0, this);
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
		if(m_pRecorder) {
			m_pRecorder->Close();
			delete m_pRecorder;
			m_pRecorder = NULL;
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
	IMsgLoopCallback* m_pCallback;
	CMsgLoopRecorder* m_pRecorder;
	DWORD m_dwThreadId;
	unsigned int m_nMinTime;
	CRITICAL_SECTION m_csMsgQ;
	std::queue<CmdData> m_MsgQ[2];
	unsigned int m_nMsgQ;
};

bool MsgLoop_Init()
{
	return true;
}

bool MsgLoop_Final()
{
	for(int i=0; i<sizeof(msgloop_list)/sizeof(msgloop_list[0]); i++) {
		if(msgloop_list[i]) {
			delete msgloop_list[i];
			msgloop_list[i] = NULL;
		}
	}
	return true;
}

IMsgLoop* MsgLoop_Create()
{
	return new CMsgLoop();
}

void msgloop_thread_proc(void* arg)
{
	CMsgLoop* pGameLoop = (CMsgLoop*)arg;
	pGameLoop->Run();
}
