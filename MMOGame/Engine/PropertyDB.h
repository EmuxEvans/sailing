#pragma once

class IPropertyDBConnection {
public:
	virtual ~IPropertyDBConnection() { }

	virtual bool GenSql_CreateTable(CPropertyDBTable* pTable) = 0;
	virtual bool GenSql_DeleteTable(CPropertyDBTable* pTable) = 0;

	virtual bool Insert(CPropertyDBTable* pTable, unsigned int nPlayerId, const void* pData) = 0;
	virtual bool Delete(CPropertyDBTable* pTable, unsigned int nPlayerId) = 0;
	virtual bool Read(CPropertyDBTable* pTable, unsigned int nPlayerId, void* pData) = 0;
	virtual bool Write(CPropertyDBTable* pTable, unsigned int nPlayerId, const void* pData) = 0;

	virtual bool GetArrayCount(CPropertyDBTable* pTable, unsigned int nPlayerId, unsigned int& nCount) = 0;
	virtual bool Insert(CPropertyDBTable* pTable, unsigned int nPlayerId, unsigned int nIndex, const void* pData) = 0;
	virtual bool Delete(CPropertyDBTable* pTable, unsigned int nPlayerId, unsigned int nIndex) = 0;
	virtual bool Read(CPropertyDBTable* pTable, unsigned int nPlayerId, unsigned int nIndex, void* pData) = 0;
	virtual bool Write(CPropertyDBTable* pTable, unsigned int nPlayerId, unsigned int nIndex, const void* pData) = 0;
};

class CPropertyDBTable {
public:
	CPropertyDBTable(IPropertySet* pSet, const char* pName, bool bArray) {
		m_pSet = pSet;
		m_pName = pName;
		m_bArray = bArray;
	}
	~CPropertyDBTable() { }
protected:
	IPropertySet*	m_pSet;
	const char*		m_pName;
	bool			m_bArray;
};

class CPropertyDB
{
public:
	CPropertyDB(IPropertyDBConnection* pConnection);
	~CPropertyDB();

	
};
