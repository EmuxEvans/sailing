#ifndef _SRP6A_INCLUDE_
#define _SRP6A_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#define SRP_DEFAULT_MIN_BITS		512
#define SRP_SECRET_BITS(len)		256

#define RFC2945_KEY_LEN 40	/* length of session key (bytes) */
#define RFC2945_RESP_LEN 20	/* length of proof hashes (bytes) */

typedef struct srp6a_client_t {
	char			username[40];
	int				username_len;

	bignum_t		password;
	bignum_t		verifier;
	bignum_t		secret;
	bignum_t		pubkey;
	bignum_t		u;
	bignum_t		key;

	bignum_t		modulus;
	bignum_t		generator;
	unsigned char	salt[100];
	int				salt_len;

	SHA1_CTX hash;
	SHA1_CTX ckhash;
	unsigned char k[RFC2945_KEY_LEN];
} srp6a_client_t;

typedef struct srp6a_server_t {
	char			username[30];
	int				username_len;

	bignum_t		password;
	bignum_t		verifier;
	bignum_t		secret;
	bignum_t		pubkey;
	bignum_t		u;
	bignum_t		key;

	bignum_t		modulus;
	bignum_t		generator;
	unsigned char	salt[100];
	int				salt_len;

	SHA1_CTX hash;
	SHA1_CTX ckhash;
	SHA1_CTX oldhash;
	SHA1_CTX oldckhash;
	unsigned char k[RFC2945_KEY_LEN];
	unsigned char r[RFC2945_RESP_LEN];

} srp6a_server_t;

ZION_API void srp6a_client_init(srp6a_client_t* srp);
ZION_API void srp6a_server_init(srp6a_server_t* srp);
ZION_API void srp6a_client_clear(srp6a_client_t* srp);
ZION_API void srp6a_server_clear(srp6a_server_t* srp);

ZION_API int srp6a_client_set_username(srp6a_client_t* srp, const char* username);
ZION_API int srp6a_server_set_username(srp6a_server_t* srp, const char* username);
ZION_API int srp6a_client_set_password(srp6a_client_t* srp, const char* password);
ZION_API int srp6a_server_set_password(srp6a_server_t* srp, const char* password);
ZION_API int srp6a_client_set_param(srp6a_client_t* srp,
			const unsigned char * modulus, int modlen,
			const unsigned char * generator, int genlen,
			const unsigned char * salt, int saltlen);
ZION_API int srp6a_server_set_param(srp6a_server_t* srp,
			const unsigned char * modulus, int modlen,
			const unsigned char * generator, int genlen,
			const unsigned char * salt, int saltlen);
ZION_API int srp6a_client_gen_pub(srp6a_client_t* srp, unsigned char* pkey, int* pkeylen);
ZION_API int srp6a_server_gen_pub(srp6a_server_t* srp, unsigned char* pkey, int* pkeylen);
ZION_API int srp6a_client_comput_key(srp6a_client_t* srp, const unsigned char * key, int keylen, unsigned char * res, int * reslen);
ZION_API int srp6a_server_comput_key(srp6a_server_t* srp, const unsigned char * key, int keylen, unsigned char * res, int * reslen);
ZION_API void srp6a_client_respond(srp6a_client_t* srp, unsigned char * proof, int * prooflen);
ZION_API void srp6a_server_respond(srp6a_server_t* srp, unsigned char * proof, int * prooflen);
ZION_API int srp6a_server_verify(srp6a_server_t* srp, const unsigned char * proof, int prooflen);
ZION_API int srp6a_client_verify(srp6a_client_t* srp, const unsigned char * proof, int prooflen);

#ifdef __cplusplus
}
#endif

#endif
