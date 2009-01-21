#include <string.h>
#include <assert.h>

#include <skates/skates.h>

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

void* CCubeUser::operator new (size_t n)
{
	return mempool_alloc(user_pool);
}

void CCubeUser::operator delete(void* p)
{
	mempool_free(user_pool, p);
}

CCubeUser::CCubeUser(ICubeUserController* pController) : CGameUser<CCubeUser>(pController)
{
	m_pHandle = NULL;
	GetController()->OnInit(this);
}

CCubeUser::~CCubeUser()
{
	GetController()->OnFinal(this);
}

void CCubeUser::OnData(const void* pData, unsigned int nSize)
{
	if(TCP_TEXTMODE) {
	} else {
	}
}

void CCubeUserController::OnInit(CCubeUser* pUser)
{
}

void CCubeUserController::OnFinal(CCubeUser* pUser)
{
}

void CCubeUserController::OnConnect(CCubeUser* pUser)
{
	m_LoginChannel.Join(pUser);
}

void CCubeUserController::OnDisconnect(CCubeUser* pUser)
{
}

void CCubeUserController::OnData(CCubeUser* pUser, const void* pData, unsigned int nSize)
{
}

void CCubeUserController::SendData(CCubeUser* pUser, IGameChannel<CCubeUser>* pChannel, const void* pData, unsigned int nSize)
{
}

void CCubeUserController::Disconnect(CCubeUser* pUser)
{
}
