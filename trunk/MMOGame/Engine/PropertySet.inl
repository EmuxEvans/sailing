#pragma once

template<int Count>
CPropertySet<Count>::CPropertySet()
{
	m_pName = NULL;
	m_pDesc = NULL;
	memset(m_Infos, 0, sizeof(m_Infos));
	InitData();
}

template<int Count>
CPropertySet<Count>::~CPropertySet()
{
}

template<int Count>
int CPropertySet<Count>::PropertyCount()
{
	return Count;
}

template<int Count>
const PropertyInfo* CPropertySet<Count>::GetProperty(int nIndex)
{
	assert(nIndex>=0 && nIndex<Count);
	if(nIndex<0 || nIndex>=Count) return NULL;
	return &m_Infos[nIndex];
}

template<int Count>
void CPropertySet<Count>::SetInfo(const char* pName, const char* pDesc)
{
	m_pName = pName;
	m_pDesc = pDesc;
}

template<int Count>
void CPropertySet<Count>::SetProperty(int nIndex, const char* pName, int nType, unsigned int nOffset, unsigned int nSize, unsigned int nCount, const char* pDesc)
{
	assert(nIndex>=0 && nIndex<Count);
	if(nIndex<0 || nIndex>=Count) return;
	strcpy(m_Infos[nIndex].szName, pName);
	m_Infos[nIndex].nType = nType;
	m_Infos[nIndex].nOffset = nOffset;
	m_Infos[nIndex].nSize = nSize;
	m_Infos[nIndex].nCount = nCount;
	m_Infos[nIndex].pDesc = pDesc;
}
