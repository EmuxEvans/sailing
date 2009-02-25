#pragma once

// Game FES
class IGameFES
{
public:
	virtual ~IGameFES() {}

	virtual bool SendData(unsigned int nUserId, const void* pData, unsigned int nSize) = 0;
	virtual bool Disconnect(unsigned int nUserId) = 0;
};

bool GameFES_Init();
bool GameFES_Final();
IGameFES* GameFES_Get(unsigned int nIp, unsigned int short nPort);

// Async Procedure Call
class CGameAPC
{
public:
	static bool Init();
	static bool Final();
	static bool QueueWorkItem(CGameAPC* pAPC);

protected:
	CGameAPC() { }
	virtual ~CGameAPC() { }

public:
	virtual void Execute() = 0;
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






















class CGameClient
{
protected:
	CGameClient(unsigned int nUserId, IGameFES* pFES) {
		m_nUserId = nUserId;
		m_pFES = pFES;
	}
	virtual ~CGameClient() {
	}

public:
	bool SendData(unsigned int nUserId, const void* pData, unsigned int nSize) {
		return m_pFES->SendData(nUserId, pData, nSize);
	}
	bool Disconnect(unsigned int nUserId) {
		return m_pFES->Disconnect(m_nUserId);
	}

	unsigned int GetUserId() const {
		return m_nUserId;
	}
	IGameFES* GetFES() {
		return m_pFES;
	}

private:
	unsigned int m_nUserId;
	IGameFES* m_pFES;
};
