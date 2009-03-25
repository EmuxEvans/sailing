#pragma once

class CProperty;

template<int Count>
class CPropertySet;

class CProperty : public IProperty
{
	template<int Count>
	friend class CPropertySet;
public:
	CProperty();
	virtual ~CProperty();

	virtual const char* GetName();
	virtual int GetType();
	virtual IPropertySet* GetTypeDefine();
	virtual unsigned int GetOffset();
	virtual unsigned int GetSize();
	virtual unsigned int GetCount();
	virtual const char* GetDesc();

protected:
	const char*		m_Name;
	int				m_Type;
	IPropertySet*	m_TypeDefine;
	unsigned int	m_Offset;
	unsigned int	m_Size;
	unsigned int	m_Count;
	const char*		m_Desc;
};

template<int Count>
class CPropertySet : public IPropertySet
{
public:
	CPropertySet();
	virtual ~CPropertySet();

	virtual int PropertyCount();
	virtual IProperty* GetProperty(int nIndex);

protected:
	virtual void InitData() = 0;
	void SetInfo(const char* pName, const char* pDesc);
	void SetProperty(int nIndex, const char* pName, int nType, unsigned int nOffset, unsigned int nSize, unsigned int nCount, const char* pDesc);

private:
	const char* m_pName;
	const char* m_pDesc;
	CProperty m_Infos[Count];
};

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
IProperty* CPropertySet<Count>::GetProperty(int nIndex)
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
