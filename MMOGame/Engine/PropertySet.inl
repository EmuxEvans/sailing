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

	virtual const char* GetName();
	virtual const char* GetDesc();
	virtual unsigned int GetSize();

	virtual int PropertyCount();
	virtual IProperty* GetProperty(int nIndex);

protected:
	virtual void InitData() = 0;
	void SetInfo(const char* pName, const char* pDesc);
	void SetProperty(int nIndex, const char* pName, int nType, unsigned int nOffset, unsigned int nSize, unsigned int nCount, const char* pDesc);

private:
	const char*		m_pName;
	const char*		m_pDesc;
	unsigned int	m_nSize;
	CProperty		m_Infos[Count];
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
const char* CPropertySet<Count>::GetName()
{
	return m_pName;
}

template<int Count>
const char* CPropertySet<Count>::GetDesc()
{
	return m_pDesc;
}

template<int Count>
unsigned int CPropertySet<Count>::GetSize()
{
	return m_nSize;
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
	m_Infos[nIndex].m_Name = pName;
	m_Infos[nIndex].m_Type = nType;
	m_Infos[nIndex].m_Offset = nOffset;
	m_Infos[nIndex].m_Size = nSize;
	m_Infos[nIndex].m_Count = nCount;
	m_Infos[nIndex].m_Desc = pDesc;
}
