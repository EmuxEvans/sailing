
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

bool IPropertySet::GetBOOL(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return 0;
	return *((bool*)((char*)pData + GetProperty(nIndex)->nOffset));
}

int IPropertySet::GetINT(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return 0;
	return *((int*)((char*)pData + GetProperty(nIndex)->nOffset));
}

unsigned int IPropertySet::GetDWORD(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return 0;
	return *((unsigned int*)((char*)pData + GetProperty(nIndex)->nOffset));
}

float IPropertySet::GetFLOAT(const void* pData, int nIndex)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return 0.0f;
	return *((float*)((char*)pData + GetProperty(nIndex)->nOffset));
}

bool IPropertySet::SetValue(void* pData, int nIndex, bool bValue)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	*((bool*)((char*)pData + GetProperty(nIndex)->nOffset)) = bValue;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, int nValue)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	*((int*)((char*)pData + GetProperty(nIndex)->nOffset)) = nValue;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, unsigned int nValue)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	*((unsigned int*)((char*)pData + GetProperty(nIndex)->nOffset)) = nValue;
	return true;
}

bool IPropertySet::SetValue(void* pData, int nIndex, float fValue)
{
	assert(nIndex>=0 && nIndex<PropertyCount());
	if(nIndex<0 || nIndex>=PropertyCount()) return false;
	*((float*)((char*)pData + GetProperty(nIndex)->nOffset)) = fValue;
	return true;
}

char IPropertySet::GetBOOL(const void* pData, const char* pName)
{
	return GetBOOL(pData, GetPropertyIndex(pName));
}

int IPropertySet::GetINT(const void* pData, const char* pName)
{
	return GetINT(pData, GetPropertyIndex(pName));
}

unsigned int IPropertySet::GetDWORD(const void* pData, const char* pName)
{
	return GetDWORD(pData, GetPropertyIndex(pName));
}

float IPropertySet::GetFLOAT(const void* pData, const char* pName)
{
	return GetFLOAT(pData, GetPropertyIndex(pName));
}

bool IPropertySet::SetValue(void* pData, const char* pName, char bValue)
{
	return SetValue(pData, GetPropertyIndex(pName), bValue);
}

bool IPropertySet::SetValue(void* pData, const char* pName, int nValue)
{
	return SetValue(pData, GetPropertyIndex(pName), nValue);
}

bool IPropertySet::SetValue(void* pData, const char* pName, unsigned int nValue)
{
	return SetValue(pData, GetPropertyIndex(pName), nValue);
}

bool IPropertySet::SetValue(void* pData, const char* pName, float fValue)
{
	return SetValue(pData, GetPropertyIndex(pName), fValue);
}
