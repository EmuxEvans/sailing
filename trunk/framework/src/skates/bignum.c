#include <assert.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/bignum.h"

void bignum_init(bignum_t* num)
{
	mp_init(&num->_int);
}

void bignum_clear(bignum_t* num)
{
	mp_clear(&num->_int);
}

int bignum_from_bin(bignum_t* num, const unsigned char* buf, int buflen)
{
	bignum_init(num);
	return mp_read_unsigned_bin(&num->_int, buf, buflen)==MP_OKAY?1:0;
}

int bignum_to_bin(bignum_t* num, unsigned char* buf, int* buflen)
{
	int len;
	len = mp_unsigned_bin_size(&num->_int);
	if(*buflen<len) {
		assert(0);
		return 0;
	}
	mp_to_unsigned_bin(&num->_int, buf);
	*buflen = len;
	return 1;
}

int bignum_from_int(bignum_t* num, int value)
{
	bignum_init(num);
	return mp_set_int(&num->_int, value)==MP_OKAY?1:0;
}

int bignum_modexp(bignum_t* r, bignum_t* b, bignum_t* e, bignum_t* m)
{
	return mp_exptmod(&b->_int, &e->_int, &m->_int, &r->_int)==MP_OKAY?1:0;
}

int bignum_modmul(bignum_t* r, bignum_t* m1, bignum_t* m2, bignum_t* modulus)
{
	return mp_mulmod(&m1->_int, &m2->_int, &modulus->_int, &r->_int)==MP_OKAY?1:0;
}

int bignum_mul(bignum_t* r, bignum_t* m1, bignum_t* m2)
{
	return mp_mul(&m1->_int, &m2->_int, &r->_int)==MP_OKAY?1:0;
}

int bignum_mod(bignum_t* r, bignum_t* d, bignum_t* m)
{
	return mp_mod(&d->_int, &m->_int, &r->_int)==MP_OKAY?1:0;
}

int bignum_add(bignum_t* result, bignum_t* a1, bignum_t* a2)
{
	return mp_add(&a1->_int, &a2->_int, &result->_int)==MP_OKAY?1:0;
}

int bignum_add_int(bignum_t* result, bignum_t* a1, int a2)
{
	return mp_add_d(&a1->_int, a2, &result->_int)==MP_OKAY?1:0;
}

int bignum_sub(bignum_t* result, bignum_t* a1, bignum_t* a2)
{
	return mp_sub(&a1->_int, &a2->_int, &result->_int)==MP_OKAY?1:0;
}

int bignum_cmp(bignum_t* num, bignum_t* v)
{
	return mp_cmp(&num->_int, &v->_int);
}

int bignum_cmp_int(bignum_t* num, int v)
{
	return mp_cmp_d(&num->_int, v);
}

int bignum_bitlen(bignum_t* num)
{
	return mp_count_bits(&num->_int);
}

int bignum_bytelen(bignum_t* num)
{
	return (mp_count_bits(&num->_int) + 7) / 8;
}
