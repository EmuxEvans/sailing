#pragma once

typedef struct CmdData {
	unsigned int nCmd;
	unsigned int nWho;
	const void* pData;
	unsigned int nSize;
} CmdData;

class CDataReader {
public:
	CDataReader(const char* pBuf, unsigned int nSize);

	void Reset();

	template<class T>
	bool GetValue(T& Value, T Min=0, T Max=0);
	template<class T>
	T GetValue();
	bool GetString(const char*& pValue, unsigned int nMaxLen=0);
	const char* GetString(unsigned int nMaxLen=0);
	template<class T>
	bool GetArray(T*& Array, unsigned int* Count=NULL, unsigned int Max=0xffff);
	template<class T>
	bool GetArray(T*& Array, unsigned int Count);
	template<class T>
	T* GetArray(T*& Array, unsigned int* Count=NULL);

private:
	const char* m_pBuf;
	unsigned int m_nCurrent, m_nSize;
};

class CDataWriter {
public:
	CDataWriter(char* pBuf, unsigned int nSize);

	void Reset();
	unsigned int GetLength();

	template<class T>
	bool PutValue(T Value);
	bool PutString(const char* pValue);
	template<class T>
	bool PutArray(T* Array, unsigned int Count);

private:
	char* m_pBuf;
	unsigned int m_nCurrent, m_nSize;
};

class CCmdDataReader : public CDataReader {
public:
	CCmdDataReader(const CmdData* pCmdData);

	unsigned int CmdCode();
	unsigned int CmdWho();

private:
	const CmdData* m_pCmdData;
};

template<unsigned int nSize>
class CDataBuffer : public CDataWriter {
public:
	CDataBuffer() : CDataWriter(m_szBuf, sizeof(m_szBuf)) {
	}

	void* GetBuffer() {
		return m_szBuf;
	}

private:
	char m_szBuf[nSize];
};

template<unsigned int nSize>
class CCmdDataWriter : public CDataBuffer<nSize> {
public:
	CCmdDataWriter(unsigned int nCmd, unsigned int nWho);

	void Reset(unsigned int nCmd, unsigned int nWho);
	const CmdData* GetCmdData();

private:
	CmdData m_CmdData;
};

template<class T>
bool CDataReader::GetValue(T& Value, T Min, T Max)
{
	if(m_nCurrent+sizeof(T)>m_nSize) return false;
	Value = *((const T*)((const char*)m_pCmdData->pData + m_nCurrent));
	if((Min!=0 || Max!=0) && (Value<Min || Value>Max)) return false;
	m_nCurrent += sizeof(T);
	return true;
}

template<class T>
T CDataReader::GetValue() {
	T Value;
	if(m_nCurrent+sizeof(T)>m_nSize) return 0;
	Value = *((const T*)(m_pBuf + m_nCurrent));
	m_nCurrent += sizeof(T);
	return Value;
}

template<class T>
bool CDataReader::GetArray(T*& Array, unsigned int* Count, unsigned int Max)
{
	return true;
}

template<class T>
bool CDataReader::GetArray(T*& Array, unsigned int Count)
{
	return true;
}

template<class T>
T* CDataReader::GetArray(T*& Array, unsigned int* Count)
{
	return NULL;
}

template<unsigned int nSize>
CCmdDataWriter<nSize>::CCmdDataWriter(unsigned int nCmd, unsigned int nWho) : CDataBuffer()
{
	Reset(nCmd, nWho);
}

template<unsigned int nSize>
void CCmdDataWriter<nSize>::Reset(unsigned int nCmd, unsigned int nWho)
{
	m_CmdData.nCmd = nCmd;
	m_CmdData.nWho = nWho;
	CDataBuffer::Reset();
}

template<unsigned int nSize>
const CmdData* CCmdDataWriter<nSize>::GetCmdData()
{
	m_CmdData.nSize = GetCurrent();
	return m_CmdData;
}

template<class T>
bool CDataWriter::PutValue(T Value)
{
	if(m_nCurrent+sizeof(T)>m_nSize) return false;
	*((T*)(m_pBuf + m_nCurrent)) = Value;
	m_nCurrent += sizeof(T);
	return true;
}

template<class T>
bool CDataWriter::PutArray(T* Array, unsigned int Count)
{
	return true;
}
