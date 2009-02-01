#include <stdio.h>
#include <assert.h>

#include <skates/skates.h>

#include "../common/GameUser.h"
#include "../common/GameRoom.h"
#include "../common/GameUser.inl"
#include "../common/GameRoom.inl"

#include "../Cube.proto.h"
#include "../Cube.proto.h"
#include "../Cube.net.proto.h"
#include "../Cube.net.clt.hpp"
#include "CubeClient.proto.h"
#include "CubeClient.h"

static void do_test();

int main(int argc, char* argv[])
{
	sock_init();
	mempool_init();
	fdwatch_init();
	threadpool_init(1);
	network_init(1000);
	dymempool_init(1024, 12*1024);

	do_test();

	dymempool_final();
	network_final();
	threadpool_final();
	fdwatch_final();
	mempool_final();
	sock_final();

	return 0;
}

#include <skates/srp6a_key.inc>

class CLoginClientProcess : public CLoginClientHook
{
public:
	CLoginClientProcess(CCubeClient* pClient) : CLoginClientHook("LoginClientProcess") {
		m_pClient = pClient;
		srp6a_client_init(&srpc);
		m_pClient->m_Login.SetHook(this);
	}
	~CLoginClientProcess() {
		m_pClient = NULL;
	}

	void Login(const char* username, const char* password) {
		int ret;

		srp6a_client_clear(&srpc);
		srp6a_client_init(&srpc);
		ret = srp6a_client_set_username(&srpc, username);
		assert(ret==0);
		m_pClient->GetLogin()->begin((char*)username);
		strcpy(m_Password, password);
	}

	virtual void pubkey_callback(LoginSalt* salt, LoginPubkey* pubkey)
	{
		LoginPubkey _pubkey;
		LoginSession _session;
		LoginProof _proof;
		int ret;

		ret = srp6a_client_set_param(&srpc, (const unsigned char*)srp6a_keys[0].modulus, srp6a_keys[0].bits, (const unsigned char*)srp6a_keys[0].generator, 1, salt->data, salt->data_array_size);
		assert(ret==0);
		ret = srp6a_client_set_password(&srpc, "password");
		assert(ret==0);
		_pubkey.data_array_size = sizeof(_pubkey.data);
		ret = srp6a_client_gen_pub(&srpc, _pubkey.data, &_pubkey.data_array_size);
		assert(ret==0);
		_session.data_array_size = sizeof(_session.data);
		ret = srp6a_client_comput_key(&srpc, pubkey->data, pubkey->data_array_size, _session.data, &_session.data_array_size);
		assert(ret==0);
		_proof.data_array_size = sizeof(_proof.data);
		srp6a_client_respond(&srpc, _proof.data, &_proof.data_array_size);
		m_pClient->GetLogin()->verify(&_pubkey, &_proof);
	}

	virtual void verify_callback(LoginProof* proof)
	{
		int ret;

		ret = srp6a_client_verify(&srpc, proof->data, proof->data_array_size);
		assert(ret==0);
		printf("done\n");
	}

	virtual void on_error(os_int code)
	{
	}

private:
	CCubeClient* m_pClient;
	srp6a_client_t srpc;
	char m_Password[100];
};

void do_test()
{
	CCubeClient Client;
	CLoginClientProcess ClientProcess(&Client);

	Client.Connect("127.0.0.1:9527");
	printf("press any key, begin\n");
	getchar();
	ClientProcess.Login("username", "password");
	getchar();


}
