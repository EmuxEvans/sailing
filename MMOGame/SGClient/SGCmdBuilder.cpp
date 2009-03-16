#include "StdAfx.h"
#include "SGCmdBuilder.h"

#include "..\Engine\CmdData.h"
#include "..\SGGame\SGCmdCode.h"

void CSGCmdSet::PushCmd(const char* name, unsigned short code)
{
	m_Cmds.push_back(CmdInfo(name, code));
}

void CSGCmdSet::PushArg(const char* name, int type)
{
	m_Cmds[m_Cmds.size()-1].m_Args.push_back(CmdArg(name, type));
}

int CSGCmdSet::GetCmdCount()
{
	return m_Cmds.size();
}

const CmdInfo* CSGCmdSet::GetCmd(int nIndex)
{
	if(nIndex<0 && nIndex>=(int)m_Cmds.size()) return NULL;
	return &m_Cmds[nIndex];
}

const CmdInfo* CSGCmdSet::GetCmd(const char* name)
{
	for(size_t l=0; l<m_Cmds.size(); l++) {
		if(m_Cmds[l].m_Name==name) {
			return &m_Cmds[l];
		}
	}
	return NULL;
}

const CmdInfo* CSGCmdSet::GetCmd(unsigned short nCmd)
{
	for(size_t l=0; l<m_Cmds.size(); l++) {
		if(m_Cmds[l].m_Code==nCmd) {
			return &m_Cmds[l];
		}
	}
	return NULL;
}

CSGClientCmdSet::CSGClientCmdSet()
{
	PushCmd("login", SGCMDCODE_LOGIN);
	PushArg("username", CMDARG_TYPE_STRING);
	PushArg("password", CMDARG_TYPE_STRING);

	PushCmd("move", SGCMDCODE_MOVE);
	PushArg("sx", CMDARG_TYPE_FLOAT);
	PushArg("sy", CMDARG_TYPE_FLOAT);
	PushArg("sz", CMDARG_TYPE_FLOAT);
	PushArg("ex", CMDARG_TYPE_FLOAT);
	PushArg("ey", CMDARG_TYPE_FLOAT);
	PushArg("ez", CMDARG_TYPE_FLOAT);
	PushArg("time", CMDARG_TYPE_DWORD);
}

CSGServerCmdSet::CSGServerCmdSet()
{
	PushCmd("login_seed", SGCMDCODE_LOGIN_SEED);
	PushArg("salt", CMDARG_TYPE_BYTE|CMDARG_TYPE_ARRAY);
}

CSGCmdBuilder::CSGCmdBuilder(CSGCmdSet* pCmdSet)
{
	m_pCmdSet = pCmdSet;
}

CSGCmdBuilder::~CSGCmdBuilder()
{
}

const char* escape_blank(const char* buf)
{
	for(;;buf++) {

		if(*buf=='/' && buf[1]=='/') {
			buf+=2;
			while(*buf!='\n' && *buf!='\0') buf++;
		}
		if(*buf=='\0') return buf;
		if(*buf<=' ') continue;

		return buf;
	}
}

const char* CSGCmdBuilder::GetTokenString(const char* pInput, char* pOutput, int nLength)
{
	int end;

	pInput = escape_blank(pInput);
	if(*pInput!='"') return NULL;
	for(end=1; ;end++) {
		if(pInput[end]=='\0') return NULL;
		if(pInput[end]=='"') break;
		if(pInput[end]=='\\') {
			if(pInput[end-1]=='\0') return NULL;
			end++; continue;
		}
	}
	if(end+2>nLength) return NULL;

	memcpy(pOutput, pInput+1, end+1-2);
	pOutput[end+1-2] = '\0';

	return pInput+end+1;
}

const char* CSGCmdBuilder::GetTokenNumber(const char* pInput, char* pOutput, int nLength)
{
	int end;

	pInput = escape_blank(pInput);

	for(end=0; ;end++) {
		if(pInput[end]>='0' && pInput[end]<='9') continue;
		if(pInput[end]=='+') continue;
		if(pInput[end]=='-') continue;
		if(pInput[end]=='.') continue;
		if(end==0) return NULL;
		break;
	}

	if(end+1>nLength) return NULL;
	memcpy(pOutput, pInput, end);
	pOutput[end] = '\0';

	return pInput+end;
}

const char* CSGCmdBuilder::GetTokenIdentity(const char* pInput, char* pOutput, int nLength)
{
	int end;

	pInput = escape_blank(pInput);

	for(end=0;; end++) {
		if(pInput[end]>='0' && pInput[end]<='9') {
			if(end==0) return NULL;
			continue;
		}
		if(pInput[end]>='a' && pInput[end]<='z') continue;
		if(pInput[end]>='A' && pInput[end]<='Z') continue;
		if(pInput[end]=='_') continue;

		break;
	}
	if(end==0) return NULL;

	if(end+1>nLength) return NULL;
	memcpy(pOutput, pInput, end);
	pOutput[end] = '\0';
	return pInput+end;
}

const void* CSGCmdBuilder::ParseString(const char* pString, unsigned int& nLength)
{
	char szCmdName[100];
	pString = GetTokenIdentity(pString, szCmdName, sizeof(szCmdName));
	if(!pString) return NULL;

	const CmdInfo* pCmdInfo = m_pCmdSet->GetCmd(szCmdName);
	if(!pCmdInfo) return NULL;

	CDataWriter data(m_Bufs, sizeof(m_Bufs));
	data.PutValue(pCmdInfo->m_Code);

	for(size_t l=0; l<pCmdInfo->m_Args.size(); l++) {
		char szValue[1000];

		switch(pCmdInfo->m_Args[l].m_Type) {
		case CMDARG_TYPE_DWORD:
			{
				pString = GetTokenNumber(pString, szValue, sizeof(szValue));
				if(!pString) return NULL;
				data.PutValue<unsigned int>((unsigned int)atoi(szValue));
				break;
			}
		case CMDARG_TYPE_FLOAT:
			{
				pString = GetTokenNumber(pString, szValue, sizeof(szValue));
				if(!pString) return NULL;
				data.PutValue<float>((float)atof(szValue));
				break;
			}
		case CMDARG_TYPE_STRING:
			{
				pString = GetTokenString(pString, szValue, sizeof(szValue));
				if(!pString) return NULL;
				data.PutString(szValue);
				break;
			}
		default:
			return NULL;
		}
	}

	nLength = data.GetLength();
	return m_Bufs;
}

CSGCmdParser::CSGCmdParser(CSGCmdSet* pCmdSet)
{
	m_pCmdSet = pCmdSet;
}

CSGCmdParser::~CSGCmdParser()
{
}

bool CSGCmdParser::ParseData(CSGCmdResultSet& ResultSet, const void* pData, unsigned int nSize)
{
	return true;
}
