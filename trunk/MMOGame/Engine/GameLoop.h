#pragma once

typedef struct FESClientData {
	unsigned int nSeq;
	unsigned int nIP;
	unsigned short nPort;
} FESClientData;

class IGameFES;
class CGameConnection;
class CGameAPC;
class CGameAsyncDB;
class IGameLoopCallback;
class IGameLoop;

// Game FES
typedef struct SERVER_ADDRESS {
	unsigned int ip;
	unsigned short port;
} SERVER_ADDRESS;

class IGameFES
{
public:
	virtual ~IGameFES() {}

	virtual bool SendData(unsigned int nFESSeq, const void* pData, unsigned int nSize) = 0;
	virtual bool Disconnect(unsigned int nFESSeq) = 0;
};

// Game Connection
class CGameConnection
{
public:
	CGameConnection(unsigned int nUserId, IGameFES* pFES, unsigned int nFESSeq) {
		m_nUserId = nUserId;
		m_pFES = pFES;
		m_nFESSeq = nFESSeq;
	}
	virtual ~CGameConnection() {
	}

	unsigned int GetUserId() {
		return m_nUserId;
	}
	IGameFES* GetGameFES() {
		return m_pFES;
	}

	bool SendData(const void* pData, unsigned int nSize) {
		return m_pFES->SendData(m_nFESSeq, pData, nSize);
	}
	bool Disconnect() {
		return m_pFES->Disconnect(m_nFESSeq);
	}
	virtual void OnData(const void* pData, unsigned int nSize) = 0;

private:
	unsigned int m_nUserId;
	IGameFES* m_pFES;
	unsigned int m_nFESSeq;
};

// Game Loop
class IGameLoopCallback
{
public:
	virtual ~IGameLoopCallback() {}

	virtual CGameConnection* CreateConnection(unsigned int nUserId, IGameFES* pFES, unsigned int nFESSeq) = 0;

	virtual void Process(const CmdData* pCmdData) = 0;
	virtual void Tick(unsigned int nCurrent, unsigned int nDelta) = 0;

	virtual void OnStart() = 0;
	virtual void OnShutdown() = 0;
};

class IGameLoop
{
public:
	virtual bool Start(unsigned int nMinTime) = 0;
	virtual bool Stop() = 0;
	virtual void Wait() = 0;
	virtual bool PushMsg(unsigned int nCmd, unsigned int nWho, const void* pData, unsigned int nSize) = 0;
};

bool GameLoop_Init();
bool GameLoop_Final();
IGameLoop* GameLoop_Create(IGameLoopCallback* pCallback);
void GameLoop_Destroy(IGameLoop* pLoop);
IGameFES* GameLoop_GetFES(unsigned int nIp, unsigned int short nPort);

// Async Procedure Call
class CGameAPC
{
protected:
	CGameAPC(IGameLoop* pGameLoop);
	virtual ~CGameAPC();

public:
	bool Queue();
	virtual void Execute() = 0;

private:
	IGameLoop* m_pGameLoop;
};

// Async Database Write OR Read
class CGameAsyncDB
{
protected:
	CGameAsyncDB(IGameLoop* pGameLoop);
	virtual ~CGameAsyncDB();

public:
	bool Queue();
	virtual void Execute() = 0;

	IGameLoop* GetGameLoop() {
		return m_pGameLoop;
	}

private:
	IGameLoop* m_pGameLoop;
};
