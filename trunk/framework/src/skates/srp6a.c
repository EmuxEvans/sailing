#include <stdlib.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/bignum.h"
#include "../../inc/skates/sha1.h"
#include "../../inc/skates/srp6a.h"

static void t_mgf1(unsigned char * mask, unsigned masklen, const unsigned char * seed, unsigned seedlen);

void srp6a_client_init(srp6a_client_t* srp)
{
	memset(srp, 0, sizeof(*srp));
	SHA1Init(&srp->hash);
	SHA1Init(&srp->ckhash);
}

void srp6a_server_init(srp6a_server_t* srp)
{
	memset(srp, 0, sizeof(*srp));
	SHA1Init(&srp->hash);
	SHA1Init(&srp->ckhash);
	SHA1Init(&srp->oldhash);
	SHA1Init(&srp->oldckhash);
}

void srp6a_client_clear(srp6a_client_t* srp)
{
	if(srp->password._int.dp!=NULL)	bignum_clear(&srp->password);
	if(srp->verifier._int.dp!=NULL)	bignum_clear(&srp->verifier);
	if(srp->secret._int.dp!=NULL)	bignum_clear(&srp->secret);
	if(srp->pubkey._int.dp!=NULL)	bignum_clear(&srp->pubkey);
	if(srp->u._int.dp!=NULL)		bignum_clear(&srp->u);
	if(srp->key._int.dp!=NULL)		bignum_clear(&srp->key);
	if(srp->modulus._int.dp!=NULL)	bignum_clear(&srp->modulus);
	if(srp->generator._int.dp!=NULL)bignum_clear(&srp->generator);
}

void srp6a_server_clear(srp6a_server_t* srp)
{
	if(srp->password._int.dp!=NULL)	bignum_clear(&srp->password);
	if(srp->verifier._int.dp!=NULL)	bignum_clear(&srp->verifier);
	if(srp->secret._int.dp!=NULL)	bignum_clear(&srp->secret);
	if(srp->pubkey._int.dp!=NULL)	bignum_clear(&srp->pubkey);
	if(srp->u._int.dp!=NULL)		bignum_clear(&srp->u);
	if(srp->key._int.dp!=NULL)		bignum_clear(&srp->key);
	if(srp->modulus._int.dp!=NULL)	bignum_clear(&srp->modulus);
	if(srp->generator._int.dp!=NULL)bignum_clear(&srp->generator);
}

int srp6a_client_set_username(srp6a_client_t* srp, const char* username)
{
	srp->username_len = strlen(username);
	if(srp->username_len>sizeof(srp->username)) return -1;
	memcpy(srp->username, username, srp->username_len);
	return 0;
}

int srp6a_server_set_username(srp6a_server_t* srp, const char* username)
{
	srp->username_len = strlen(username);
	if(srp->username_len>sizeof(srp->username)) return -1;
	memcpy(srp->username, username, srp->username_len);
	return 0;
}

int srp6a_client_set_password(srp6a_client_t* srp, const char* password)
{
	SHA1_CTXT ctxt;
	unsigned char dig[SHA_DIGESTSIZE];

	SHA1Init(&ctxt);
	SHA1Update(&ctxt, (const unsigned char*)srp->username, (size_t)(srp->username_len));
	SHA1Update(&ctxt, (const unsigned char*)":", 1);
	SHA1Update(&ctxt, (const unsigned char*)password, strlen(password));
	SHA1Final(dig, &ctxt);	/* dig = H(U | ":" | P) */

	SHA1Init(&ctxt);
	SHA1Update(&ctxt, srp->salt, srp->salt_len);
	SHA1Update(&ctxt, dig, sizeof(dig));
	SHA1Final(dig, &ctxt);	/* dig = H(s | H(U | ":" | P)) */
	memset(&ctxt, 0, sizeof(ctxt));

	bignum_from_bin(&srp->password, dig, sizeof(dig));
	memset(dig, 0, sizeof(dig));

	/* verifier = g^x mod N */
	bignum_from_int(&srp->verifier, 0);
	bignum_modexp(&srp->verifier, &srp->generator, &srp->password, &srp->modulus);

	return 0;
}

int srp6a_server_set_password(srp6a_server_t* srp, const char* password)
{
	SHA1_CTXT ctxt;
	unsigned char dig[SHA_DIGESTSIZE];

	SHA1Init(&ctxt);
	SHA1Update(&ctxt, (const unsigned char*)srp->username, srp->username_len);
	SHA1Update(&ctxt, (const unsigned char*)":", 1);
	SHA1Update(&ctxt, (const unsigned char*)password, strlen(password));
	SHA1Final(dig, &ctxt);	/* dig = H(U | ":" | P) */

	SHA1Init(&ctxt);
	SHA1Update(&ctxt, srp->salt, srp->salt_len);
	SHA1Update(&ctxt, dig, sizeof(dig));
	SHA1Final(dig, &ctxt);	/* dig = H(s | H(U | ":" | P)) */
	memset(&ctxt, 0, sizeof(ctxt));

	bignum_from_bin(&srp->password, dig, sizeof(dig));
	memset(dig, 0, sizeof(dig));

	/* verifier = g^x mod N */
	bignum_from_int(&srp->verifier, 0);
	bignum_modexp(&srp->verifier, &srp->generator, &srp->password, &srp->modulus);

	return 0;
}

int srp6a_client_set_param(srp6a_client_t* srp,
			const unsigned char * modulus, int modlen,
			const unsigned char * generator, int genlen,
			const unsigned char * salt, int saltlen)
{
	int i;
	unsigned char buf1[SHA_DIGESTSIZE], buf2[SHA_DIGESTSIZE];
	SHA1_CTXT ctxt;

	if(!bignum_from_bin(&srp->modulus, modulus, modlen)) {
		return -1;
	}
	if(!bignum_from_bin(&srp->generator, generator, genlen)) {
		return -1;
	}
	if(saltlen>sizeof(srp->salt) || saltlen<0) {
		return -1;
	}
	memcpy(srp->salt, salt, saltlen);
	srp->salt_len = saltlen;
	// check modules_bit > SRP_DEFAULT_MIN_BITS 512

	/************************************************/

	/* Update hash state */
	SHA1Init(&ctxt);
	SHA1Update(&ctxt, modulus, modlen);
	SHA1Final(buf1, &ctxt);	/* buf1 = H(modulus) */

	SHA1Init(&ctxt);
	SHA1Update(&ctxt, generator, genlen);
	SHA1Final(buf2, &ctxt);	/* buf2 = H(generator) */

	for(i = 0; i < sizeof(buf1); ++i)
	buf1[i] ^= buf2[i];		/* buf1 = H(modulus) xor H(generator) */

	/* hash: H(N) xor H(g) */
	SHA1Update(&srp->hash, buf1, sizeof(buf1));

	SHA1Init(&ctxt);
	SHA1Update(&ctxt, (const unsigned char*)srp->username, srp->username_len);
	SHA1Final(buf1, &ctxt);	/* buf1 = H(user) */

	/* hash: (H(N) xor H(g)) | H(U) */
	SHA1Update(&srp->hash, buf1, sizeof(buf1));

	/* hash: (H(N) xor H(g)) | H(U) | s */
	SHA1Update(&srp->hash, salt, saltlen);

	return 0;
}

int srp6a_server_set_param(srp6a_server_t* srp,
			const unsigned char * modulus, int modlen,
			const unsigned char * generator, int genlen,
			const unsigned char * salt, int saltlen)
{
	unsigned char buf1[SHA_DIGESTSIZE], buf2[SHA_DIGESTSIZE];
	SHA1_CTXT ctxt;
	int i;

	if(!bignum_from_bin(&srp->modulus, modulus, modlen)) {
		return -1;
	}
	if(!bignum_from_bin(&srp->generator, generator, genlen)) {
		return -1;
	}
	if(saltlen>sizeof(srp->salt) || saltlen<0) {
		return -1;
	}
	memcpy(srp->salt, salt, saltlen);
	srp->salt_len = saltlen;
	// check modules_bit > SRP_DEFAULT_MIN_BITS 512

	/************************************************/

	/* Update hash state */
	SHA1Init(&ctxt);
	SHA1Update(&ctxt, modulus, modlen);
	SHA1Final(buf1, &ctxt);	/* buf1 = H(modulus) */

	SHA1Init(&ctxt);
	SHA1Update(&ctxt, generator, genlen);
	SHA1Final(buf2, &ctxt);	/* buf2 = H(generator) */

	for(i = 0; i < sizeof(buf1); ++i)
	buf1[i] ^= buf2[i];		/* buf1 = H(modulus) XOR H(generator) */

	/* ckhash: H(N) xor H(g) */
	SHA1Update(&srp->ckhash, buf1, sizeof(buf1));

	SHA1Init(&ctxt);
	SHA1Update(&ctxt, (const unsigned char*)srp->username, srp->username_len);
	SHA1Final(buf1, &ctxt);	/* buf1 = H(user) */

	/* ckhash: (H(N) xor H(g)) | H(U) */
	SHA1Update(&srp->ckhash, buf1, sizeof(buf1));

	/* ckhash: (H(N) xor H(g)) | H(U) | s */
	SHA1Update(&srp->ckhash, salt, saltlen);

	return 0;
}

int srp6a_client_gen_pub(srp6a_client_t* srp, unsigned char* pkey, int* pkeylen)
{
	int slen = (SRP_SECRET_BITS(bignum_bitlen(&srp->modulus)) + 7) / 8;

	if(slen>*pkeylen) return -1;

	memset(pkey, 0xee, (size_t)slen);  // = t_random(data, slen);
	bignum_from_bin(&srp->secret, pkey, slen);

	/* Force g^a mod n to "wrap around" by adding log[2](n) to "a". */
	bignum_add_int(&srp->secret, &srp->secret, bignum_bitlen(&srp->modulus));
	/* A = g^a mod n */
	bignum_from_int(&srp->pubkey, 0);
	bignum_modexp(&srp->pubkey, &srp->generator, &srp->secret, &srp->modulus);
	bignum_to_bin(&srp->pubkey, pkey, pkeylen);
	/* hash: (H(N) xor H(g)) | H(U) | s | A */
	SHA1Update(&srp->hash, pkey, *pkeylen);
	/* ckhash: A */
	SHA1Update(&srp->ckhash, pkey, *pkeylen);

	return 0;
}

int srp6a_server_gen_pub(srp6a_server_t* srp, unsigned char* pkey, int* pkeylen)
{
	//SRP_RESULT ret;
	//BigInteger k;
	//cstr * s;
	SHA1_CTXT ctxt;
	unsigned char dig[SHA_DIGESTSIZE];
	unsigned char* modulus;
	int modlen, genlen;
	bignum_t k;
	int slen;

	// gen k
	modlen = bignum_bytelen(&srp->modulus);
	modulus = (unsigned char*)alloca(modlen);
	if(modulus==NULL)
		return -1;
	bignum_to_bin(&srp->modulus, modulus, &modlen);
	SHA1Init(&ctxt);
	SHA1Update(&ctxt, modulus, modlen);
	genlen = modlen;
	bignum_to_bin(&srp->generator, modulus, &genlen);
	memmove(modulus+modlen-genlen, modulus, genlen);
	memset(modulus, 0, modlen-genlen);
	SHA1Update(&ctxt, modulus, modlen);
	SHA1Final(dig, &ctxt);

	bignum_from_bin(&k, dig, SHA_DIGESTSIZE);
	if(bignum_cmp_int(&k, 0)==0) {
		bignum_clear(&k);
		return -1;
	}

	//
	slen = (SRP_SECRET_BITS(bignum_bitlen(&srp->modulus)) + 7) / 8;

	memset(pkey, 0xee, (size_t)slen);  // = t_random(data, slen);
	bignum_from_bin(&srp->secret, pkey, slen);
	bignum_from_int(&srp->pubkey, 0);

	/* B = kv + g^b mod n (blinding) */
	bignum_mul(&srp->pubkey, &k, &srp->verifier);
	bignum_modexp(&k, &srp->generator, &srp->secret, &srp->modulus);
	bignum_add(&k, &k, &srp->pubkey);
	bignum_mod(&srp->pubkey, &k, &srp->modulus);

	bignum_to_bin(&srp->pubkey, pkey, pkeylen);

	/* oldckhash: B */
	SHA1Update(&srp->oldckhash, pkey, *pkeylen);

	bignum_clear(&k);
	return 0;
}

int srp6a_client_comput_key(srp6a_client_t* srp, const unsigned char * key, int keylen, unsigned char * res, int * reslen)
{
	bignum_t k;
	SHA1_CTXT ctxt;
	unsigned char dig[SHA_DIGESTSIZE];
	unsigned char* modulus;
	int modlen, genlen;
	bignum_t gb, e;

	modlen = bignum_bytelen(&srp->modulus);
	modulus = (unsigned char*)alloca(modlen);
	if(modulus==NULL)
		return -1;
	bignum_to_bin(&srp->modulus, modulus, &modlen);
	SHA1Init(&ctxt);
	SHA1Update(&ctxt, modulus, modlen);
	genlen = modlen;
	bignum_to_bin(&srp->generator, modulus, &genlen);
	memmove(modulus+modlen-genlen, modulus, genlen);
	memset(modulus, 0, modlen-genlen);
	SHA1Update(&ctxt, modulus, modlen);
	SHA1Final(dig, &ctxt);

	bignum_from_bin(&k, dig, SHA_DIGESTSIZE);
	if(bignum_cmp_int(&k, 0)==0) {
		bignum_clear(&k);
		return -1;
	}

	//
	bignum_to_bin(&srp->modulus, modulus, &modlen);
	if(keylen > modlen) {
		bignum_clear(&k);
		return -1;
	}

	/* Compute u from client's and server's values */
	SHA1Init(&ctxt);
	/* Use s as a temporary to store client's value */
	bignum_to_bin(&srp->pubkey, modulus, &modlen);
    SHA1Update(&ctxt, modulus, modlen);
    if(keylen < modlen) {
      memcpy(modulus + (modlen - keylen), key, keylen);
      memset(modulus, 0, modlen - keylen);
      SHA1Update(&ctxt, modulus, modlen);
    } else 
      SHA1Update(&ctxt, key, keylen);
	SHA1Final(dig, &ctxt);
	bignum_from_bin(&srp->u, dig, SHA_DIGESTSIZE);

	/* hash: (H(N) xor H(g)) | H(U) | s | A | B */
	SHA1Update(&srp->hash, key, keylen);

	bignum_from_bin(&gb, key, keylen);
	/* reject B == 0, B >= modulus */
	if(bignum_cmp(&gb, &srp->modulus) >= 0 || bignum_cmp_int(&gb, 0) == 0) {
		bignum_clear(&k);
		bignum_clear(&gb);
		return -1;
	}
	bignum_from_int(&e, 0);
	bignum_from_int(&srp->key, 0);
	/* unblind g^b (mod N) */
	bignum_sub(&srp->key, &srp->modulus, &srp->verifier);
	/* use e as temporary, e == -k*v (mod N) */
	bignum_mul(&e, &k, &srp->key);
	bignum_add(&e, &e, &gb);
	bignum_mod(&gb, &e, &srp->modulus);

	/* compute gb^(a + ux) (mod N) */
	bignum_mul(&e, &srp->password, &srp->u);
	bignum_add(&e, &e, &srp->secret);	/* e = a + ux */

	bignum_modexp(&srp->key, &gb, &e, &srp->modulus);

	/* convert srp->key into a session key, update hash states */
	bignum_to_bin(&srp->key, modulus, &modlen);
	t_mgf1(srp->k, RFC2945_KEY_LEN, modulus, modlen); /* Interleaved hash */

	/* hash: (H(N) xor H(g)) | H(U) | s | A | B | K */
	SHA1Update(&srp->hash, srp->k, RFC2945_KEY_LEN);
	/* hash: (H(N) xor H(g)) | H(U) | s | A | B | K | ex_data */

	memcpy(res, srp->k, RFC2945_KEY_LEN);
	*reslen = RFC2945_KEY_LEN;

	bignum_clear(&k);
	bignum_clear(&gb);
	bignum_clear(&e);
	return 0;
}

int srp6a_server_comput_key(srp6a_server_t* srp, const unsigned char * pubkey, int pubkeylen, unsigned char * res, int * reslen)
{
	bignum_t t1, t2, t3;
	SHA1_CTXT ctxt;
	unsigned char dig[SHA_DIGESTSIZE];
	int modlen;
	unsigned char *s;
	int slen, mlen;

	modlen = bignum_bytelen(&srp->modulus);
	if(pubkeylen > modlen)
		return -1;

	/* ckhash: (H(N) xor H(g)) | H(U) | s | A */
	SHA1Update(&srp->ckhash, pubkey, pubkeylen);

	slen = mlen = bignum_bytelen(&srp->pubkey) + 10;
	s = (unsigned char*)alloca(slen);
	bignum_to_bin(&srp->pubkey, s, &slen);
	/* get encoding of B */

	/* ckhash: (H(N) xor H(g)) | H(U) | s | A | B */
	SHA1Update(&srp->ckhash, s, slen);

	/* hash: A */
	SHA1Update(&srp->hash, pubkey, pubkeylen);
	/* oldhash: A */
	SHA1Update(&srp->oldhash, pubkey, pubkeylen);

	/* Compute u = H(pubkey || mypubkey) */
	SHA1Init(&ctxt);
	SHA1Update(&ctxt, pubkey, pubkeylen);
	SHA1Update(&ctxt, s, slen);
	SHA1Final(dig, &ctxt);	/* dig = H(A || B) */
	bignum_from_bin(&srp->u, dig, SHA_DIGESTSIZE);

	/* compute A*v^u */
	bignum_from_int(&t1, 0);
	bignum_modexp(&t1, &srp->verifier, &srp->u, &srp->modulus); /* t1 = v^u */
	bignum_from_bin(&t2, pubkey, pubkeylen); /* t2 = A */
	bignum_from_int(&t3, 0);
	bignum_modmul(&t3, &t2, &t1, &srp->modulus);

	if(bignum_cmp_int(&t3, 1) <= 0) {	/* Reject A*v^u == 0,1 (mod N) */
		bignum_clear(&t1);
		bignum_clear(&t2);
		bignum_clear(&t3);
		return -1;
	}

	bignum_add_int(&t1, &t3, 1);
	if(bignum_cmp(&t1, &srp->modulus) == 0) {  /* Reject A*v^u == -1 (mod N) */
		bignum_clear(&t1);
		bignum_clear(&t2);
		bignum_clear(&t3);
		return -1;
	}

	bignum_from_int(&srp->key, 0);
	bignum_modexp(&srp->key, &t3, &srp->secret, &srp->modulus);  /* (Av^u)^b */

	/* convert srp->key into session key, update hashes */
	slen = mlen;
	bignum_to_bin(&srp->key, s, &slen);
	t_mgf1(srp->k, RFC2945_KEY_LEN, s, slen); /* Interleaved hash */

	/* ckhash: (H(N) xor H(g)) | H(U) | s | A | B | K */
	SHA1Update(&srp->ckhash, srp->k, RFC2945_KEY_LEN);
	/* ckhash: (H(N) xor H(g)) | H(U) | s | A | B | K | ex_data */

	/* oldhash: A | K */
	SHA1Update(&srp->oldhash, srp->k, RFC2945_KEY_LEN);
	/* oldckhash: B | K */
	SHA1Update(&srp->oldckhash, srp->k, RFC2945_KEY_LEN);

	if(*reslen<RFC2945_KEY_LEN)
		return -1;
	memcpy(res, srp->k, RFC2945_KEY_LEN);
	*reslen = RFC2945_KEY_LEN;

	bignum_clear(&t1);
	bignum_clear(&t2);
	bignum_clear(&t3);

	return 0;
}

void srp6a_client_respond(srp6a_client_t* srp, unsigned char * proof, int * prooflen)
{
	SHA1Final(proof, &srp->hash);
	*prooflen = RFC2945_RESP_LEN;

	/* ckhash: A | M | K */
	SHA1Update(&srp->ckhash, proof, *prooflen);
	SHA1Update(&srp->ckhash, srp->k, RFC2945_KEY_LEN);
}

void srp6a_server_respond(srp6a_server_t* srp, unsigned char * proof, int * prooflen)
{
	memcpy(proof, srp->r, RFC2945_RESP_LEN);
	*prooflen = RFC2945_RESP_LEN;
}

int srp6a_server_verify(srp6a_server_t* srp, const unsigned char * proof, int prooflen)
{
	unsigned char expected[SHA_DIGESTSIZE];

	SHA1Final(expected, &srp->oldckhash);
	if(prooflen==RFC2945_RESP_LEN && memcmp(expected, proof, RFC2945_RESP_LEN)==0) {
		SHA1Final(srp->r, &srp->oldhash);
		return 0;
	}
	SHA1Final(expected, &srp->ckhash);
	if(prooflen==RFC2945_RESP_LEN && memcmp(expected, proof, RFC2945_RESP_LEN)==0) {
		/* hash: A | M | K */
		SHA1Update(&srp->hash, expected, sizeof(expected));
		SHA1Update(&srp->hash, srp->k, RFC2945_KEY_LEN);
		SHA1Final(srp->r, &srp->hash);
		return 0;
	}
	return -1;
}

int srp6a_client_verify(srp6a_client_t* srp, const unsigned char * proof, int prooflen)
{
	unsigned char expected[SHA_DIGESTSIZE];
	SHA1Final(expected, &srp->ckhash);
	return prooflen==RFC2945_RESP_LEN && memcmp(expected, proof, RFC2945_RESP_LEN)==0?0:-1;
}

void t_mgf1(unsigned char * mask, unsigned masklen, const unsigned char * seed, unsigned seedlen)
{
	SHA1_CTXT ctxt;
	unsigned i = 0;
	unsigned pos = 0;
	unsigned char cnt[4];
	unsigned char hout[SHA_DIGESTSIZE];

	while(pos < masklen) {
		cnt[0] = (i >> 24) & 0xFF;
		cnt[1] = (i >> 16) & 0xFF;
		cnt[2] = (i >> 8) & 0xFF;
		cnt[3] = i & 0xFF;
		SHA1Init(&ctxt);
		SHA1Update(&ctxt, seed, seedlen);
		SHA1Update(&ctxt, cnt, 4);

		if(pos + SHA_DIGESTSIZE > masklen) {
			SHA1Final(hout, &ctxt);
			memcpy(mask + pos, hout, masklen - pos);
			pos = masklen;
		}
		else {
			SHA1Final(mask + pos, &ctxt);
			pos += SHA_DIGESTSIZE;
		}

		++i;
	}

	memset(hout, 0, sizeof(hout));
	memset((unsigned char *)&ctxt, 0, sizeof(ctxt));
}
