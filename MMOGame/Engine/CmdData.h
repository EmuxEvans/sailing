#pragma once

enum {
	CMDARG_TYPE_CHAR	= 0x0001,
	CMDARG_TYPE_SHORT	= 0x0002,
	CMDARG_TYPE_INT		= 0x0003,

	CMDARG_TYPE_BYTE	= 0x0011,
	CMDARG_TYPE_WORD	= 0x0012,
	CMDARG_TYPE_DWORD	= 0x0013,

	CMDARG_TYPE_FLOAT	= 0x0101,
	CMDARG_TYPE_STRING	= 0x0102,
	CMDARG_TYPE_STRUCT	= 0x0103,
	CMDARG_TYPE_ARRAY	= 0x1000,
};

class CmdArg
{
public:
	CmdArg(const char* name, unsigned int type, const char* struct_name=NULL, unsigned int struct_size=0) {
		m_Name = name;
		m_Type = type;
		m_StructName = struct_name?struct_name:"";
		m_StructSize = struct_size;
	}

	std::string m_Name;
	unsigned int m_Type;
	std::string m_StructName;
	unsigned int m_StructSize;
};

class CmdInfo
{
public:
	CmdInfo(const char* name, unsigned short code) {
		m_Name = name;
		m_Code = code;
	}

	std::string m_Name;
	unsigned short m_Code;
	std::vector<CmdArg> m_Args;
};

class CCmdSet
{
public:
	CCmdSet() {}
	~CCmdSet() {}

	void PushCmd(const char* name, unsigned short code) {
		m_Cmds.push_back(CmdInfo(name, code));
	}
	void PushArg(const char* name, int type, const char* struct_name=NULL, unsigned int struct_size=0) {
		m_Cmds[m_Cmds.size()-1].m_Args.push_back(CmdArg(name, type, struct_name, struct_size));
	}

	int GetCmdCount() {
		return m_Cmds.size();
	}
	const CmdInfo* GetCmd(int nIndex) {
		if(nIndex<0 && nIndex>=(int)m_Cmds.size()) return NULL;
		return &m_Cmds[nIndex];
	}
	const CmdInfo* GetCmd(const char* name) {
		for(size_t l=0; l<m_Cmds.size(); l++) {
			if(m_Cmds[l].m_Name==name) {
				return &m_Cmds[l];
			}
		}
		return NULL;
	}
	const CmdInfo* GetCmd(unsigned short nCmd) {
		for(size_t l=0; l<m_Cmds.size(); l++) {
			if(m_Cmds[l].m_Code==nCmd) {
				return &m_Cmds[l];
			}
		}
		return NULL;
	}

private:
	std::vector<CmdInfo> m_Cmds;
};

typedef struct CmdData {
	unsigned int nCmd;
	unsigned int nWho;
	void* pData;
	unsigned int nSize;
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
	const char* GetString(unsigned int nMaxLen=0xffff);
	template<class T>
	bool GetArray(const T*& Array, unsigned int* Count, unsigned int Max=0xffff);
	template<class T>
	bool GetArray(const T*& Array, unsigned int Count);

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
	if(m_nCurrent+sizeof(T)>m_nSize) return false;
	Value = *((const T*)(m_pBuf + m_nCurrent));
	if((Min!=0 || Max!=0) && (Value<Min || Value>Max)) return false;
	m_nCurrent += sizeof(T);
	return true;
}

template<class T>
T CDataReader::GetValue() {
	T Value;
	assert(m_nCurrent+sizeof(T)<=m_nSize);
	if(m_nCurrent+sizeof(T)>m_nSize) return 0;
	Value = *((const T*)(m_pBuf + m_nCurrent));
	m_nCurrent += sizeof(T);
	return Value;
}

template<class T>
bool CDataReader::GetStruct(const T*& Value)
{
	assert(m_nCurrent+sizeof(T)<=m_nSize);
	if(m_nCurrent+sizeof(T)>m_nSize) return 0;
	Value = (const T*)(m_pBuf + m_nCurrent);
	m_nCurrent += sizeof(T);
	return Value;
}

template<class T>
bool CDataReader::GetArray(const T*& Array, unsigned int* Count, unsigned int Max)
{
	assert(m_nCurrent+sizeof(unsigned short)<=m_nSize);
	if(m_nCurrent+sizeof(unsigned short)>m_nSize) return false;
	*Count = *((unsigned short*)(m_pBuf+m_nCurrent));
	if(Max!=0x10000 && *Count>Max) return false;
	assert(m_nCurrent+sizeof(unsigned short)+sizeof(T)*(*Count)<=m_nSize);
	if(m_nCurrent+sizeof(unsigned short)+sizeof(T)*(*Count)>m_nSize) return false;
	Array = (T*)(m_pBuf+m_nCurrent+sizeof(unsigned short));
	m_nCurrent += sizeof(unsigned short) + sizeof(T)*(*Count);	
	return true;
}

template<class T>
bool CDataReader::GetArray(const T*& Array, unsigned int Count)
{
	assert(m_nCurrent+sizeof(unsigned short)<=m_nSize);
	if(m_nCurrent+sizeof(unsigned short)>m_nSize) return false;
	assert((unsigned int)(*((unsigned short*)(m_pBuf+m_nCurrent)))==Count);
	if((unsigned int)(*((unsigned short*)(m_pBuf+m_nCurrent)))!=Count) return false;
	assert(m_nCurrent+sizeof(unsigned short)+sizeof(T)*(ACount)<=m_nSize);
	if(m_nCurrent+sizeof(unsigned short)+sizeof(T)*(ACount)>m_nSize) return false;
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
	memcpy(m_pBuf + m_nCurrent, pValue);
	m_nCurrent += sizeof(T);
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
