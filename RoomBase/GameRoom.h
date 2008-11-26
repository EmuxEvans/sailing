#pragma once

template<class TGameUser>
class IGameUserCallback
{
public:
	virtual void OnConnect(TGameUser* pUser) = NULL;
	virtual void OnDisconnect(TGameUser* pUser) = NULL;
	virtual void OnData(TGameUser* pUser, const void* pData, int nSize) = NULL;
};

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
class IGameRoomCallback
{
public:
	virtual void OnCreate(TGameRoom* pRoom) = NULL;
	virtual void OnDestroy(TGameRoom* pRoom) = NULL;
	virtual bool MemberPrepareJoin(TGameRoom* pRoom, TGameUser* pUser) = NULL;
	virtual void MemberJoin(TGameRoom* pRoom, TGameMember* pMember) = NULL;
	virtual void MemberLeave(TGameRoom* pRoom, TGameMember* pMember) = NULL;
	virtual void MemberOndata(TGameRoom* pRoom, TGameMember* pMember, const void* pData, int nSize) = NULL;
	virtual void OnTimer(TGameRoom* pRoom, int nTimerId) = NULL;
};

class IGameUser;
class IGameRoom;

class IGameUser
{
public:
	virtual void Connect() = NULL;
	virtual void Disconnect() = NULL;
	virtual void OnData(const void* pData, int nSize) = NULL;
	virtual void SendData(const void* pData, int nSize) = NULL;
	virtual bool BindRoom(IGameRoom* pRoom, int nUIdx) = NULL;
};

class IGameRoom
{
public:
	virtual void Join(IGameUser* pUser) = NULL;
	virtual void OnData(unsigned int nUIdx, const void* pData, int nSize) = NULL;
	virtual void Disconnect(unsigned int nUIdx) = NULL;
};

template<class TGameUser>
class CGameUser : public IGameUser
{
public:
	CGameUser();
	virtual ~CGameUser();

	IGameUserCallback<TGameUser>* GetCallback();
	void SetCallback(IGameUserCallback<TGameUser>* pCallback);

	virtual void Connect();
	virtual void Disconnect();
	virtual void OnData(const void* pData, int nSize);
	virtual void SendData(const void* pData, int nSize);
	virtual bool BindRoom(IGameRoom* pRoom, int nUIdx);

private:
	struct {
		IGameRoom*		pRoom;
		unsigned int	nUIdx;
	}								m_pRoomList[20];
	IGameUserCallback<TGameUser>*	m_pCallback;
};

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
class CGameRoom : public IGameRoom
{
public:
	CGameRoom();
	virtual ~CGameRoom();

	int GetMemberCount();
	TGameMember* GetMember(int nIndex);
	TGameMember* GetMember(const char* pNick);
	TGameMember* GetNextMember(TGameMember* pMember);
	TGameMember* GetPrevMember(TGameMember* pMember);

private:
	TGameMember* m_pMemberList[nMemberMax];
};

template<class TGameUser, class TGameRoom, class TGameMember>
class CGameMember
{
public:
	CGameMember(TGameUser* pUser, TGameRoom* pRoom, unsigned int nUIdx);
	virtual ~CGameMember();

	TGameUser* GetGameUser();
	TGameRoom* GetGameRoom();
	unsigned int GetUidx();

	void SetMask(unsigned int nMask);
	unsigned int GetMask() const;

private:
	TGameUser*		m_pUser;
	TGameRoom*		m_pRoom;
	unsigned int	m_nUIdx;
	unsigned int	m_nMask;
};
