#ifndef _BIGNUM_INCLUDE_
#define _BIGNUM_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#include "tommath/tommath.h"

typedef struct bignum_t {
	mp_int		_int;
} bignum_t;

ZION_API void bignum_init(bignum_t* num);
ZION_API void bignum_clear(bignum_t* num);

ZION_API int bignum_from_bin(bignum_t* num, const unsigned char* buf, int buflen);
ZION_API int bignum_to_bin(bignum_t* num, unsigned char* buf, int* buflen);
ZION_API int bignum_from_int(bignum_t* num, int value);

ZION_API int bignum_modexp(bignum_t* r, bignum_t* b, bignum_t* e, bignum_t* m);
ZION_API int bignum_modmul(bignum_t* r, bignum_t* m1, bignum_t* m2, bignum_t* modulus);

ZION_API int bignum_mul(bignum_t* r, bignum_t* m1, bignum_t* m2);
ZION_API int bignum_mod(bignum_t* r, bignum_t* d, bignum_t* m);

ZION_API int bignum_add(bignum_t* result, bignum_t* a1, bignum_t* a2);
ZION_API int bignum_add_int(bignum_t* result, bignum_t* a1, int a2);

ZION_API int bignum_sub(bignum_t* result, bignum_t* a1, bignum_t* a2);

ZION_API int bignum_cmp(bignum_t* num, bignum_t* v);
ZION_API int bignum_cmp_int(bignum_t* num, int v);

ZION_API int bignum_bitlen(bignum_t* num);
ZION_API int bignum_bytelen(bignum_t* num);

#ifdef __cplusplus
}
#endif

#endif
