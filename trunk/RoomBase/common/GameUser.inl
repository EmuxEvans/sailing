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
void CGameUser<TGameUser>::OnConnect()
{
	m_pController->OnConnect((TGameUser*)this);
}

template<class TGameUser>
void CGameUser<TGameUser>::OnDisconnect()
{
	for(int idx=0; idx<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); idx++) {
		if(!m_DynChannel[idx].pChannel) continue;
		m_DynChannel[idx].pChannel->Disconnect(m_DynChannel[idx].nCIdx);
	}
	m_pController->OnDisconnect((TGameUser*)this);
}

template<class TGameUser>
void CGameUser<TGameUser>::OnData(const void* pData, unsigned int nSize)
{
}

template<class TGameUser>
void CGameUser<TGameUser>::SendData(const void* pData, unsigned int nSize)
{
	m_pController->SendData((TGameUser*)this, pData, nSize);
}

template<class TGameUser>
void CGameUser<TGameUser>::Disconnect()
{
	m_pController->Disconnect();
}

template<class TGameUser>
bool CGameUser<TGameUser>::BindChannel(IGameChannel* pChannel, unsigned int nCIdx)
{
	for(int idx=0; idx<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); idx++) {
		if(m_DynChannel[idx].pChannel!=NULL) continue;
		m_DynChannel[idx].pChannel = pChannel;
		m_DynChannel[idx].nCIdx = nCIdx;
		return true;
	}
	return false;
}

template<class TGameUser>
bool CGameUser<TGameUser>::UnbindChannel(IGameChannel* pChannel, unsigned int nCIdx)
{
	for(int idx=0; idx<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); idx++) {
		if(m_DynChannel[idx].pChannel!=NULL || m_DynChannel[idx].nCIdx!=NULL) continue;
		m_DynChannel[idx].pChannel = NULL;
		m_DynChannel[idx].nCIdx = 0;
		return true;
	}
	return false;
}
