#include <string.h>
#include <assert.h>
#include <string>
#include <vector>

#include "CmdInfo.h"

CmdArg::CmdArg(const char* name, unsigned int type, const char* desc)
{
	m_Name = name;
	m_Type = type;
	m_StructName = NULL;
	m_StructSize = 0;
	m_Desc = desc;
}

CmdArg::CmdArg(const char* name, unsigned int type, const char* struct_name, unsigned int struct_size, const char* desc)
{
	m_Name = name;
	m_Type = type;
	m_StructName = struct_name;
	m_StructSize = struct_size;
	m_Desc = desc;
}

CmdInfo::CmdInfo(const char* name, unsigned short code, const char* desc)
{
	m_Name = name;
	m_Code = code;
	m_Desc = desc;
}

CCmdSet::CCmdSet()
{
}

CCmdSet::~CCmdSet()
{
}

void CCmdSet::PushCmd(const char* name, unsigned short code, const char* desc)
{
	m_Cmds.push_back(CmdInfo(name, code));
}

void CCmdSet::PushArg(const char* name, int type, const char* desc)
{
	m_Cmds[m_Cmds.size()-1].m_Args.push_back(CmdArg(name, type, desc));
}

void CCmdSet::PushArg(const char* name, int type, const char* struct_name, unsigned int struct_size, const char* desc)
{
	m_Cmds[m_Cmds.size()-1].m_Args.push_back(CmdArg(name, type, struct_name, struct_size, desc));
}

int CCmdSet::GetCmdCount()
{
	return m_Cmds.size();
}

const CmdInfo* CCmdSet::GetCmd(int nIndex)
{
	if(nIndex<0 && nIndex>=(int)m_Cmds.size()) return NULL;
	return &m_Cmds[nIndex];
}

const CmdInfo* CCmdSet::GetCmd(const char* name)
{
	for(size_t l=0; l<m_Cmds.size(); l++) {
		if(strcmp(m_Cmds[l].m_Name, name)==0) {
			return &m_Cmds[l];
		}
	}
	return NULL;
}

const CmdInfo* CCmdSet::GetCmd(unsigned short nCmd)
{
	for(size_t l=0; l<m_Cmds.size(); l++) {
		if(m_Cmds[l].m_Code==nCmd) {
			return &m_Cmds[l];
		}
	}
	return NULL;
}
