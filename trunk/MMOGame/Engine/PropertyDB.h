#pragma once

class CPropertyDBTable {
public:
	enum TableType {
		SINGLE,
		ARRAY,
		QUEUE,
	};

	CPropertyDBTable(IPropertySet* pSet, const char* pTableName, TableType nTableType) {
		m_pSet = pSet;
		m_pTableName = pTableName;
		m_nTableType = nTableType;
	}
	~CPropertyDBTable() { }

	IPropertySet* GetPropertySet() {
		return m_pSet;
	}
	const char* GetTableName() {
		return m_pTableName;
	}
	TableType GetTableType() {
		return m_nTableType;
	}

private:
	IPropertySet*	m_pSet;
	const char*		m_pTableName;
	TableType		m_nTableType;
};

class IPropertyDBConnection {
public:
	virtual ~IPropertyDBConnection() { }
	virtual void Release() = 0;

	virtual bool CreateTable(CPropertyDBTable* pTable, char* pSqlText, unsigned int nLength) = 0;
	virtual bool DeleteTable(CPropertyDBTable* pTable, char* pSqlText, unsigned int nLength) = 0;

	virtual bool Insert(CPropertyDBTable* pTable, unsigned int nUID, const void* pData) = 0;
	virtual bool Delete(CPropertyDBTable* pTable, unsigned int nUID) = 0;
	virtual bool Read(CPropertyDBTable* pTable, unsigned int nUID, void* pData) = 0;
	virtual bool Write(CPropertyDBTable* pTable, unsigned int nUID, const void* pData) = 0;

	virtual bool GetArrayCount(CPropertyDBTable* pTable, unsigned int nUID, unsigned int& nCount) = 0;
	virtual bool Insert(CPropertyDBTable* pTable, unsigned int nUID, unsigned int nIndex, const void* pData) = 0;
	virtual bool Delete(CPropertyDBTable* pTable, unsigned int nUID, unsigned int nIndex) = 0;
	virtual bool Read(CPropertyDBTable* pTable, unsigned int nUID, unsigned int nIndex, void* pData) = 0;
	virtual bool Write(CPropertyDBTable* pTable, unsigned int nUID, unsigned int nIndex, const void* pData) = 0;
};

IPropertyDBConnection* CreatePropertyDBConnection(const char* connstr);
