#pragma once

class IMsgLoop;

// Async Procedure Call
class CAPC
{
protected:
	CAPC(IMsgLoop* pMsgLoop);
	virtual ~CAPC();

public:
	bool Queue();
	virtual void Execute() = 0;

private:
	IMsgLoop* m_pMsgLoop;
};

// Async Database Write OR Read
class CAsyncDB
{
protected:
	CAsyncDB(IMsgLoop* pMsgLoop);
	virtual ~CAsyncDB();

public:
	bool Queue();
	virtual void Execute() = 0;

	IMsgLoop* GetMsgLoop() {
		return m_pMsgLoop;
	}

private:
	IMsgLoop* m_pMsgLoop;
};


bool Async_Init();
bool Async_Final();
