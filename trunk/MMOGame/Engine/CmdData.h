#pragma once

typedef struct CmdData {
	unsigned int nCmd;
	unsigned int nWho;
	unsigned int nSize;
	void* pData;
} CmdData;

class CDataReader {
public:
	CDataReader(const void* pBuf, unsigned int nSize);

	void Reset();

	template<class T>
	bool GetValue(T& Value, T Min=0, T Max=0);
	template<class T>
	T GetValue();
	bool GetString(const char*& pValue, unsigned int nMaxLen=0xffff);
	template<class T>
	bool GetStruct(const T*& Value);
	const void* GetStruct(unsigned int nSize);
	const char* GetString(unsigned int nMaxLen=0xffff);
	template<class T>
	bool GetArray(const T*& Array, unsigned int* Count, unsigned int Max=0xffff);
	template<class T>
	bool GetArray(const T*& Array, unsigned int Count);

	bool Error() {
		return m_bError;
	}
private:
	bool m_bError;
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
	bool PutStruct(const T* pValue);
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
	assert(m_nCurrent+sizeof(T)<=m_nSize);
	if(m_nCurrent+sizeof(T)>m_nSize) { m_bError = true; return false; }
	Value = *((const T*)(m_pBuf + m_nCurrent));
	if((Min!=0 || Max!=0) && (Value<Min || Value>Max)) { m_bError = true; return false; }
	m_nCurrent += sizeof(T);
	return true;
}

template<class T>
T CDataReader::GetValue() {
	T Value;
	assert(m_nCurrent+sizeof(T)<=m_nSize);
	if(m_nCurrent+sizeof(T)>m_nSize) { m_bError = true; return (T)-1; }
	Value = *((const T*)(m_pBuf + m_nCurrent));
	m_nCurrent += sizeof(T);
	return Value;
}

template<class T>
bool CDataReader::GetStruct(const T*& Value)
{
	assert(m_nCurrent+sizeof(T)<=m_nSize);
	if(m_nCurrent+sizeof(T)>m_nSize) { m_bError = true; return false; }
	Value = (const T*)(m_pBuf + m_nCurrent);
	m_nCurrent += sizeof(T);
	return true;
}

template<class T>
bool CDataReader::GetArray(const T*& Array, unsigned int* Count, unsigned int Max)
{
	assert(m_nCurrent+sizeof(unsigned short)<=m_nSize);
	if(m_nCurrent+sizeof(unsigned short)>m_nSize) { m_bError = true; return false; }
	*Count = *((unsigned short*)(m_pBuf+m_nCurrent));
	if(Max!=0x10000 && *Count>Max) { m_bError = true; return false; }
	assert(m_nCurrent+sizeof(unsigned short)+sizeof(T)*(*Count)<=m_nSize);
	if(m_nCurrent+sizeof(unsigned short)+sizeof(T)*(*Count)>m_nSize) { m_bError = true; return false; }
	Array = (T*)(m_pBuf+m_nCurrent+sizeof(unsigned short));
	m_nCurrent += sizeof(unsigned short) + sizeof(T)*(*Count);	
	return true;
}

template<class T>
bool CDataReader::GetArray(const T*& Array, unsigned int Count)
{
	assert(m_nCurrent+sizeof(unsigned short)<=m_nSize);
	if(m_nCurrent+sizeof(unsigned short)>m_nSize) { m_bError = true; return false; }
	assert((unsigned int)(*((unsigned short*)(m_pBuf+m_nCurrent)))==Count);
	if((unsigned int)(*((unsigned short*)(m_pBuf+m_nCurrent)))!=Count) { m_bError = true; return false; }
	assert(m_nCurrent+sizeof(unsigned short)+sizeof(T)*(ACount)<=m_nSize);
	if(m_nCurrent+sizeof(unsigned short)+sizeof(T)*(ACount)>m_nSize) { m_bError = true; return false; }
	Array = (T*)(m_pBuf+m_nCurrent+sizeof(unsigned short));
	m_nCurrent += sizeof(unsigned short) + sizeof(T)*Count;
	return true;
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
	m_CmdData.pData = GetBuffer();
	m_CmdData.nSize = GetLength();
	return &m_CmdData;
}

template<class T>
bool CDataWriter::PutValue(T Value)
{
	assert(m_nCurrent+sizeof(T)<=m_nSize);
	if(m_nCurrent+sizeof(T)>m_nSize) return false;
	*((T*)(m_pBuf + m_nCurrent)) = Value;
	m_nCurrent += sizeof(T);
	return true;
}

template<class T>
bool CDataWriter::PutStruct(const T* pValue)
{
	assert(m_nCurrent+sizeof(T)<=m_nSize);
	if(m_nCurrent+sizeof(T)>m_nSize) return false;
	memcpy(m_pBuf + m_nCurrent, pValue, sizeof(*pValue));
	m_nCurrent += sizeof(*pValue);
	return true;
}

template<class T>
bool CDataWriter::PutArray(T* Array, unsigned int Count)
{
	assert(Count<0xffff);
	if(Count>0xffff) return false;
	assert(m_nCurrent+sizeof(unsigned short)+sizeof(T)*Count<=m_nSize);
	if(m_nCurrent+sizeof(unsigned short)+sizeof(T)*Count>m_nSize) return false;
	memcpy(m_pBuf+m_nCurrent, &Count, sizeof(unsigned short));
	memcpy(m_pBuf+m_nCurrent+sizeof(unsigned short), Array, sizeof(T)*Count);
	m_nCurrent += sizeof(unsigned short) + sizeof(T)*Count;
	return true;
}
