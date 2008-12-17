#pragma once

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::CGameRoom(IGameRoomController<TGameUser, TGameRoom, TGameMember>* pController)
{
	memset(m_pMemberList, 0, sizeof(m_pMemberList));
	m_pController = pController;
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::~CGameRoom()
{
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
bool CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::Join(IGameUser* pUser)
{
	unsigned int nIndex, nCIdx;
	TGameMember* pMember;

	for(nIndex=0; nIndex<sizeof(m_pMemberList)/sizeof(m_pMemberList[0]); nIndex++) {
		if(!m_pMemberList[nIndex]) break;
	}
	if(nIndex==sizeof(m_pMemberList)/sizeof(m_pMemberList[0])) return false;

	if(!m_pController->MemberPrepareJoin((TGameRoom*)this, (TGameUser*)pUser)) {
		return false;
	}

	nCIdx = (GenChannelSeq()<<16|nIndex);
	if(!pUser->BindChannel(this, nCIdx)) {
		return false;
	}

	pMember = new TGameMember((TGameUser*)pUser, (TGameRoom*)this, GenChannelSeq()<<16|nIndex);

	return true;
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
void CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::OnData(unsigned int nCIdx, const void* pData, int nSize)
{
	TGameMember* pMember = GetMember(nCIdx);
	if(!pMember) return;

	m_pController->MemberOndata((TGameRoom*)this, pMember, pData, nSize);
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
void CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::Disconnect(unsigned int nCIdx)
{
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
void CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::MemberAttach(TGameMember* pMember)
{
	unsigned int nIndex = pMember->GetRIdx()&0xffff;
	assert(nIndex<sizeof(m_pMemberList)/sizeof(m_pMemberList[0]));
	assert(m_pMemberList[nIndex]==NULL);

	m_pMemberList[nIndex] = pMember;
	m_pController->MemberJoin((TGameRoom*)this, pMember);
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
void CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::MemberDetach(TGameMember* pMember)
{
	unsigned int nIndex = pMember->GetRIdx()&0xffff;
	assert(nIndex<sizeof(m_pMemberList)/sizeof(m_pMemberList[0]));
	assert(m_pMemberList[nIndex]==pMember);

	m_pController->MemberLeave((TGameRoom*)this, pMember);
	m_pMemberList[nIndex] = NULL;
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
int CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetMemberCount()
{
	return 0;
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
TGameMember* CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetMember(int nIndex)
{
	if(nIndex<0 || nIndex>=sizeof(m_pMemberList)/sizeof(m_pMemberList[0])) {
		return NULL;
	} else {
		return m_pMemberList[nIndex];
	}
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
TGameMember* CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetMember(unsigned int nCIdx)
{
	int nIndex = (int)(nCIdx & 0xffff);

	if(nIndex<0 || nIndex>=sizeof(m_pMemberList)/sizeof(m_pMemberList[0])) {
		return NULL;
	}
	if(m_pMemberList[nIndex]==NULL) {
		return NULL;
	}
	if(m_pMemberList[nIndex]->GetRIdx()!=nCIdx) {
		return NULL;
	}

	return m_pMemberList[nIndex];
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
TGameMember* CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetNextMember(TGameMember* pMember)
{
	int nIndex;

	if(pMember) {
		nIndex = (int)(pMember->GetRIdx() & 0xffff) + 1;
	} else {
		nIndex = 0;
	}

	for(; nIndex<sizeof(m_pMemberList)/sizeof(m_pMemberList[0]); nIndex++) {
		if(!m_pMemberList[nIndex]) continue;
		return m_pMemberList[nIndex];
	}

	return NULL;
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
TGameMember* CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetPrevMember(TGameMember* pMember)
{
	int nIndex;

	if(pMember) {
		nIndex = (int)(pMember->GetRIdx() & 0xffff) - 1;
	} else {
		nIndex = sizeof(m_pMemberList)/sizeof(m_pMemberList[0]) - 1;
	}

	for(; nIndex>=0; nIndex--) {
		if(!m_pMemberList[nIndex]) continue;
		return m_pMemberList[nIndex];
	}

	return NULL;
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
TGameMember* CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetNextMember(TGameMember* pMember, unsigned int nMask, unsigned int nValue)
{
	int nIndex;

	if(pMember) {
		nIndex = (int)(pMember->GetRIdx() & 0xffff) + 1;
	} else {
		nIndex = 0;
	}

	for(; nIndex<sizeof(m_pMemberList)/sizeof(m_pMemberList[0]); nIndex++) {
		if(!m_pMemberList[nIndex]) continue;
		if(m_pMemberList[nIndex]->GetMask()&nMask!=nValue) continue;
		return m_pMemberList[nIndex];
	}

	return NULL;
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
TGameMember* CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetPrevMember(TGameMember* pMember, unsigned int nMask, unsigned int nValue)
{
	int nIndex;

	if(pMember) {
		nIndex = (int)(pMember->GetRIdx() & 0xffff) - 1;
	} else {
		nIndex = sizeof(m_pMemberList)/sizeof(m_pMemberList[0]) - 1;
	}

	for(; nIndex>=0; nIndex--) {
		if(!m_pMemberList[nIndex]) continue;
		if(m_pMemberList[nIndex]->GetMask()&nMask!=nValue) continue;
		return m_pMemberList[nIndex];
	}

	return NULL;
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
void CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::Send(TGameMember* pMember, unsigned int nMask, unsigned int nValue, unsigned int nFlags, const void* pData, int nSize)
{
}

template<class TGameUser, class TGameRoom, class TGameMember>
CGameMember<TGameUser, TGameRoom, TGameMember>::CGameMember(TGameUser* pUser, TGameRoom* pRoom, unsigned int nCIdx)
{
	m_pUser = pUser;
	m_pRoom = pRoom;
	m_nCIdx = nCIdx;
	m_nMask = 0;

	pRoom->MemberAttach((TGameMember*)this);
}

template<class TGameUser, class TGameRoom, class TGameMember>
CGameMember<TGameUser, TGameRoom, TGameMember>::~CGameMember()
{
	m_pRoom->MemberDetach((TGameMember*)this);
	m_pRoom = NULL;
}

template<class TGameUser, class TGameRoom, class TGameMember>
TGameUser* CGameMember<TGameUser, TGameRoom, TGameMember>::GetGameUser()
{
	return m_pUser;
}

template<class TGameUser, class TGameRoom, class TGameMember>
unsigned int CGameMember<TGameUser, TGameRoom, TGameMember>::GetGameUIdx()
{
	return m_nUIdx;
}

template<class TGameUser, class TGameRoom, class TGameMember>
void CGameMember<TGameUser, TGameRoom, TGameMember>::SetGameUser(TGameUser* pUser, unsigned int nUIdx)
{
	m_pUser = pUser;
	m_nUIdx = nUIdx;
}

template<class TGameUser, class TGameRoom, class TGameMember>
TGameRoom* CGameMember<TGameUser, TGameRoom, TGameMember>::GetGameRoom()
{
	return m_pRoom;
}

template<class TGameUser, class TGameRoom, class TGameMember>
unsigned int CGameMember<TGameUser, TGameRoom, TGameMember>::GetRIdx()
{
	return m_nCIdx;
}

template<class TGameUser, class TGameRoom, class TGameMember>
void CGameMember<TGameUser, TGameRoom, TGameMember>::SetMask(unsigned int nMask)
{
	m_nMask = nMask;
}

template<class TGameUser, class TGameRoom, class TGameMember>
unsigned int CGameMember<TGameUser, TGameRoom, TGameMember>::GetMask() const
{
	return m_nMask;
}
