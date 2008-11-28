#pragma once

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

#define GAMEROOM_MATCH				(1<<0)	// 匹配
#define GAMEROOM_NOTMATCH			(1<<1)	// 不匹配
#define GAMEROOM_SELF				(1<<2)	// 包含自己
#define GAMEROOM_EXCEPT				(1<<3)	// 除了自己

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
class CGameRoom : public IGameChannel
{
public:
	CGameRoom(IGameRoomController<TGameUser, TGameRoom, TGameMember>* pController);
	virtual ~CGameRoom();

	virtual bool Join(IGameUser* pUser);
	virtual void OnData(unsigned int nCIdx, const void* pData, int nSize);
	virtual void Disconnect(unsigned int nCIdx);

	void MemberAttach(TGameMember* pMember);
	void MemberDetach(TGameMember* pMember);

	int GetMemberCount();
	TGameMember* GetMember(int nIndex);
	TGameMember* GetMember(unsigned int nCIdx);
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
	CGameMember(TGameUser* pUser, unsigned int nUIdx, TGameRoom* pRoom, unsigned int nCIdx);
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
	unsigned int	m_nCIdx;
	unsigned int	m_nMask;
	TGameUser*		m_pUser;
	unsigned int	m_nUIdx;
};
