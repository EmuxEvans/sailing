#pragma once

class CPropertyDBTable {
public:
	CPropertyDBTable(IPropertySet* pSet, const char* pTableName, bool bArray) {
		m_pSet = pSet;
		m_pTableName = pTableName;
		m_bArray = bArray;
	}
	~CPropertyDBTable() { }

	IPropertySet* GetPropertySet() {
		return m_pSet;
	}
	const char* GetTableName() {
		return m_pTableName;
	}
	bool GetIsArray() {
		return m_bArray;
	}

private:
	IPropertySet*	m_pSet;
	const char*		m_pTableName;
	bool			m_bArray;
};

class IPropertyDBConnection {
public:
	virtual ~IPropertyDBConnection() { }
	virtual void Release() = 0;

	virtual bool CreateTable(CPropertyDBTable* pTable, char* pSqlText, unsigned int nLength) = 0;
	virtual bool DeleteTable(CPropertyDBTable* pTable, char* pSqlText, unsigned int nLength) = 0;

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

IPropertyDBConnection* CreatePropertyDBConnection(const char* connstr);
