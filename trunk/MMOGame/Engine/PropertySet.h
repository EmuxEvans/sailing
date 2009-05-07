#pragma once

#define PROPERTY_TYPE_CHAR		1
#define PROPERTY_TYPE_SHORT		2
#define PROPERTY_TYPE_INT		3
#define PROPERTY_TYPE_BYTE		4
#define PROPERTY_TYPE_WORD		5
#define PROPERTY_TYPE_DWORD		6
#define PROPERTY_TYPE_FLOAT		7
#define PROPERTY_TYPE_STRING	8
#define PROPERTY_TYPE_STRUCT	9

class IProperty;
class IPropertySet;

class IProperty
{
public:
	virtual ~IProperty() { }

	virtual const char* GetName() = 0;
	virtual int GetType() = 0;
	virtual IPropertySet* GetTypeDefine() = 0;
	virtual unsigned int GetOffset() = 0;
	virtual unsigned int GetSize() = 0;
	virtual unsigned int GetCount() = 0;
	virtual const char* GetDesc() = 0;
};

class IPropertySet
{
public:
	virtual ~IPropertySet() { }

	virtual const char* GetName() = 0;
	virtual const char* GetDesc() = 0;
	virtual unsigned int GetSize() = 0;

	virtual int PropertyCount() = 0;
	virtual IProperty* GetProperty(int nIndex) = 0;

	int GetPropertyIndex(const char* pName);

	char			GetCHAR(const void* pData, int nIndex);
	short			GetSHORT(const void* pData, int nIndex);
	int				GetINT(const void* pData, int nIndex);
	unsigned char	GetBYTE(const void* pData, int nIndex);
	unsigned short	GetWORD(const void* pData, int nIndex);
	unsigned int	GetDWORD(const void* pData, int nIndex);
	float			GetFLOAT(const void* pData, int nIndex);
	const char*		GetSTRING(const void* pData, int nIndex);

	bool SetValue(void* pData, int nIndex, char				Value);
	bool SetValue(void* pData, int nIndex, short			Value);
	bool SetValue(void* pData, int nIndex, int				Value);
	bool SetValue(void* pData, int nIndex, unsigned char	Value);
	bool SetValue(void* pData, int nIndex, unsigned short	Value);
	bool SetValue(void* pData, int nIndex, unsigned int		Value);
	bool SetValue(void* pData, int nIndex, float			Value);
	bool SetValue(void* pData, int nIndex, const char*		Value);

	char			GetCHAR(const void* pData,		const char* pName) { return GetCHAR(pData, GetPropertyIndex(pName)); }
	short			GetSHORT(const void* pData,		const char* pName) { return GetSHORT(pData, GetPropertyIndex(pName)); }
	int				GetINT(const void* pData,		const char* pName) { return GetINT(pData, GetPropertyIndex(pName)); }
	unsigned char	GetBYTE(const void* pData,		const char* pName) { return GetBYTE(pData, GetPropertyIndex(pName)); }
	unsigned short	GetWORD(const void* pData,		const char* pName) { return GetWORD(pData, GetPropertyIndex(pName)); }
	unsigned int	GetDWORD(const void* pData,		const char* pName) { return GetDWORD(pData, GetPropertyIndex(pName)); }
	float			GetFLOAT(const void* pData,		const char* pName) { return GetFLOAT(pData, GetPropertyIndex(pName)); }
	const char*		GetSTRING(const void* pData,	const char* pName) { return GetSTRING(pData, GetPropertyIndex(pName)); }

	bool SetValue(void* pData, const char* pName, char				Value) { return SetValue(pData, GetPropertyIndex(pName), Value); }
	bool SetValue(void* pData, const char* pName, short				Value) { return SetValue(pData, GetPropertyIndex(pName), Value); }
	bool SetValue(void* pData, const char* pName, int				Value) { return SetValue(pData, GetPropertyIndex(pName), Value); }
	bool SetValue(void* pData, const char* pName, unsigned char		Value) { return SetValue(pData, GetPropertyIndex(pName), Value); }
	bool SetValue(void* pData, const char* pName, unsigned short	Value) { return SetValue(pData, GetPropertyIndex(pName), Value); }
	bool SetValue(void* pData, const char* pName, unsigned int		Value) { return SetValue(pData, GetPropertyIndex(pName), Value); }
	bool SetValue(void* pData, const char* pName, float				Value) { return SetValue(pData, GetPropertyIndex(pName), Value); }
	bool SetValue(void* pData, const char* pName, const char*		Value) { return SetValue(pData, GetPropertyIndex(pName), Value); }
};
