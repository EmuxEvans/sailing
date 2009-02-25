#pragma once

class CSGAreaActor;
class CSGPlayer;

class CSGGameLoopCallback : public IGameLoopCallback
{
public:
	static CSGGameLoopCallback* GetSingleton();
	static void Cleanup();

	static unsigned int AllocActorId(CSGAreaActor* pActor);
	static void FreeActorId(CSGAreaActor* pActor, unsigned int nActorId);

	static CSGPlayer* GetPlayer(unsigned int nUserId);
	static CSGAreaActor* GetActor(unsigned int nActorId);

protected:
	CSGGameLoopCallback();
	~CSGGameLoopCallback();

public:
	virtual void Process(const CmdData* pCmdData);
	virtual void Tick(unsigned int nCurrent, unsigned int nDelta);

	virtual void OnStart();
	virtual void OnShutdown();
};
