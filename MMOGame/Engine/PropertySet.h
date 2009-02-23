#pragma once

#define PROPERTY_TYPE_INT		1
#define PROPERTY_TYPE_DWORD		2
#define PROPERTY_TYPE_BOOL		3
#define PROPERTY_TYPE_FLOAT		4

typedef struct PropertyInfo {
	char			szName[20];
	int				nType;
	unsigned int	nOffset;
} PropertyInfo;

class IPropertySet
{
public:
	virtual ~IPropertySet() { }

	virtual int PropertyCount() = 0;
	virtual const PropertyInfo* GetProperty(int nIndex) = 0;

	int GetPropertyIndex(const char* pName);

	bool			GetBOOL(const void* pData, int nIndex);
	int				GetINT(const void* pData, int nIndex);
	unsigned int	GetDWORD(const void* pData, int nIndex);
	float			GetFLOAT(const void* pData, int nIndex);

	bool SetValue(void* pData, int nIndex, bool bValue);
	bool SetValue(void* pData, int nIndex, int nValue);
	bool SetValue(void* pData, int nIndex, unsigned int nValue);
	bool SetValue(void* pData, int nIndex, float fValue);

	char			GetBOOL(const void* pData, const char* pName);
	int				GetINT(const void* pData, const char* pName);
	unsigned int	GetDWORD(const void* pData, const char* pName);
	float			GetFLOAT(const void* pData, const char* pName);

	bool SetValue(void* pData, const char* pName, char bValue);
	bool SetValue(void* pData, const char* pName, int nValue);
	bool SetValue(void* pData, const char* pName, unsigned int nValue);
	bool SetValue(void* pData, const char* pName, float fValue);
};

template<int Count>
class CPropertySet : public IPropertySet
{
public:
	CPropertySet();
	virtual ~CPropertySet();

	virtual int PropertyCount();
	virtual const PropertyInfo* GetProperty(int nIndex);

protected:
	virtual void InitData() = 0;
	void SetProperty(int nIndex, const char* pName, int nType, unsigned int nOffset);

private:
	PropertyInfo m_Infos[Count];
};
