#include <string.h>

#include "CmdData.h"

CDataReader::CDataReader(const void* pBuf, unsigned int nSize)
{
	m_pBuf = (const char*)pBuf;
	m_nSize = nSize;
	Reset();
}

void CDataReader::Reset()
{
	m_nCurrent = 0;	
}

bool CDataReader::GetString(const char*& pValue, unsigned int nMaxLen)
{
	unsigned int n;
	for(n=0; n<=nMaxLen && m_nCurrent+n<m_nSize; n++) {
		if(*(m_pBuf + m_nCurrent)=='\0') {
			pValue = m_pBuf + m_nCurrent;
			m_nCurrent += n;
			return true;
		}
	}
	return false;
}

const char* CDataReader::GetString(unsigned int nMaxLen)
{
	const char* pValue;
	return GetString(pValue, nMaxLen)?pValue:NULL;
}


CDataWriter::CDataWriter(char* pBuf, unsigned int nSize)
{
	m_pBuf = pBuf;
	m_nSize = nSize;
	Reset();
}

void CDataWriter::Reset()
{
	m_nCurrent = 0;
}

unsigned int CDataWriter::GetLength()
{
	return m_nCurrent;
}

bool CDataWriter::PutString(const char* pValue)
{
	unsigned int n = (unsigned int)strlen(pValue);
	if(m_nCurrent+n+1<m_nSize) return false;
	memcpy(m_pBuf+m_nCurrent, pValue, n+1);
	m_nCurrent += n + 1;
	return true;
}

CCmdDataReader::CCmdDataReader(const CmdData* pCmdData) : CDataReader((const char*)pCmdData->pData, pCmdData->nSize), m_pCmdData(pCmdData)
{
}

unsigned int CCmdDataReader::CmdCode()
{
	return m_pCmdData->nCmd;
}

unsigned int CCmdDataReader::CmdWho()
{
	return m_pCmdData->nWho;
}
