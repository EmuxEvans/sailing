#pragma once

class IGameLoop;

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


bool GameAsync_Init();
bool GameAsync_Final();
