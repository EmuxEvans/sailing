#ifndef __PROTOCOL_INCLUDE_
#define __PROTOCOL_INCLUDE_

#include "protocol_def.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PROTOCOL_CALLBACK;
typedef struct PROTOCOL_CALLBACK PROTOCOL_CALLBACK;

struct PROTOCOL_CALLBACK {
	void (*new_field)(PROTOCOL_CALLBACK* callback, const char* name, const char* value);
	void (*new_array)(PROTOCOL_CALLBACK* callback, const char* name);
	void (*new_begin)(PROTOCOL_CALLBACK* callback);
	void (*new_item)(PROTOCOL_CALLBACK* callback, const char* value);
	void (*new_end)(PROTOCOL_CALLBACK* callback);

	char *type, *name, *value;
	int type_len, name_len, value_len;

	int is_break;
	void* user_ptr;
};

ZION_API int protocol_parse(const char* buf, PROTOCOL_CALLBACK* callback, void* ptr);
ZION_API void protocol_break(PROTOCOL_CALLBACK* callback);

ZION_API int protocol_convert(const char* value, void* buf, int type, unsigned int len);

ZION_API int protocol_binary_read(PROTOCOL_TYPE* type, const void* data, unsigned int* data_len, void* buf);
ZION_API int protocol_binary_write(PROTOCOL_TYPE* type, const void* buf, void* data, unsigned int* data_len);

ZION_API int protocol_text_read(PROTOCOL_TYPE* type, const char* data, unsigned int* data_len, void* buf);
ZION_API int protocol_text_write(PROTOCOL_TYPE* type, const void* buf, char* data, unsigned int* data_len);

ZION_API int protocol_file_read(PROTOCOL_TYPE* type, const char* filename, void* buf);
ZION_API int protocol_file_write(PROTOCOL_TYPE* type, const void* buf, const char* filename);

#ifdef __cplusplus
}
#endif

#endif
