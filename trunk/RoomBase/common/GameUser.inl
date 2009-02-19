#pragma once

template<class TConnection>
CConnection<TConnection>::CConnection()
{
	memset(m_DynChannel, 0, sizeof(m_DynChannel));
	memset(m_StaChannel, 0, sizeof(m_StaChannel));
}

template<class TConnection>
CConnection<TConnection>::~CConnection()
{
}

template<class TConnection>
void CConnection<TConnection>::OnConnect()
{
}

template<class TConnection>
void CConnection<TConnection>::OnDisconnect()
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

template<class TConnection>
void CConnection<TConnection>::OnData(const void* pData, unsigned int nSize)
{
}

template<class TConnection>
bool CConnection<TConnection>::AttachStaticChannel(unsigned short nType, const char* pName, IChannel<TConnection>* pChannel)
{
	assert(nType<sizeof(m_StaChannel)/sizeof(m_StaChannel[0]));
	assert(m_StaChannel[nType].pChannel==NULL);
	assert(strlen(pName)<sizeof(m_StaChannel[0].szName));

	if(nType>=sizeof(m_StaChannel)/sizeof(m_StaChannel[0])) return false;
	if(m_StaChannel[nType].pChannel!=NULL) return false;

	strcpy(m_StaChannel[nType].szName, pName);
	m_StaChannel[nType].pChannel = pChannel;
	m_StaChannel[nType].nChnIdx = (unsigned short)(((GenChannelSeq()&0x7f)<<8) | pChannel->GetType());

	pChannel->OnAttach(m_StaChannel[nType].nChnIdx, (TConnection*)this);
	return true;
}

template<class TConnection>
bool CConnection<TConnection>::DetachStaticChannel(IChannel<TConnection>* pChannel, unsigned short nChnIdx)
{
	assert((nChnIdx&0xff)<sizeof(m_StaChannel)/sizeof(m_StaChannel[0]));
	if((nChnIdx&0xff)>sizeof(m_StaChannel)/sizeof(m_StaChannel[0])) return false;

	assert(m_StaChannel[nChnIdx&0xff].pChannel==pChannel);
	assert(m_StaChannel[nChnIdx&0xff].nChnIdx==nChnIdx);
	if(m_StaChannel[nChnIdx&0xff].pChannel!=pChannel) return false;
	if(m_StaChannel[nChnIdx&0xff].nChnIdx!=nChnIdx) return false;

	m_StaChannel[nChnIdx&0xff].pChannel->OnDetach(m_StaChannel[nChnIdx&0xff].pChannel, m_StaChannel[nChnIdx&0xff].nChnIdx);

	m_StaChannel[nChnIdx&0xff].szName[0] = '\0';
	m_StaChannel[nChnIdx&0xff].pChannel = NULL;
	m_StaChannel[nChnIdx&0xff].nChnIdx = 0;

	return true;
}

template<class TConnection>
bool CConnection<TConnection>::AttachDynamicChannel(unsigned short nType, const char* pName, IChannel<TConnection>* pChannel)
{
	assert(strlen(pName)<sizeof(m_DynChannel[0].szName));

	for(unsigned int idx=0; idx<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); idx++) {
		if(m_DynChannel[idx].pChannel!=NULL) continue;

		strcpy(m_DynChannel[idx].szName, pChannel->GetName());
		m_DynChannel[idx].nType = nType;
		m_DynChannel[idx].pChannel = pChannel;
		m_DynChannel[idx].nChnIdx = (unsigned short)(((GenChannelSeq()&0x7f)<<8) | 0x8000 | idx);
		pChannel->OnAttach(m_DynChannel[idx].nChnIdx, (TConnection*)this);

		return true;
	}

	return false;
}

template<class TConnection>
bool CConnection<TConnection>::DetachDynamicChannel(IChannel<TConnection>* pChannel, unsigned short nChnIdx)
{
	assert((nChnIdx&0xff)<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]));
	if((nChnIdx&0xff)>sizeof(m_DynChannel)/sizeof(m_DynChannel[0])) return false;

	assert(m_DynChannel[nChnIdx&0xff].pChannel==pChannel);
	assert(m_DynChannel[nChnIdx&0xff].nChnIdx==nChnIdx);
	if(m_DynChannel[nChnIdx&0xff].pChannel!=pChannel) return false;
	if(m_DynChannel[nChnIdx&0xff].nChnIdx!=nChnIdx) return false;

	m_DynChannel[nChnIdx&0xff].pChannel->OnDetach(m_DynChannel[nChnIdx&0xff].pChannel, m_DynChannel[nChnIdx&0xff].nChnIdx);

	m_DynChannel[nChnIdx&0xff].nType = 0;
	m_DynChannel[nChnIdx&0xff].szName[0] = '\0';
	m_DynChannel[nChnIdx&0xff].pChannel = NULL;
	m_DynChannel[nChnIdx&0xff].nChnIdx = 0;

	return true;
}

template<class TConnection>
IChannel<TConnection>* CConnection<TConnection>::GetChannel(const char* pName)
{
	int i;
	for(i=0; i<sizeof(m_StaChannel)/sizeof(m_StaChannel[0]); i++) {
		if(m_StaChannel[i].pChannel==NULL) continue;
		if(strcmp(m_StaChannel[i].szName, pName)!=0) continue;
		return m_StaChannel[i].pChannel;
	}
	for(i=0; i<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]); i++) {
		if(m_DynChannel[i].pChannel==NULL) continue;
		char szName[sizeof(m_DynChannel[i].szName)+10];
		sprintf(szName, "%s[%u]", m_DynChannel[i].szName, m_DynChannel[i].nChnIdx);
		if(strcmp(szName, pName)!=0) continue;
		return m_DynChannel[i].pChannel;
	}
	return NULL;
}

template<class TConnection>
IChannel<TConnection>* CConnection<TConnection>::GetChannel(unsigned short nChnIdx)
{
	if(nChnIdx&0x8000) {
		assert((nChnIdx&0xff)<sizeof(m_DynChannel)/sizeof(m_DynChannel[0]));
		if((nChnIdx&0xff)>sizeof(m_DynChannel)/sizeof(m_DynChannel[0])) return NULL;
		if(m_DynChannel[nChnIdx&0xff].pChannel!=pChannel) return NULL;
		if(m_DynChannel[nChnIdx&0xff].nChnIdx!=nChnIdx) return NULL;
		return m_DynChannel[nChnIdx&0xff].pChannel;
	} else {
		assert((nChnIdx&0xff)<sizeof(m_StaChannel)/sizeof(m_StaChannel[0]));
		if((nChnIdx&0xff)>sizeof(m_StaChannel)/sizeof(m_StaChannel[0])) return NULL;
		if(m_StaChannel[nChnIdx&0xff].pChannel!=pChannel) return NULL;
		if(m_StaChannel[nChnIdx&0xff].nChnIdx!=nChnIdx) return NULL;
		return m_StaChannel[nChnIdx&0xff].pChannel;
	}
}
