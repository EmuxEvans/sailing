#pragma once

template<class TGameUser>
CGameUser<TGameUser>::CGameUser(IGameUserController<TGameUser>* pController)
{
	memset(m_DynChannel, 0, sizeof(m_DynChannel));
	memset(m_StaChannel, 0, sizeof(m_StaChannel));
	m_pController = pController;
}

template<class TGameUser>
CGameUser<TGameUser>::~CGameUser()
{
}

template<class TGameUser>
void CGameUser<TGameUser>::OnConnect()
{
	m_pController->OnConnect((TGameUser*)this);
}

template<class TGameUser>
void CGameUser<TGameUser>::OnDisconnect()
{
	for(int idx=0; idx<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); idx++) {
		if(!m_DynChannel[idx].pChannel) continue;
		m_DynChannel[idx].pChannel->Disconnect((TGameUser*)this, m_DynChannel[idx].nCIdx);
	}
	for(int idx=0; idx<sizeof(m_StaChannel)/sizeof(m_StaChannel[0]); idx++) {
		if(!m_StaChannel[idx].pChannel) continue;
		m_StaChannel[idx].pChannel->Disconnect((TGameUser*)this, m_StaChannel[idx].nCIdx);
	}
	m_pController->OnDisconnect((TGameUser*)this);
}

//template<class TGameUser>
//void CGameUser<TGameUser>::OnData(const void* pData, unsigned int nSize)
//{
//	assert(0);
//}

template<class TGameUser>
void CGameUser<TGameUser>::SendData(IGameChannel<TGameUser>* pChannel, const void* pData, unsigned int nSize)
{
	m_pController->SendData((TGameUser*)this, pChannel, pData, nSize);
}

template<class TGameUser>
void CGameUser<TGameUser>::Disconnect()
{
	m_pController->Disconnect((TGameUser*)this);
}

template<class TGameUser>
bool CGameUser<TGameUser>::BindChannel(IGameChannel<TGameUser>* pChannel, unsigned int nCIdx)
{
	if(pChannel->IsDynamic()) {
		for(int idx=0; idx<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); idx++) {
			if(m_DynChannel[idx].pChannel!=NULL) continue;
			memset(&m_DynChannel[idx], 0, sizeof(m_DynChannel[idx]));
			strcpy(m_DynChannel[idx].szName, pChannel->GetName());
			m_DynChannel[idx].pChannel = pChannel;
			m_DynChannel[idx].nCIdx = nCIdx;
			return true;
		}
	} else {
		assert(pChannel->GetType()<sizeof(m_StaChannel)/sizeof(m_StaChannel[0]));
		assert(m_StaChannel[pChannel->GetType()].pChannel==NULL);
		if(pChannel->GetType()>=sizeof(m_StaChannel)/sizeof(m_StaChannel[0])) return false;
		if(strlen(pChannel->GetName())>=sizeof(m_StaChannel[0])) return false;
		if(m_StaChannel[pChannel->GetType()].pChannel!=NULL) return false;
		memset(&m_StaChannel[pChannel->GetType()], 0, sizeof(m_StaChannel[0]));
		strcpy(m_StaChannel[pChannel->GetType()].szName, pChannel->GetName());
		m_StaChannel[pChannel->GetType()].pChannel = pChannel;
		m_StaChannel[pChannel->GetType()].nCIdx = nCIdx;
	}
	return false;
}

template<class TGameUser>
bool CGameUser<TGameUser>::UnbindChannel(IGameChannel<TGameUser>* pChannel, unsigned int nCIdx)
{
	if(pChannel->IsDynamic()) {
		for(int idx=0; idx<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); idx++) {
			if(m_DynChannel[idx].pChannel!=NULL || m_DynChannel[idx].nCIdx!=NULL) continue;
			m_DynChannel[idx].pChannel = NULL;
			return true;
		}
	} else {
		assert(pChannel->GetType()<sizeof(m_StaChannel)/sizeof(m_StaChannel[0]));
		assert(m_StaChannel[pChannel->GetType()].pChannel==pChannel);
		if(pChannel->GetType()>=sizeof(m_StaChannel)/sizeof(m_StaChannel[0])) return false;
		if(m_StaChannel[pChannel->GetType()].pChannel!=pChannel) return false;
		m_StaChannel[pChannel->GetType()].pChannel = NULL;
	}
	return false;
}
