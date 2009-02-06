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

CServerLoginChannel::CServerLoginChannel()
{
	m_pHooks = this;
	SetChannel("Login", 0, false);
}

CServerLoginChannel::~CServerLoginChannel()
{
}

bool CServerLoginChannel::Join(CCubeUser* pUser)
{
	pUser->BindChannel(this, 0, m_nUCIdx);
	m_pUser = pUser;
	return true;
}

void CServerLoginChannel::OnData(CCubeUser* pUser, unsigned int nCIdx, const void* pData, unsigned int nSize)
{
	if(!Dispatch(pData, nSize)) {
		assert(0);
		pUser->Disconnect();
	}
}

void CServerLoginChannel::Disconnect(CCubeUser* pUser, unsigned int nCIdx)
{
	pUser->UnbindChannel(this, 0, m_nUCIdx);
	m_pUser = NULL;
}

char g_SendBuf[1000];

char* CServerLoginChannel::GetSendBuf(unsigned int& nSendBufSize)
{
	nSendBufSize = sizeof(g_SendBuf);
	return g_SendBuf;
}

void CServerLoginChannel::SendBuf(const char* pSendBuf, unsigned int nSendBufSize)
{
	m_pUser->SendData(this, m_nUCIdx, pSendBuf, nSendBufSize);
}

#include <skates/srp6a_key.inc>

void CServerLoginChannel::begin(char* username)
{
	int ret;
	LoginSalt _salt;
	LoginPubkey _pubkey;

	if(step!=LOGINSTEP_PUBKEY) {
		m_pUser->Disconnect();
		return;
	}

	if(strcmp(username, _username)!=0) {
		on_error(-1);
		step = LOGINSTEP_ERROR;
		return;
	}

	randbytes(_salt.data, sizeof(_salt.data));
	_salt.data_array_size = sizeof(_salt.data);
	ret = srp6a_server_set_username(&srps, _username);
	assert(ret==0);
	ret = srp6a_server_set_param(&srps, (const unsigned char*)srp6a_keys[0].modulus, srp6a_keys[0].bits, (const unsigned char*)srp6a_keys[0].generator, 1, _salt.data, _salt.data_array_size);
	assert(ret==0);
	ret = srp6a_server_set_password(&srps, _password);
	assert(ret==0);
	_pubkey.data_array_size = sizeof(_pubkey.data);
	ret = srp6a_server_gen_pub(&srps, _pubkey.data, &_pubkey.data_array_size);
	pubkey_callback(&_salt, &_pubkey);

	step = LOGINSTEP_PROOF;
}

void CServerLoginChannel::verify(LoginPubkey* pubkey, LoginProof* proof)
{
	int ret;
	LoginSession _session;
	LoginProof _proof;

	if(step!=LOGINSTEP_PROOF) {
		m_pUser->Disconnect();
		return;
	}

	_session.data_array_size = sizeof(_session.data);
	ret = srp6a_server_comput_key(&srps, pubkey->data, pubkey->data_array_size, _session.data, &_session.data_array_size);
	assert(ret==0);

	ret = srp6a_server_verify(&srps, proof->data, proof->data_array_size);
	assert(ret==0);
	_proof.data_array_size = sizeof(_proof.data);
	srp6a_server_respond(&srps, _proof.data, &_proof.data_array_size);
	verify_callback(&_proof);

	step = LOGINSTEP_DONE;
}
