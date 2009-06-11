#pragma once

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
	CmdArg(const char* name, unsigned int type, const char* desc="");
	CmdArg(const char* name, unsigned int type, const char* struct_name, unsigned int struct_size, const char* desc="");

	const char* m_Name;
	unsigned int m_Type;
	const char* m_StructName;
	unsigned int m_StructSize;
	const char* m_Desc;
};

class CmdInfo
{
public:
	CmdInfo(const char* name, unsigned short code, const char* desc="");

	const char* m_Name;
	unsigned short m_Code;
	const char* m_Desc;
	std::vector<CmdArg> m_Args;
};

class CCmdSet
{
public:
	CCmdSet();
	~CCmdSet();

	void PushCmd(const char* name, unsigned short code, const char* desc="");
	void PushArg(const char* name, int type, const char* desc="");
	void PushArg(const char* name, int type, const char* struct_name, unsigned int struct_size, const char* desc="");

	int GetCmdCount();
	const CmdInfo* GetCmd(int nIndex);
	const CmdInfo* GetCmd(const char* name);
	const CmdInfo* GetCmd(unsigned short nCmd);

private:
	std::vector<CmdInfo> m_Cmds;
};
