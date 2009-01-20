#include <string.h>
#include <assert.h>

#include <skates/skates.h>

#include "../common/GameUser.h"
#include "../common/GameRoom.h"
#include "../common/GameUser.inl"
#include "../common/GameRoom.inl"

#include "../CubeRoom.proto.h"
#include "CubeRoom.h"
#include "CubeServer.h"

void* CCubeUser::operator new (size_t n)
{
	return mempool_alloc(user_pool);
}

void CCubeUser::operator delete(void* p)
{
	mempool_free(user_pool, p);
}

void* CCubeRoom::operator new (size_t n)
{
	return mempool_alloc(room_pool);
}

void CCubeRoom::operator delete(void* p)
{
	mempool_free(room_pool, p);
}

void* CCubeMember::operator new (size_t n)
{
	return mempool_alloc(memb_pool);
}

void CCubeMember::operator delete(void* p)
{
	mempool_free(memb_pool, p);
}

void CCubeUserController::OnConnect(CCubeUser* pUser)
{
}

void CCubeUserController::OnDisconnect(CCubeUser* pUser)
{
}

void CCubeUserController::OnData(CCubeUser* pUser, const void* pData, unsigned int nSize)
{
}

void CCubeUserController::SendData(CCubeUser* pUser, const void* pData, unsigned int nSize)
{
}

void CCubeUserController::Disconnect()
{
}

void CCubeRoomController::OnCreate(CCubeRoom* pRoom)
{
}

void CCubeRoomController::OnDestroy(CCubeRoom* pRoom)
{
}

bool CCubeRoomController::MemberPrepareJoin(CCubeRoom* pRoom, CCubeUser* pUser)
{
	return false;
}

void CCubeRoomController::MemberJoin(CCubeRoom* pRoom, CCubeMember* pMember)
{
}

void CCubeRoomController::MemberLeave(CCubeRoom* pRoom, CCubeMember* pMember)
{
}

void CCubeRoomController::MemberOndata(CCubeRoom* pRoom, CCubeMember* pMember, const void* pData, unsigned int nSize)
{
}
