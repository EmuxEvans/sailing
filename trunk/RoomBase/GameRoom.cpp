#include <string.h>

#include "GameRoom.h"

template<class TGameUser>
CGameUser<TGameUser>::CGameUser()
{
	memset(m_pRoomList, 0, sizeof(m_pRoomList));
}

template<class TGameUser>
CGameUser<TGameUser>::~CGameUser()
{
}

template<class TGameUser>
IGameUserCallback<TGameUser>* CGameUser<TGameUser>::GetCallback()
{
	return m_pCallback;
}

template<class TGameUser>
void CGameUser<TGameUser>::SetCallback(IGameUserCallback<TGameUser>* pCallback)
{
	m_pCallback = pCallback;
}

template<class TGameUser>
void CGameUser<TGameUser>::Connect()
{
	m_pCallback->OnConnect(this);
}

template<class TGameUser>
void CGameUser<TGameUser>::Disconnect()
{
	for(int idx=0; idx<sizeof(m_pRoomList)/sizeof(m_pRoomList[0]); idx++) {
		if(!m_pRoomList[idx]) continue;
		m_pRoomList[idx].pRoom->Disconnect(m_pRoomList[idx].nUIdx);
	}
	m_pCallback->OnDisconnect(this);
}

template<class TGameUser>
void CGameUser<TGameUser>::OnData(const void* pData, int nSize)
{
}

template<class TGameUser>
void CGameUser<TGameUser>::SendData(const void* pData, int nSize)
{
}

template<class TGameUser>
bool CGameUser<TGameUser>::BindRoom(IGameRoom* pRoom, int nUIdx)
{
	for(int idx=0; idx<sizeof(m_pRoomList)/sizeof(m_pRoomList[0]); idx++) {
		if(m_pRoomList[idx].pRoom!=NULL) continue;
		m_pRoomList[idx].pRoom = pRoom;
		m_pRoomList[idx].nUIdx = nUIdx;
		return true;
	}
	return false;
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::CGameRoom()
{
	memset(m_pMemberList, 0, sizeof(m_pMemberList));
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::~CGameRoom()
{
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
int CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetMemberCount()
{
	return 0;
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
TGameMember* CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetMember(int nIndex)
{
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
TGameMember* CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetMember(const char* pNick)
{
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
TGameMember* CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetNextMember(TGameMember* pMember)
{
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
TGameMember* CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetPrevMember(TGameMember* pMember)
{
}

template<class TGameUser, class TGameRoom, class TGameMember>
CGameMember<TGameUser, TGameRoom, TGameMember>::CGameMember(TGameUser* pUser, TGameRoom* pRoom, unsigned int nUIdx)
{
}

template<class TGameUser, class TGameRoom, class TGameMember>
CGameMember<TGameUser, TGameRoom, TGameMember>::~CGameMember()
{
}

template<class TGameUser, class TGameRoom, class TGameMember>
TGameUser* CGameMember<TGameUser, TGameRoom, TGameMember>::GetGameUser()
{
}

template<class TGameUser, class TGameRoom, class TGameMember>
TGameRoom* CGameMember<TGameUser, TGameRoom, TGameMember>::GetGameRoom()
{
}

template<class TGameUser, class TGameRoom, class TGameMember>
unsigned int CGameMember<TGameUser, TGameRoom, TGameMember>::GetUidx()
{
	return 0;
}

template<class TGameUser, class TGameRoom, class TGameMember>
void CGameMember<TGameUser, TGameRoom, TGameMember>::SetMask(unsigned int nMask)
{
}

template<class TGameUser, class TGameRoom, class TGameMember>
unsigned int CGameMember<TGameUser, TGameRoom, TGameMember>::GetMask() const
{
	return 0;
}
