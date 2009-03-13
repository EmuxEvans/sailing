#pragma once

enum {
	CMDARG_TYPE_DWORD	= 0x0001,
	CMDARG_TYPE_FLOAT	= 0x0002,
	CMDARG_TYPE_STRING	= 0x0003,
	CMDARG_TYPE_STRUCT	= 0x0004,
	CMDARG_TYPE_ARRAY	= 0x0100,
};

class CmdArg
{
public:
	CmdArg(const char* name, int type, const char* struct_name=NULL, unsigned int struct_size=0) {
		m_Name = name;
		m_Type = type;
		m_StructName = struct_name?struct_name:"";
		m_StructSize = struct_size;
	}

	std::string m_Name;
	int m_Type;
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

class CSGCmdSet
{
public:
	CSGCmdSet() {}
	~CSGCmdSet() {}

	void PushCmd(const char* name, unsigned short code);
	void PushArg(const char* name, int type);

	const CmdInfo* GetCmd(int nIndex);
	int GetCmdCount();
	const CmdInfo* GetCmd(const char* name);

private:
	std::vector<CmdInfo> m_Cmds;
};

class CSGClientCmdSet : public CSGCmdSet
{
public:
	CSGClientCmdSet();
};

class CSGServerCmdSet : public CSGCmdSet
{
public:
	CSGServerCmdSet();
};

class CSGCmdBuilder
{
public:
	CSGCmdBuilder(CSGCmdSet* pCmdSet);
	~CSGCmdBuilder();

	const char* GetTokenString(const char* pInput, char* pOutput, int nLength);
	const char* GetTokenNumber(const char* pInput, char* pOutput, int nLength);
	const char* GetTokenIdentity(const char* pInput, char* pOutput, int nLength);

	const void* ParseString(const char* pString, unsigned int& nLength);

private:
	CSGCmdSet* m_pCmdSet;
	char m_Bufs[1000];
};

class CSGCmdResult
{
public:
	CSGCmdResult(unsigned int value);
	CSGCmdResult(float value);
	CSGCmdResult(const char* value);

	unsigned int	GetDWORD()	const { assert(m_Type==CMDARG_TYPE_DWORD);	return m_dword; }
	float			GetFLOAT()	const { assert(m_Type==CMDARG_TYPE_FLOAT);	return m_float; }
	const char*		GetSTRING()	const { assert(m_Type==CMDARG_TYPE_STRING);	return m_string; }

private:
	int m_Type;
	union {
		unsigned int	m_dword;
		float			m_float;
		const char*		m_string;
	};
};

class CSGCmdResultSet
{
public:
	CSGCmdResultSet(const CmdInfo* pCmdInfo) {
		m_pCmdInfo = pCmdInfo;
	}

	void Reset() {
		m_Results.clear();
	}

	void PutResult(CSGCmdResult& Result) {
		m_Results.push_back(Result);
	}

	const CSGCmdResult& operator[](size_t nIndex) const {
		assert(nIndex>=0 && nIndex<m_Results.size());
		return m_Results[nIndex];
	}
	const CSGCmdResult& operator[](const char* pName) const {
		int nIndex;
		for(nIndex=0; nIndex<(int)m_Results.size(); nIndex++) {
			if(m_pCmdInfo->m_Args[nIndex].m_Name==pName) break;
		}
		assert(nIndex<(int)m_Results.size());
		return m_Results[nIndex];
	}

private:
	const CmdInfo* m_pCmdInfo;
	std::vector<CSGCmdResult> m_Results;
};

class CSGCmdParser
{
public:
	CSGCmdParser(CSGCmdSet* pCmdSet);
	~CSGCmdParser();

	bool ParseData(CSGCmdResultSet& ResultSet, const void* pData, unsigned int nSize);
	
private:
	CSGCmdSet* m_pCmdSet;
};
