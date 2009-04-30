#pragma once

class CDataTable
{
public:
	CDataTable(const char* pTableName, bool bArray, IPropertySet* pSet) {
		m_pTableName = pTableName;
		m_bArray = bArray;
		m_pSet = pSet;
	}

	const char* TableName() {
		return m_pTableName;
	}
	bool IsArray() {
		return m_bArray;
	}
	IPropertySet* GetPropertySet() {
		return m_pSet;
	}

private:
	const char* m_pTableName;
	bool m_bArray;
	IPropertySet* m_pSet;
};

class IDataOperator
{
public:
	virtual ~IDataOperator() { }

	virtual bool Reset() = 0;
	virtual bool Post(const CmdData* pCmdData) = 0;
	virtual bool Cancel() = 0;
	virtual void Release() = 0;

	virtual bool Insert(CDataTable* pTable, unsigned int nUUId, const void* pData) = 0;
	virtual bool Insert(CDataTable* pTable, unsigned int nUUId, unsigned int nIndex, const void* pData) = 0;

	virtual bool Delete(CDataTable* pTable, unsigned int nUUID, const void* pData);
	virtual bool Delete(CDataTable* pTable, unsigned int nUUID, unsigned int nIndex, const void* pData);

	virtual bool Write(CDataTable* pTable, unsigned int nUUId, const void* pData) = 0;
	virtual bool Write(CDataTable* pTable, unsigned int nUUId, unsigned int nIndex, const void* pData) = 0;

	virtual bool Read(CDataTable* pTable, unsigned int nUUId, void* pData) = 0;
	virtual bool Read(CDataTable* pTable, unsigned int nUUId, unsigned int nIndex, void* pData) = 0;
};
