#include <stdio.h>

#include "PropertySet.h"
#include "PropertyDB.h"

#define snprintf _snprintf

class CPropertyDBConnection_SQLite : public IPropertyDBConnection {
public:
	CPropertyDBConnection_SQLite() { }
	virtual ~CPropertyDBConnection_SQLite() { }
	virtual void Release() { delete this; }

	virtual bool CreateTable(CPropertyDBTable* pTable, char* pSqlText, unsigned int nLength);
	virtual bool DeleteTable(CPropertyDBTable* pTable, char* pSqlText, unsigned int nLength);

	virtual bool Insert(CPropertyDBTable* pTable, unsigned int nPlayerId, const void* pData) { return true; }
	virtual bool Delete(CPropertyDBTable* pTable, unsigned int nPlayerId) { return true; }
	virtual bool Read(CPropertyDBTable* pTable, unsigned int nPlayerId, void* pData) { return true; }
	virtual bool Write(CPropertyDBTable* pTable, unsigned int nPlayerId, const void* pData) { return true; }

	virtual bool GetArrayCount(CPropertyDBTable* pTable, unsigned int nPlayerId, unsigned int& nCount) { return true; }
	virtual bool Insert(CPropertyDBTable* pTable, unsigned int nPlayerId, unsigned int nIndex, const void* pData) { return true; }
	virtual bool Delete(CPropertyDBTable* pTable, unsigned int nPlayerId, unsigned int nIndex) { return true; }
	virtual bool Read(CPropertyDBTable* pTable, unsigned int nPlayerId, unsigned int nIndex, void* pData) { return true; }
	virtual bool Write(CPropertyDBTable* pTable, unsigned int nPlayerId, unsigned int nIndex, const void* pData) { return true; }
};

IPropertyDBConnection* CreatePropertyDBConnection(const char* connstr)
{
	return new CPropertyDBConnection_SQLite();
}

bool CPropertyDBConnection_SQLite::CreateTable(CPropertyDBTable* pTable, char* pSqlText, unsigned int nLength)
{
	unsigned int nSize = 0;
	nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, "create table %s (", pTable->GetTableName());
	if(nSize>=nLength) return false;

	if(pTable->GetIsArray()) {
		nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, "uid INTEGER");
		if(nSize>=nLength) return false;
		nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", idx INTEGER");
		if(nSize>=nLength) return false;
	} else {
		nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, "uid INTEGER");
		if(nSize>=nLength) return false;
	}

	for(int i=0; i<pTable->GetPropertySet()->PropertyCount(); i++) {
		IProperty* pProperty = pTable->GetPropertySet()->GetProperty(i);
		switch(pTable->GetPropertySet()->GetProperty(i)->GetType()) {
		case PROPERTY_TYPE_CHAR:
			nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", %s INTEGER", pTable->GetPropertySet()->GetProperty(i)->GetName());
			if(nSize>=nLength) return false;
		case PROPERTY_TYPE_SHORT:
			break;
			nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", %s INTEGER", pTable->GetPropertySet()->GetProperty(i)->GetName());
			if(nSize>=nLength) return false;
			break;
		case PROPERTY_TYPE_INT:
			nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", %s INTEGER", pTable->GetPropertySet()->GetProperty(i)->GetName());
			if(nSize>=nLength) return false;
			break;
		case PROPERTY_TYPE_BYTE:
			nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", %s INTEGER", pTable->GetPropertySet()->GetProperty(i)->GetName());
			if(nSize>=nLength) return false;
			break;
		case PROPERTY_TYPE_WORD:
			nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", %s INTEGER", pTable->GetPropertySet()->GetProperty(i)->GetName());
			if(nSize>=nLength) return false;
			break;
		case PROPERTY_TYPE_DWORD:
			nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", %s INTEGER", pTable->GetPropertySet()->GetProperty(i)->GetName());
			if(nSize>=nLength) return false;
			break;
		case PROPERTY_TYPE_FLOAT:
			nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", %s REAL", pTable->GetPropertySet()->GetProperty(i)->GetName());
			if(nSize>=nLength) return false;
			break;
		case PROPERTY_TYPE_STRING:
			nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", %s TEXT", pTable->GetPropertySet()->GetProperty(i)->GetName());
			if(nSize>=nLength) return false;
			break;
		default:
			return false;
		}
	}

	if(pTable->GetIsArray()) {
		nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", PRIMARY KEY(uid, idx)");
		if(nSize>=nLength) return false;
	} else {
		nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", PRIMARY KEY(uid)");
		if(nSize>=nLength) return false;
	}

	nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ")");
	if(nSize>=nLength) return false;
	return true;
}

bool CPropertyDBConnection_SQLite::DeleteTable(CPropertyDBTable* pTable, char* pSqlText, unsigned int nLength)
{
	unsigned int nSize = 0;
	nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, "drop table %s", pTable->GetTableName());
	if(nSize>=nLength) return false;
	return true;
}
