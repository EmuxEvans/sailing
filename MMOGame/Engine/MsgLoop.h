#pragma once

// Game Loop
class IMsgLoopCallback
{
public:
	virtual ~IMsgLoopCallback() {}

	virtual void Process(const CmdData* pCmdData) = 0;
	virtual void Tick(unsigned int nCurrent, unsigned int nDelta) = 0;

	virtual void OnStart() = 0;
	virtual void OnShutdown() = 0;
};

class IMsgLoop
{
public:
	virtual ~IMsgLoop() {}

	virtual bool Playback(IMsgLoopCallback* pCallback, const char* pLogFileName) = 0;

	virtual bool Start(IMsgLoopCallback* pCallback, unsigned int nMinTime, const char* pLogFileName) = 0;
	virtual bool Stop() = 0;
	virtual void Wait() = 0;
	virtual bool PushMsg(unsigned int nCmd, unsigned int nWho, const void* pData, unsigned int nSize) = 0;
};

bool MsgLoop_Init();
bool MsgLoop_Final();
IMsgLoop* MsgLoop_Create();
