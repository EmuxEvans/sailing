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
//	m_pController->OnData((TGameUser*)this, pData, nSize);
//}
//
//template<class TGameUser>
//void CGameUser<TGameUser>::SendData(IGameChannel<TGameUser>* pChannel, unsigned short& nUCIdx, const void* pData, unsigned int nSize)
//{
//	m_pController->SendData((TGameUser*)this, pChannel, nUCIdx, pData, nSize);
//}

template<class TGameUser>
void CGameUser<TGameUser>::Disconnect()
{
	m_pController->Disconnect((TGameUser*)this);
}

template<class TGameUser>
bool CGameUser<TGameUser>::BindChannel(IGameChannel<TGameUser>* pChannel, unsigned int nCIdx, unsigned short& nUCIdx)
{
	if(pChannel->IsDynamic()) {
		for(unsigned int idx=0; idx<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); idx++) {
			if(m_DynChannel[idx].pChannel!=NULL) continue;
			memset(&m_DynChannel[idx], 0, sizeof(m_DynChannel[idx]));
			strcpy(m_DynChannel[idx].szName, pChannel->GetName());
			m_DynChannel[idx].pChannel = pChannel;
			m_DynChannel[idx].nCIdx = nCIdx;
			nUCIdx = (unsigned short)(((GenChannelSeq()&0x7f)<<8) | 0x8000 | idx);
			m_DynChannel[idx].nUCIdx = nUCIdx;
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
		nUCIdx = (unsigned short)(((GenChannelSeq()&0x7f)<<8) | pChannel->GetType());
		m_StaChannel[pChannel->GetType()].nUCIdx = nUCIdx;
		return true;
	}
	return false;
}

template<class TGameUser>
bool CGameUser<TGameUser>::UnbindChannel(IGameChannel<TGameUser>* pChannel, unsigned int nCIdx, unsigned short nUCIdx)
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

template<class TGameUser>
IGameChannel<TGameUser>* CGameUser<TGameUser>::GetChannel(const char* pName, unsigned int& nCIdx)
{
	const char* at;
	at = strchr(pName, ':');
	if(at) {
		char name[100];
		for(int idx=0; idx<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); idx++) {
			if(!m_DynChannel[idx].pChannel) continue;
			sprintf(name, "%s:%04x", m_DynChannel[idx].pChannel->GetName(), m_DynChannel[idx].nCIdx);
			if(strcmp(name, pName)==0) {
				nCIdx = m_DynChannel[idx].nCIdx;
				return m_DynChannel[idx].pChannel;
			}
		}
	} else {
		for(int idx=0; idx<sizeof(m_StaChannel)/sizeof(m_StaChannel[0]); idx++) {
			if(!m_StaChannel[idx].pChannel) continue;
			if(strcmp(m_StaChannel[idx].pChannel->GetName(), pName)==0) {
				nCIdx = m_StaChannel[idx].nCIdx;
				return m_StaChannel[idx].pChannel;
			}
		}
	}
	return NULL;
}

template<class TGameUser>
IGameChannel<TGameUser>* CGameUser<TGameUser>::GetChannel(unsigned short nUCIdx, unsigned int& nCIdx)
{
	unsigned int idx;
	idx = (unsigned int)(nUCIdx&0xff);
	if(nUCIdx&0x8000) {
		if(idx>=sizeof(m_DynChannel)/sizeof(m_DynChannel[0]))
			return NULL;
		if(m_DynChannel[idx].nUCIdx!=nUCIdx)
			return NULL;
		nCIdx = m_DynChannel[idx].nCIdx;
		return m_DynChannel[idx].pChannel;
	} else {
		if(idx>=sizeof(m_StaChannel)/sizeof(m_StaChannel[0]))
			return NULL;
		if(m_StaChannel[idx].nUCIdx!=nUCIdx)
			return NULL;
		nCIdx = m_StaChannel[idx].nCIdx;
		return m_StaChannel[idx].pChannel;
	}
}
