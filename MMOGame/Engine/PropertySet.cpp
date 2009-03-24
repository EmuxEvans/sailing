
#include <string.h>
#include <assert.h>

#include "PropertySet.h"
#include "PropertySet.inl"

int IPropertySet::GetPropertyIndex(const char* pName)
{
	for(int i=0; i<PropertyCount(); i++) {
		if(strcmp(pName, GetProperty(i)->szName)==0) return i;
	}
	return -1;
}

char IPropertySet::GetCHAR(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_CHAR);
	if(pInfo->nType!=PROPERTY_TYPE_CHAR) return -1;
	return *((char*)((char*)pData + pInfo->nOffset));
}

short IPropertySet::GetSHORT(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_SHORT);
	if(pInfo->nType!=PROPERTY_TYPE_SHORT) return -1;
	return *((short*)((char*)pData + pInfo->nOffset));
}

int IPropertySet::GetINT(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_INT);
	if(pInfo->nType!=PROPERTY_TYPE_INT) return -1;
	return *((int*)((char*)pData + pInfo->nOffset));
}

unsigned char IPropertySet::GetBYTE(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_BYTE);
	if(pInfo->nType!=PROPERTY_TYPE_BYTE) return -1;
	return *((unsigned char*)((char*)pData + pInfo->nOffset));
}

unsigned short IPropertySet::GetWORD(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_WORD);
	if(pInfo->nType!=PROPERTY_TYPE_WORD) return -1;
	return *((unsigned short*)((char*)pData + pInfo->nOffset));
}

unsigned int IPropertySet::GetDWORD(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_DWORD);
	if(pInfo->nType!=PROPERTY_TYPE_DWORD) return -1;
	return *((unsigned int*)((char*)pData + pInfo->nOffset));
}

float IPropertySet::GetFLOAT(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return -1;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_FLOAT);
	if(pInfo->nType!=PROPERTY_TYPE_FLOAT) return -1;
	return *((float*)((char*)pData + pInfo->nOffset));
}

const char* IPropertySet::GetSTRING(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return NULL;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_STRING);
	if(pInfo->nType!=PROPERTY_TYPE_STRING) return NULL;
	return (const char*)pData + pInfo->nOffset;
}

bool IPropertySet::SetValue(void* pData, int nIndex, char Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_CHAR);
	if(pInfo->nType!=PROPERTY_TYPE_CHAR) return false;
	*((char*)((char*)pData + pInfo->nOffset)) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, short Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_SHORT);
	if(pInfo->nType!=PROPERTY_TYPE_SHORT) return false;
	*((short*)((char*)pData + pInfo->nOffset)) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, int Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_INT);
	if(pInfo->nType!=PROPERTY_TYPE_INT) return false;
	*((int*)((char*)pData + pInfo->nOffset)) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, unsigned char Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_BYTE);
	if(pInfo->nType!=PROPERTY_TYPE_BYTE) return false;
	*((unsigned char*)((char*)pData + pInfo->nOffset)) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, unsigned short Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_WORD);
	if(pInfo->nType!=PROPERTY_TYPE_WORD) return false;
	*((unsigned short*)((char*)pData + pInfo->nOffset)) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, unsigned int Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_DWORD);
	if(pInfo->nType!=PROPERTY_TYPE_DWORD) return false;
	*((unsigned int*)((char*)pData + pInfo->nOffset)) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, float Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_FLOAT);
	if(pInfo->nType!=PROPERTY_TYPE_FLOAT) return false;
	*((float*)((char*)pData + pInfo->nOffset)) = Value;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, const char* Value)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	const PropertyInfo* pInfo = GetProperty(nIndex);
	assert(pInfo->nType==PROPERTY_TYPE_FLOAT);
	if(pInfo->nType!=PROPERTY_TYPE_FLOAT) return false;
	unsigned int len = strlen(Value);
	assert(len<=pInfo->nSize);
	if(len>pInfo->nSize) return false;
	memcpy((char*)pData+pInfo->nOffset, Value, len+1);
	return true;
}
