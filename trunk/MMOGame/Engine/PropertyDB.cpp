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

	virtual bool Insert(CPropertyDBTable* pTable, unsigned int nUID, const void* pData);
	virtual bool Delete(CPropertyDBTable* pTable, unsigned int nUID);
	virtual bool Read(CPropertyDBTable* pTable, unsigned int nUID, void* pData);
	virtual bool Write(CPropertyDBTable* pTable, unsigned int nUID, const void* pData);

	virtual bool GetArrayCount(CPropertyDBTable* pTable, unsigned int nUID, unsigned int& nCount);
	virtual bool Insert(CPropertyDBTable* pTable, unsigned int nUID, unsigned int nIndex, const void* pData);
	virtual bool Delete(CPropertyDBTable* pTable, unsigned int nUID, unsigned int nIndex);
	virtual bool Read(CPropertyDBTable* pTable, unsigned int nUID, unsigned int nIndex, void* pData);
	virtual bool Write(CPropertyDBTable* pTable, unsigned int nUID, unsigned int nIndex, const void* pData);
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

	switch(pTable->GetTableType()) {
	case CPropertyDBTable::SINGLE:
		nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, "uid INTEGER");
		if(nSize>=nLength) return false;
		break;
	case CPropertyDBTable::ARRAY:
		nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, "uid INTEGER");
		if(nSize>=nLength) return false;
		nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", idx INTEGER");
		if(nSize>=nLength) return false;
		break;
	case CPropertyDBTable::QUEUE:
		break;
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

	switch(pTable->GetTableType()) {
	case CPropertyDBTable::SINGLE:
		nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", PRIMARY KEY(uid)");
		if(nSize>=nLength) return false;
		break;
	case CPropertyDBTable::ARRAY:
		nSize += (unsigned int)snprintf(pSqlText+nSize, nLength-nSize, ", PRIMARY KEY(uid, idx)");
		if(nSize>=nLength) return false;
		break;
	case CPropertyDBTable::QUEUE:
		break;
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

bool CPropertyDBConnection_SQLite::Insert(CPropertyDBTable* pTable, unsigned int nUID, const void* pData)
{
	if(pTable->GetTableType()!=CPropertyDBTable::SINGLE) return false;

	unsigned int nSize = 0;
	char SqlText[1000];

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "insert into %s values(", pTable->GetTableName());
	if(nSize>=sizeof(SqlText)) return false;

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%u", nUID);
	if(nSize>=sizeof(SqlText)) return false;

	for(int i=0; i<pTable->GetPropertySet()->PropertyCount(); i++) {
		switch(pTable->GetPropertySet()->GetProperty(i)->GetType()) {
		case PROPERTY_TYPE_CHAR:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %d", 
				*((const char*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_SHORT:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %d", 
				*((const short*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_INT:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %d", 
				*((const int*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_BYTE:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %u", 
				*((const unsigned char*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_WORD:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %u", 
				*((const unsigned short*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_DWORD:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %u", 
				*((const unsigned int*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_FLOAT:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %f", 
				*((const float*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_STRING:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", '%s'", 
				(const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset());
			if(nSize>=sizeof(SqlText)) return false;
			break;
		default:
			return false;
		}
	}

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ")");
	if(nSize>=sizeof(SqlText)) return false;
	return true;
}

bool CPropertyDBConnection_SQLite::Delete(CPropertyDBTable* pTable, unsigned int nUID)
{
	if(pTable->GetTableType()!=CPropertyDBTable::SINGLE) return false;

	unsigned int nSize = 0;
	char SqlText[1000];

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "delete from %s where uid=%u", pTable->GetTableName(), nUID);
	if(nSize>=sizeof(SqlText)) return false;

	return true;
}

bool CPropertyDBConnection_SQLite::Read(CPropertyDBTable* pTable, unsigned int nUID, void* pData)
{
	if(pTable->GetTableType()!=CPropertyDBTable::SINGLE) return false;

	unsigned int nSize = 0;
	char SqlText[1000];

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "select ");
	if(nSize>=sizeof(SqlText)) return false;
	for(int i=0; i<pTable->GetPropertySet()->PropertyCount(); i++) {
		if(i>0) {
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %s", pTable->GetPropertySet()->GetProperty(i)->GetName());
			if(nSize>=sizeof(SqlText)) return false;
		} else {
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s", pTable->GetPropertySet()->GetProperty(i)->GetName());
			if(nSize>=sizeof(SqlText)) return false;
		}
	}
	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, " from %s where uid=%u", pTable->GetTableName(), nUID);
	if(nSize>=sizeof(SqlText)) return false;

	return true;
}

bool CPropertyDBConnection_SQLite::Write(CPropertyDBTable* pTable, unsigned int nUID, const void* pData)
{
	if(pTable->GetTableType()!=CPropertyDBTable::SINGLE) return false;

	unsigned int nSize = 0;
	char SqlText[1000];

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "update %s set ", pTable->GetTableName());
	if(nSize>=sizeof(SqlText)) return false;
	for(int i=0; i<pTable->GetPropertySet()->PropertyCount(); i++) {
		if(i>0) {
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", ");
			if(nSize>=sizeof(SqlText)) return false;
		}

		switch(pTable->GetPropertySet()->GetProperty(i)->GetType()) {
		case PROPERTY_TYPE_CHAR:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%d",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const char*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_SHORT:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%d",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const short*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_INT:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%d",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const int*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_BYTE:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%u",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const unsigned char*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_WORD:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%u",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const unsigned short*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_DWORD:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%u",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const unsigned int*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_FLOAT:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%f",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const float*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_STRING:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s='%s'",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				(const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset());
			if(nSize>=sizeof(SqlText)) return false;
			break;
		default:
			return false;
		}
	}

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, " where uid=%u", nUID);
	if(nSize>=sizeof(SqlText)) return false;

	return true;
}

bool CPropertyDBConnection_SQLite::GetArrayCount(CPropertyDBTable* pTable, unsigned int nUID, unsigned int& nCount)
{
	return true;
}

bool CPropertyDBConnection_SQLite::Insert(CPropertyDBTable* pTable, unsigned int nUID, unsigned int nIndex, const void* pData)
{
	if(pTable->GetTableType()!=CPropertyDBTable::ARRAY) return false;

	unsigned int nSize = 0;
	char SqlText[1000];

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "insert into %s values(", pTable->GetTableName());
	if(nSize>=sizeof(SqlText)) return false;

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%u, %u", nUID, nIndex);
	if(nSize>=sizeof(SqlText)) return false;

	for(int i=0; i<pTable->GetPropertySet()->PropertyCount(); i++) {
		switch(pTable->GetPropertySet()->GetProperty(i)->GetType()) {
		case PROPERTY_TYPE_CHAR:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %d", 
				*((const char*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_SHORT:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %d", 
				*((const short*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_INT:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %d", 
				*((const int*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_BYTE:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %u", 
				*((const unsigned char*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_WORD:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %u", 
				*((const unsigned short*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_DWORD:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %u", 
				*((const unsigned int*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_FLOAT:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %f", 
				*((const float*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_STRING:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", '%s'", 
				(const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset());
			if(nSize>=sizeof(SqlText)) return false;
			break;
		default:
			return false;
		}
	}

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ")");
	if(nSize>=sizeof(SqlText)) return false;
	return true;
}

bool CPropertyDBConnection_SQLite::Delete(CPropertyDBTable* pTable, unsigned int nUID, unsigned int nIndex)
{
	if(pTable->GetTableType()!=CPropertyDBTable::ARRAY) return false;

	unsigned int nSize = 0;
	char SqlText[1000];

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "delete from %s where uid=%u and idx=%u", pTable->GetTableName(), nUID, nIndex);
	if(nSize>=sizeof(SqlText)) return false;

	return true;
}

bool CPropertyDBConnection_SQLite::Read(CPropertyDBTable* pTable, unsigned int nUID, unsigned int nIndex, void* pData)
{
	if(pTable->GetTableType()!=CPropertyDBTable::ARRAY) return false;

	unsigned int nSize = 0;
	char SqlText[1000];

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "select ");
	if(nSize>=sizeof(SqlText)) return false;
	for(int i=0; i<pTable->GetPropertySet()->PropertyCount(); i++) {
		if(i>0) {
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", %s", pTable->GetPropertySet()->GetProperty(i)->GetName());
			if(nSize>=sizeof(SqlText)) return false;
		} else {
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s", pTable->GetPropertySet()->GetProperty(i)->GetName());
			if(nSize>=sizeof(SqlText)) return false;
		}
	}
	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, " from %s where uid=%u and idx=%u", pTable->GetTableName(), nUID, nIndex);
	if(nSize>=sizeof(SqlText)) return false;

	return true;
}

bool CPropertyDBConnection_SQLite::Write(CPropertyDBTable* pTable, unsigned int nUID, unsigned int nIndex, const void* pData)
{
	if(pTable->GetTableType()!=CPropertyDBTable::ARRAY) return false;

	unsigned int nSize = 0;
	char SqlText[1000];

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "update %s set ", pTable->GetTableName());
	if(nSize>=sizeof(SqlText)) return false;
	for(int i=0; i<pTable->GetPropertySet()->PropertyCount(); i++) {
		if(i>0) {
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, ", ");
			if(nSize>=sizeof(SqlText)) return false;
		}

		switch(pTable->GetPropertySet()->GetProperty(i)->GetType()) {
		case PROPERTY_TYPE_CHAR:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%d",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const char*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_SHORT:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%d",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const short*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_INT:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%d",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const int*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_BYTE:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%u",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const unsigned char*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_WORD:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%u",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const unsigned short*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_DWORD:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%u",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const unsigned int*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_FLOAT:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s=%f",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				*((const float*)((const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset())));
			if(nSize>=sizeof(SqlText)) return false;
			break;
		case PROPERTY_TYPE_STRING:
			nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, "%s='%s'",
				pTable->GetPropertySet()->GetProperty(i)->GetName(),
				(const char*)pData + pTable->GetPropertySet()->GetProperty(i)->GetOffset());
			if(nSize>=sizeof(SqlText)) return false;
			break;
		default:
			return false;
		}
	}

	nSize += (unsigned int)snprintf(SqlText+nSize, sizeof(SqlText)-nSize, " where uid=%u and idx=%u", nUID, nIndex);
	if(nSize>=sizeof(SqlText)) return false;

	return true;
}
