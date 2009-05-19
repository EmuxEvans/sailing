#pragma once

class CSGAreaActor;
class CSGPlayer;
class CSGArea;

class CSGConnection;
class CSGGameLoopCallback;

class IGameFES;

class CSGConnection
{
public:
	CSGConnection(CSGGameLoopCallback* pCallback, unsigned int nUserId, IGameFES* pFES, unsigned int nFESSeq);
	virtual ~CSGConnection();
	void Release();

	CSGGameLoopCallback* GetCallback() {
		return m_pCallback;
	}
	CSGPlayer* GetPlayer() {
		return m_pPlayer;
	}

	unsigned int GetUserId() {
		return m_nUserId;
	}
	IGameFES* GetGameFES() {
		return m_pFES;
	}

	bool SendData(const void* pData, unsigned int nSize);
	//	return m_pFES->SendData(m_nFESSeq, pData, nSize);
	bool Disconnect();
	//	return m_pFES->Disconnect(m_nFESSeq);

	void OnConnect();
	void OnData(const void* pData, unsigned int nSize);
	void OnDisconnect();

private:
	CSGGameLoopCallback* m_pCallback;
	CSGPlayer* m_pPlayer;
	unsigned int m_nUserId;
	IGameFES* m_pFES;
	unsigned int m_nFESSeq;
};

class CSGGameLoopCallback : public IMsgLoopCallback
{
	friend class CSGConnection;
	friend class CSGPlayer;
public:
	static CSGGameLoopCallback* GetSingleton();
	static void Cleanup();

	static unsigned int AllocActorId(CSGAreaActor* pActor);
	static void FreeActorId(CSGAreaActor* pActor, unsigned int nActorId);
	static CSGAreaActor* GetActor(unsigned int nActorId);

protected:
	CSGGameLoopCallback();
	~CSGGameLoopCallback();

public:
	virtual CSGConnection* CreateConnection(unsigned int nUserId, IGameFES* pFES, unsigned int nFESSeq);
	CSGConnection* GetConnection(unsigned int nUserId);

	CSGPlayer* GetPlayer(unsigned int nPlayerId);
	CSGPlayer* GetPlayer(const char* pName);
	CSGPlayer* GetNextPlayer(CSGPlayer* pPlayer);

	CSGArea* GetArea(unsigned int nAreaId);

	virtual void Process(const CmdData* pCmdData);
	virtual void Tick(unsigned int nCurrent, unsigned int nDelta);

	virtual void OnStart();
	virtual void OnShutdown();

protected:
	void OnConnectionAttach(unsigned int nUserId, CSGConnection* pConnection);
	void OnConnectionDetach(unsigned int nUserId, CSGConnection* pConnection);
	void OnPlayerAttach(unsigned int nPlayerId, const char* pName, CSGPlayer* pPlayer);
	void OnPlayerDetach(unsigned int nPlayerId, const char* pName, CSGPlayer* pPlayer);
private:
	std::map<unsigned int, CSGConnection*> m_mapConnections;
	std::map<unsigned int, CSGPlayer*> m_mapPlayersById;
	std::map<std::string, CSGPlayer*> m_mapPlayersByName;
	std::map<unsigned int, CSGPlayer*>::iterator m_itrPlayersById;
};
