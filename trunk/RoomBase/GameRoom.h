#pragma once

template<class TGameUser>
class IGameUserController
{
public:
	virtual void OnConnect(TGameUser* pUser) = NULL;
	virtual void OnDisconnect(TGameUser* pUser) = NULL;
	virtual void OnData(TGameUser* pUser, const void* pData, int nSize) = NULL;
};

template<class TGameUser, class TGameRoom, class TGameMember>
class IGameRoomController
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
	virtual bool BindRoom(unsigned int& nIndex, IGameRoom* pRoom, int nRIdx) = NULL;
	virtual bool UnbindRoom(unsigned int nIndex, IGameRoom* pRoom, int nRIdx) = NULL;
};

class IGameRoom
{
public:
	virtual bool Join(IGameUser* pUser) = NULL;
	virtual void OnData(unsigned int nRIdx, const void* pData, int nSize) = NULL;
	virtual void Disconnect(unsigned int nRIdx) = NULL;
};

template<class TGameUser>
class CGameUser : public IGameUser
{
public:
	CGameUser(IGameUserController<TGameUser>* pController);
	virtual ~CGameUser();

	virtual void Connect();
	virtual void Disconnect();
	virtual void OnData(const void* pData, int nSize);
	virtual void SendData(const void* pData, int nSize);
	virtual bool BindRoom(unsigned int& nIndex, IGameRoom* pRoom, int nRIdx);
	virtual bool UnbindRoom(unsigned int nIndex, IGameRoom* pRoom, int nRIdx);

private:
	struct {
		IGameRoom*		pRoom;
		unsigned int	nRIdx;
	}								m_DynChannel[20];
	struct {
		IGameRoom*		pRoom;
		unsigned int	nRIdx;
	}								m_ptaChannel[20];
	IGameUserController<TGameUser>*	m_pController;
};

#define GAMEROOM_MATCH				(1<<0)	// 匹配
#define GAMEROOM_NOTMATCH			(1<<1)	// 不匹配
#define GAMEROOM_SELF				(1<<2)	// 包含自己
#define GAMEROOM_EXCEPT				(1<<3)	// 除了自己

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
class CGameRoom : public IGameRoom
{
public:
	CGameRoom(IGameRoomController<TGameUser, TGameRoom, TGameMember>* pController);
	virtual ~CGameRoom();

	virtual bool Join(IGameUser* pUser);
	virtual void OnData(unsigned int nRIdx, const void* pData, int nSize);
	virtual void Disconnect(unsigned int nRIdx);

	void MemberAttach(TGameMember* pMember);
	void MemberDetach(TGameMember* pMember);

	int GetMemberCount();
	TGameMember* GetMember(int nIndex);
	TGameMember* GetMember(unsigned int nRIdx);
	TGameMember* GetNextMember(TGameMember* pMember);
	TGameMember* GetPrevMember(TGameMember* pMember);
	TGameMember* GetNextMember(TGameMember* pMember, unsigned int nMask, unsigned int nValue);
	TGameMember* GetPrevMember(TGameMember* pMember, unsigned int nMask, unsigned int nValue);

	void Send(TGameMember* pMember, unsigned int nMask, unsigned int nValue, unsigned int nFlags, const void* pData, int nSize);

private:
	TGameMember* m_pMemberList[nMemberMax];
	IGameRoomController<TGameUser, TGameRoom, TGameMember>*	m_pController;
};

template<class TGameUser, class TGameRoom, class TGameMember>
class CGameMember
{
public:
	CGameMember(TGameUser* pUser, unsigned int nUIdx, TGameRoom* pRoom, unsigned int nRIdx);
	virtual ~CGameMember();

	TGameUser* GetGameUser();
	unsigned int GetGameUIdx();
	void SetGameUser(TGameUser* pUser, unsigned int nUIdx);


	TGameRoom* GetGameRoom();
	unsigned int GetRIdx();

	void SetMask(unsigned int nMask);
	unsigned int GetMask() const;

private:
	TGameRoom*		m_pRoom;
	unsigned int	m_nRIdx;
	unsigned int	m_nMask;
	TGameUser*		m_pUser;
	unsigned int	m_nUIdx;
};

unsigned int GenGameSeq();
