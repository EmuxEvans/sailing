#ifndef _SHA1_INCLUDE_
#define _SHA1_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#define SHA_DIGESTSIZE 20

typedef struct SHA1_CTXT {
	union {
		os_byte		b8[20];
		os_dword	b32[5];
	}			h;
	union {
		os_byte		b8[8];
		os_qword	b64[1];
	}			c;
	union {
		os_byte		b8[64];
		os_dword	b32[16];
	}			m;
	os_byte		count;
} SHA1_CTXT;

ZION_API void SHA1Init(SHA1_CTXT* ctx);
ZION_API void SHA1Update(SHA1_CTXT* ctx, const unsigned char *, size_t);
ZION_API void SHA1Final(unsigned char *, SHA1_CTXT* ctx);

#ifdef __cplusplus
}
#endif

#endif
