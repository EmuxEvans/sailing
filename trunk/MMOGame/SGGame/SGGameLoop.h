#pragma once

class CSGAreaActor;
class CSGPlayer;
class CSGArea;

class CSGConnection : public CGameConnection
{
public:
	CSGConnection(unsigned int nUserId, IGameFES* pFES, unsigned int nFESSeq);
	virtual ~CSGConnection();

	virtual void OnData(const void* pData, unsigned int nSize);

	CSGPlayer* GetPlayer();

private:
	CSGPlayer* m_pPlayer;
};

class CSGGameLoopCallback : public IGameLoopCallback
{
public:
	static CSGGameLoopCallback* GetSingleton();
	static void Cleanup();

	static unsigned int AllocActorId(CSGAreaActor* pActor);
	static void FreeActorId(CSGAreaActor* pActor, unsigned int nActorId);

	static CSGPlayer* GetPlayer(unsigned int nUserId);
	static CSGAreaActor* GetActor(unsigned int nActorId);
	static CSGArea* GetArea(unsigned int nAreaId);

protected:
	CSGGameLoopCallback();
	~CSGGameLoopCallback();

public:
	virtual CGameConnection* CreateConnection(unsigned int nUserId, IGameFES* pFES, unsigned int nFESSeq);

	virtual void Process(const CmdData* pCmdData);
	virtual void Tick(unsigned int nCurrent, unsigned int nDelta);

	virtual void OnStart();
	virtual void OnShutdown();
};
