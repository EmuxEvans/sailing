#ifndef _NEW_MISC_H_
#define _NEW_MISC_H_

ZION_API void strtrim(char* str);
ZION_API void strltrim(char* str);

ZION_API int hex2bin(const char* hex, unsigned char* buf, int buflen);
ZION_API void bin2hex(const unsigned char* buf, int buflen, char* hex);

ZION_API const char* strget(const char* str, char split, char* out, size_t out_size);
ZION_API const char* strget2int(const char* str, char split, int* value);
ZION_API const char* strget2float(const char* str, char split, float* value);

ZION_API char* strput(char* str, size_t len, char split, const char* in);
ZION_API char* strput4int(char* str, size_t len, char split, int value);
ZION_API char* strput4float(char* str, size_t len, char split, float value);

ZION_API const char* strget_space(const char* str, char* out, size_t out_size);
ZION_API const char* strget2int_space(const char* str, int* out);
ZION_API const char* strget2float_space(const char* str, float* out);

ZION_API char* strput_space(char* str, size_t len, const char* in);
ZION_API char* strput4int_space(char* str, size_t len, int value);
ZION_API char* strput4float_space(char* str, size_t len, float value);

#endif
