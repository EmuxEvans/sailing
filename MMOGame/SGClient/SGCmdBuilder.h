#pragma once

enum {
	CMDARG_TYPE_DWORD	= 0x0001,
	CMDARG_TYPE_STRING	= 0x0002,
	CMDARG_TYPE_ARRAY	= 0x0100,
};

class CmdArg
{
public:
	CmdArg(const char* name, int type) {
		m_Name = name;
		m_Type = type;
	}

	std::string m_Name;
	int m_Type;
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

class CSGCmdBuilder
{
public:
	CSGCmdBuilder();
	~CSGCmdBuilder();

	void PushCmd(const char* name, unsigned short code);
	void PushArg(const char* name, int type);

	const CmdInfo* GetCmd(const char* name);
	const CmdInfo* GetCmd(int nIndex);
	int GetCmdCount();

	const char* GetTokenString(const char* pInput, char* pOutput, int nLength);
	const char* GetTokenNumber(const char* pInput, char* pOutput, int nLength);
	const char* GetTokenIdentity(const char* pInput, char* pOutput, int nLength);

	const void* ParseString(const char* pString, unsigned int& nLength);

private:
	std::vector<CmdInfo> m_Cmds;
	char m_Bufs[1000];

};
