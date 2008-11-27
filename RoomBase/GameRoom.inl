#pragma once

template<class TGameUser>
CGameUser<TGameUser>::CGameUser(IGameUserController<TGameUser>* pController)
{
	memset(m_DynChannel, 0, sizeof(m_DynChannel));
	m_pController = pController;
}

template<class TGameUser>
CGameUser<TGameUser>::~CGameUser()
{
}

template<class TGameUser>
void CGameUser<TGameUser>::Connect()
{
	m_pController->OnConnect((TGameUser*)this);
}

template<class TGameUser>
void CGameUser<TGameUser>::Disconnect()
{
	for(int idx=0; idx<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); idx++) {
		if(!m_DynChannel[idx].pRoom) continue;
		m_DynChannel[idx].pRoom->Disconnect(m_DynChannel[idx].nRIdx);
	}
	m_pController->OnDisconnect((TGameUser*)this);
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
bool CGameUser<TGameUser>::BindRoom(unsigned int& nIndex, IGameRoom* pRoom, int nRIdx)
{
	for(int idx=0; idx<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); idx++) {
		if(m_DynChannel[idx].pRoom!=NULL) continue;
		m_DynChannel[idx].pRoom = pRoom;
		m_DynChannel[idx].nRIdx = nRIdx;
		return true;
	}
	return false;
}

template<class TGameUser>
bool CGameUser<TGameUser>::UnbindRoom(unsigned int nIndex, IGameRoom* pRoom, int nRIdx)
{
	for(int idx=0; idx<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); idx++) {
		if(m_DynChannel[idx].pRoom!=NULL || m_DynChannel[idx].nRIdx!=NULL) continue;
		m_DynChannel[idx].pRoom = NULL;
		m_DynChannel[idx].nRIdx = 0;
		return true;
	}
	return false;
}

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
	unsigned int nIndex, nRIdx, nUIdx;
	TGameMember* pMember;

	for(nIndex=0; nIndex<sizeof(m_pMemberList)/sizeof(m_pMemberList[0]); nIndex++) {
		if(!m_pMemberList[nIndex]) break;
	}
	if(nIndex==sizeof(m_pMemberList)/sizeof(m_pMemberList[0])) return false;

	if(!m_pController->MemberPrepareJoin((TGameRoom*)this, (TGameUser*)pUser)) {
		return false;
	}

	nRIdx = (GenGameSeq()<<16|nIndex);
	nUIdx = TGameRoom::GetRoomType();
	if(!pUser->BindRoom(nUIdx, this, nRIdx)) {
		return false;
	}

	pMember = TGameMember::Create((TGameUser*)pUser, nUIdx, (TGameRoom*)this, GenGameSeq()<<16|nIndex);

	return true;
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
void CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::OnData(unsigned int nRIdx, const void* pData, int nSize)
{
	TGameMember* pMember = GetMember(nRIdx);
	if(!pMember) return;

	m_pController->MemberOndata((TGameRoom*)this, pMember, pData, nSize);
}

template<class TGameUser, class TGameRoom, class TGameMember, int nMemberMax>
void CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::Disconnect(unsigned int nRIdx)
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
TGameMember* CGameRoom<TGameUser, TGameRoom, TGameMember, nMemberMax>::GetMember(unsigned int nRIdx)
{
	int nIndex = (int)(nRIdx & 0xffff);

	if(nIndex<0 || nIndex>=sizeof(m_pMemberList)/sizeof(m_pMemberList[0])) {
		return NULL;
	}
	if(m_pMemberList[nIndex]==NULL) {
		return NULL;
	}
	if(m_pMemberList[nIndex]->GetRIdx()!=nRIdx) {
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
CGameMember<TGameUser, TGameRoom, TGameMember>::CGameMember(TGameUser* pUser, unsigned int nUIdx, TGameRoom* pRoom, unsigned int nRIdx)
{
	m_pUser = pUser;
	m_nUIdx = nUIdx;
	m_pRoom = pRoom;
	m_nRIdx = nRIdx;
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
	return m_nRIdx;
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
