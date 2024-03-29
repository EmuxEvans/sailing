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

void* CCubeUser::operator new (size_t n)
{
	return mempool_alloc(user_pool);
}

void CCubeUser::operator delete(void* p)
{
	mempool_free(user_pool, p);
}

CCubeUser::CCubeUser(ICubeUserController* pController, bool bTextMode) : CGameUser<CCubeUser>(pController)
{
	m_pHandle = NULL;
	GetController()->OnInit(this);
	m_bTextMode = bTextMode;
}

CCubeUser::~CCubeUser()
{
	GetController()->OnFinal(this);
}

void CCubeUser::OnData(const void* pData, unsigned int nSize)
{
	IGameChannel<CCubeUser>* pChannel;
	unsigned int nCIdx;
	const void* pUserData;
	unsigned int nUserDataSize;

	if(m_bTextMode) {
		const char* at;
		char name[100];
		at = strchr((const char*)pData, '.');
		if(!at) {
			assert(0);
			return;
		}
		memcpy(name, pData, at-(const char*)pData);
		name[at-(const char*)pData] = '\0';
		strtrim(strltrim(name));
		pChannel = GetChannel(name, nCIdx);
		if(!pChannel) {
			assert(0);
			return;
		}
		pUserData = at + 1;
		nUserDataSize = nSize - (at - (const char*)pData) - 1;
	} else {
		if(nSize<2) {
			assert(0);
			return;
		}
		pChannel = GetChannel(*((unsigned short*)pData), nCIdx);
		if(!pChannel) {
			assert(0);
			return;
		}
		pUserData = (const char*)pData + sizeof(unsigned short);
		nUserDataSize = nSize - sizeof(unsigned short);
	}

	pChannel->OnData(this, nCIdx, pUserData, nUserDataSize);
}

void CCubeUser::SendData(IGameChannel<CCubeUser>* pChannel, unsigned short nUCIdx, const void* pData, unsigned int nSize)
{
	char buf[100];
	unsigned int buf_size;
	int ret;

	if(m_bTextMode) {
		if(nUCIdx&0x8000) {
			buf_size = 2 + sprintf(buf+2, "%s:%04x.", pChannel->GetName(), nUCIdx);
		} else {
			buf_size = 2 + sprintf(buf+2, "%s.", pChannel->GetName());
		}
	} else {
		*((unsigned short*)(buf+sizeof(unsigned short))) = nUCIdx;
		buf_size = sizeof(unsigned short) + sizeof(unsigned short);
	}
	*((unsigned short*)buf) = buf_size - 2 + nSize;

	NETWORK_DOWNBUF* downbufs[10];
	unsigned int count;

	count = network_downbufs_alloc(downbufs, sizeof(downbufs)/sizeof(downbufs[0]), buf_size + nSize);
	ret = network_downbufs_fill(downbufs, count, 0, buf, buf_size);
	assert(ret==ERR_NOERROR);
	ret = network_downbufs_fill(downbufs, count, buf_size, pData, nSize);
	assert(ret==ERR_NOERROR);
	ret = network_send(m_pHandle, downbufs, count);
	assert(ret==ERR_NOERROR);
	if(ret!=ERR_NOERROR) {
		network_downbufs_free(downbufs, count);
		return;
	}
}

void CCubeUser::Disconnect()
{
	if(m_pHandle)
		network_disconnect(m_pHandle);
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

void CCubeUserController::SendData(CCubeUser* pUser, IGameChannel<CCubeUser>* pChannel, unsigned short nUCIdx, const void* pData, unsigned int nSize)
{
}

void CCubeUserController::Disconnect(CCubeUser* pUser)
{
}
