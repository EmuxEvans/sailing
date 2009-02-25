#pragma once

class CSGGameLoopCallback : public IGameLoopCallback
{
public:
	CSGGameLoopCallback();
	~CSGGameLoopCallback();

	virtual void Process(const CmdData* pCmdData);
	virtual void Tick(unsigned int nCurrent, unsigned int nDelta);

	virtual void OnStart();
	virtual void OnShutdown();
};
