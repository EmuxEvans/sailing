#include <string.h>
#include <assert.h>

#include <skates/skates.h>
#include <sailing/proto_net.hpp>

#include "../common/GameUser.h"
#include "../common/GameRoom.h"
#include "../common/GameUser.inl"
#include "../common/GameRoom.inl"

#include "../Cube.proto.h"
#include "../Cube.proto.h"
#include "../Cube.net.proto.h"
#include "../Cube.net.svr.hpp"
#include "CubeServer.proto.h"

#include "Cube.h"
#include "CubeServer.h"

const char* _username = "username";
const char* _password = "password";

CServerLoginChannel::CServerLoginChannel() : m_Stub(NULL), m_Parser(NULL, &m_pHook, 1)
{
	m_pHook = this;
	SetChannel("Login", 0, false);
}

CServerLoginChannel::~CServerLoginChannel()
{
}

bool CServerLoginChannel::Join(CCubeUser* pUser)
{
	pUser->BindChannel(this, 0, m_nUCIdx);
	m_pUser = pUser;

	m_Stub.SeedCallback(0);

	return true;
}

void CServerLoginChannel::OnData(CCubeUser* pUser, unsigned int nCIdx, const void* pData, unsigned int nSize)
{

	if(!m_Parser.Dispatch(pData, nSize)) {
		assert(0);
		pUser->Disconnect();
	}
}

void CServerLoginChannel::Disconnect(CCubeUser* pUser, unsigned int nCIdx)
{
	pUser->UnbindChannel(this, 0, m_nUCIdx);
	m_pUser = NULL;
}

void CServerLoginChannel::Login(char* username, char* password)
{
	strcpy(m_szUserName, username);
	m_Stub.Login_Callback(0);
}

void CServerLoginChannel::Create(char* nickname)
{
	m_Stub.Create_Callback(0);
}
