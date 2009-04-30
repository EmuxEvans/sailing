
#include <assert.h>
#include <string.h>
#include <vector>

#include "CmdData.h"
#include "GameLoop.h"
#include "PropertySet.h"
#include "DataObject.h"

class CDataOperator : public IDataOperator
{
public:
	CDataOperator();
	virtual ~CDataOperator();

	virtual bool Reset();
	virtual bool Post(const CmdData* pCmdData);
	virtual bool Cancel();
	virtual void Release();

	virtual bool Insert(CDataTable* pTable, unsigned int nUUId, const void* pData);
	virtual bool Insert(CDataTable* pTable, unsigned int nUUId, unsigned int nIndex, const void* pData);

	virtual bool Delete(CDataTable* pTable, unsigned int nUUID, const void* pData);
	virtual bool Delete(CDataTable* pTable, unsigned int nUUID, unsigned int nIndex, const void* pData);

	virtual bool Write(CDataTable* pTable, unsigned int nUUId, const void* pData);
	virtual bool Write(CDataTable* pTable, unsigned int nUUId, unsigned int nIndex, const void* pData);

	virtual bool Read(CDataTable* pTable, unsigned int nUUId, void* pData);
	virtual bool Read(CDataTable* pTable, unsigned int nUUId, unsigned int nIndex, void* pData);
};

CDataOperator::CDataOperator()
{
}

CDataOperator::~CDataOperator()
{
}

bool CDataOperator::Reset()
{
	return true;
}

bool CDataOperator::Post(const CmdData* pCmdData)
{
	return true;
}

bool CDataOperator::Cancel()
{
	return true;
}

void CDataOperator::Release()
{
}

bool CDataOperator::Insert(CDataTable* pTable, unsigned int nUUId, const void* pData)
{
	return true;
}

bool CDataOperator::Insert(CDataTable* pTable, unsigned int nUUId, unsigned int nIndex, const void* pData)
{
	return true;
}

bool CDataOperator::Delete(CDataTable* pTable, unsigned int nUUID, const void* pData)
{
	return true;
}

bool CDataOperator::Delete(CDataTable* pTable, unsigned int nUUID, unsigned int nIndex, const void* pData)
{
	return true;
}

bool CDataOperator::Write(CDataTable* pTable, unsigned int nUUId, const void* pData)
{
	return true;
}

bool CDataOperator::Write(CDataTable* pTable, unsigned int nUUId, unsigned int nIndex, const void* pData)
{
	return true;
}

bool CDataOperator::Read(CDataTable* pTable, unsigned int nUUId, void* pData)
{
	return true;
}

bool CDataOperator::Read(CDataTable* pTable, unsigned int nUUId, unsigned int nIndex, void* pData)
{
	return true;
}
