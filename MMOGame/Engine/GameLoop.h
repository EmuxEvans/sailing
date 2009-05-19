#pragma once

typedef struct FESClientData {
	unsigned int nSeq;
	unsigned int nIP;
	unsigned short nPort;
} FESClientData;

class IGameFES;
class IGameLoopCallback;
class IGameLoop;

// Game FES
class IGameFES
{
public:
	virtual ~IGameFES() {}

	virtual bool SendData(unsigned int nFESSeq, const void* pData, unsigned int nSize) = 0;
	virtual bool Disconnect(unsigned int nFESSeq) = 0;
};

// Game Loop
class IGameLoopCallback
{
public:
	virtual ~IGameLoopCallback() {}

	virtual void Process(const CmdData* pCmdData) = 0;
	virtual void Tick(unsigned int nCurrent, unsigned int nDelta) = 0;

	virtual void OnStart() = 0;
	virtual void OnShutdown() = 0;
};

class CGameLoopRecorder : public IGameLoopCallback
{
public:
	CGameLoopRecorder(IGameLoopCallback* pCallback);
	virtual ~CGameLoopRecorder();

	bool Open(const char* pLogPath);
	bool Close();

	virtual void Process(const CmdData* pCmdData);
	virtual void Tick(unsigned int nCurrent, unsigned int nDelta);

	virtual void OnStart();
	virtual void OnShutdown();

private:
	IGameLoopCallback* m_pCallback;
	FILE* m_hLogHandle;
};

class IGameLoop
{
public:
	virtual ~IGameLoop() {}
	virtual void Release() = 0;

	virtual bool Playback(const char* pLogFileName) = 0;

	virtual bool Start(unsigned int nMinTime) = 0;
	virtual bool Stop() = 0;
	virtual void Wait() = 0;
	virtual bool PushMsg(unsigned int nCmd, unsigned int nWho, const void* pData, unsigned int nSize) = 0;
};

bool GameLoop_Init();
bool GameLoop_Final();
IGameLoop* GameLoop_Create(IGameLoopCallback* pCallback);
IGameFES* GameLoop_GetFES(unsigned int nIp, unsigned int short nPort);
