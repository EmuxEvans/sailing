
#include <string.h>
#include <assert.h>

#include "PropertySet.h"
#include "PropertySet.inl"

int IPropertySet::GetPropertyIndex(const char* pName)
{
	for(int i=0; i<PropertyCount(); i++) {
		if(strcmp(pName, GetProperty(i)->GetName())==0) return i;
	}
	return -1;
}

char IPropertySet::GetCHAR(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_CHAR);
	if(pInfo->GetType()!=PROPERTY_TYPE_CHAR) return -1;
	return *((char*)((char*)pData + pInfo->GetOffset()));
}

short IPropertySet::GetSHORT(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_SHORT);
	if(pInfo->GetType()!=PROPERTY_TYPE_SHORT) return -1;
	return *((short*)((char*)pData + pInfo->GetOffset()));
}

int IPropertySet::GetINT(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_INT);
	if(pInfo->GetType()!=PROPERTY_TYPE_INT) return -1;
	return *((int*)((char*)pData + pInfo->GetOffset()));
}

unsigned char IPropertySet::GetBYTE(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_BYTE);
	if(pInfo->GetType()!=PROPERTY_TYPE_BYTE) return -1;
	return *((unsigned char*)((char*)pData + pInfo->GetOffset()));
}

unsigned short IPropertySet::GetWORD(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_WORD);
	if(pInfo->GetType()!=PROPERTY_TYPE_WORD) return -1;
	return *((unsigned short*)((char*)pData + pInfo->GetOffset()));
}

unsigned int IPropertySet::GetDWORD(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_DWORD);
	if(pInfo->GetType()!=PROPERTY_TYPE_DWORD) return -1;
	return *((unsigned int*)((char*)pData + pInfo->GetOffset()));
}

float IPropertySet::GetFLOAT(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_FLOAT);
	if(pInfo->GetType()!=PROPERTY_TYPE_FLOAT) return -1;
	return *((float*)((char*)pData + pInfo->GetOffset()));
}

const char* IPropertySet::GetSTRING(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return NULL;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_STRING);
	if(pInfo->GetType()!=PROPERTY_TYPE_STRING) return NULL;
	return (const char*)pData + pInfo->GetOffset();
}

bool IPropertySet::SetValue(void* pData, int nIndex, char Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_CHAR);
	if(pInfo->GetType()!=PROPERTY_TYPE_CHAR) return false;
	*((char*)((char*)pData + pInfo->GetOffset())) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, short Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_SHORT);
	if(pInfo->GetType()!=PROPERTY_TYPE_SHORT) return false;
	*((short*)((char*)pData + pInfo->GetOffset())) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, int Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_INT);
	if(pInfo->GetType()!=PROPERTY_TYPE_INT) return false;
	*((int*)((char*)pData + pInfo->GetOffset())) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, unsigned char Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_BYTE);
	if(pInfo->GetType()!=PROPERTY_TYPE_BYTE) return false;
	*((unsigned char*)((char*)pData + pInfo->GetOffset())) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, unsigned short Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_WORD);
	if(pInfo->GetType()!=PROPERTY_TYPE_WORD) return false;
	*((unsigned short*)((char*)pData + pInfo->GetOffset())) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, unsigned int Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_DWORD);
	if(pInfo->GetType()!=PROPERTY_TYPE_DWORD) return false;
	*((unsigned int*)((char*)pData + pInfo->GetOffset())) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, float Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_FLOAT);
	if(pInfo->GetType()!=PROPERTY_TYPE_FLOAT) return false;
	*((float*)((char*)pData + pInfo->GetOffset())) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, const char* Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	IProperty* pInfo = GetProperty(nIndex);
	assert(pInfo->GetType()==PROPERTY_TYPE_FLOAT);
	if(pInfo->GetType()!=PROPERTY_TYPE_FLOAT) return false;
	unsigned int len = strlen(Value);
	assert(len<=pInfo->GetSize());
	if(len>pInfo->GetSize()) return false;
	memcpy((char*)pData+pInfo->GetOffset(), Value, len+1);
	return true;
}


CProperty::CProperty()
{
	m_Name = NULL;
	m_Type = 0;
	m_TypeDefine = NULL;
	m_Offset = 0;
	m_Count = 0;
	m_Desc = NULL;
}

CProperty::~CProperty()
{
}

const char* CProperty::GetName()
{
	return m_Name;
}

int CProperty::GetType()
{
	return m_Type;
}

IPropertySet* CProperty::GetTypeDefine()
{
	return m_TypeDefine;
}

unsigned int CProperty::GetOffset()
{
	return m_Offset;
}

unsigned int CProperty::GetSize()
{
	return m_Size;
}

unsigned int CProperty::GetCount()
{
	return m_Count;
}

const char* CProperty::GetDesc()
{
	return m_Desc;
}
