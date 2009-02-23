#pragma once

template<int Count>
CPropertySet<Count>::CPropertySet()
{
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
void CPropertySet<Count>::SetProperty(int nIndex, const char* pName, int nType, unsigned int nOffset)
{
	assert(nIndex>=0 && nIndex<Count);
	if(nIndex<0 || nIndex>=Count) return;
	strcpy(m_Infos[nIndex].szName, pName);
	m_Infos[nIndex].nType = nType;
	m_Infos[nIndex].nOffset = nOffset;
}
