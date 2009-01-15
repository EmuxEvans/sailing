#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/sha1.h>
#include <skates/bignum.h>
#include <skates/srp6a.h>
#include <skates/dymempool.h>

static const char* username = "user";
static const char* password = "password";
static unsigned char modulus[10000];
static int modlen;
static unsigned char generator[10000];
static int genlen;
static unsigned char salt[100];
static int saltlen;

void dotest()
{
	int ret;
	unsigned char cbuf[20000]; int cbuf_len;
	unsigned char sbuf[20000]; int sbuf_len;
	unsigned char ks[20000]; int ks_len;
	unsigned char kc[20000]; int kc_len;
	srp6a_client_t srpc;
	srp6a_server_t srps;

	srp6a_client_init(&srpc);
	srp6a_server_init(&srps);

	// step 1:
	srp6a_client_set_username(&srpc, username);
	// printf("[client] send username\n");
	srp6a_client_set_param(&srpc, modulus, modlen, generator, genlen, salt, saltlen);
	cbuf_len = sizeof(cbuf);
	srp6a_client_gen_pub(&srpc, cbuf, &cbuf_len); // A => cbuf
	// printf("[client] send A\n");

	// step 2:
	srp6a_server_set_username(&srps, username);
	srp6a_server_set_param(&srps, modulus, modlen, generator, genlen, salt, saltlen);
	srp6a_server_set_password(&srps, password);
	sbuf_len = sizeof(sbuf);
	srp6a_server_gen_pub(&srps, sbuf, &sbuf_len); // B => sbuf
	// printf("[server] send B\n");

	// step 3:
	ks_len = sizeof(ks);
	srp6a_server_comput_key(&srps, cbuf, cbuf_len, ks, &ks_len); // session_key => sess
	// printf("[server] send session key\n");

	// step 4:
	srp6a_client_set_password(&srpc, password);
	kc_len = sizeof(kc);
	srp6a_client_comput_key(&srpc, sbuf, sbuf_len, kc, &kc_len);

	// step 5:
	kc_len = sizeof(kc);
	srp6a_client_respond(&srpc, kc, &kc_len);
	// printf("[client] send client proof\n");

	// step 6:
	ret = srp6a_server_verify(&srps, kc, kc_len);
	assert(ret==0);
	sbuf_len = sizeof(sbuf);
	srp6a_server_respond(&srps, sbuf, &sbuf_len);
	// printf("[server] send server proof\n");

	// step 7:
	ret = srp6a_client_verify(&srpc, sbuf, sbuf_len);
	assert(ret==0);

	//
	srp6a_client_clear(&srpc);
	srp6a_server_clear(&srps);
}

#include <skates/srp6a_key.inc>

int main(int argc, char* argv[])
{
	int i, j;

	memset(salt, 0xf0, 10);
	saltlen = 10;
	dymempool_init(1000, 10*1024);

	for(i=0; i<sizeof(srp6a_keys)/sizeof(srp6a_keys[0]); i++) {
		if(i==8) continue;
		memcpy(modulus, srp6a_keys[i].modulus, srp6a_keys[i].bits);
		modlen = srp6a_keys[i].bits;
		memcpy(generator, srp6a_keys[i].generator, 1);
		genlen = 1;

		printf("do test %d\n", i);
		for(j=0; j<100; j++) {
			dotest();
		}
	}

	dymempool_final();

	return 0;
}
