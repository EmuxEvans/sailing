#ifndef __PROTOCOL_INCLUDE_
#define __PROTOCOL_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PROTOCOL_CALLBACK {
	int (*new_node_begin)(void* ptr, const char* name);
	int (*new_node_end)(void* ptr);
	int (*new_field)(void* ptr, const char* name, const char* value);
	int (*new_array)(void* ptr, const char* name);
	int (*new_begin)(void* ptr);
	int (*new_item)(void* ptr, const char* value);
	int (*new_end)(void* ptr);

	char *type, *name, *value;
	int type_len, name_len, value_len;
} PROTOCOL_CALLBACK;

ZION_API int protocol_parse(const char* buf, PROTOCOL_CALLBACK* callback, void* ptr);

//
#include "protocol_def.h"

ZION_API int protocol_parse_pfile(const char* text, PROTOCOL_TABLE* table);
ZION_API int protocol_generate_cfile(const PROTOCOL_TABLE* table, const char* name, char* inc, unsigned int inc_len, char* src, unsigned int src_len);

ZION_API int protocol_binary_read(PROTOCOL_TYPE* type, const char* name, const void* data, unsigned int data_len, void* buf);
ZION_API int protocol_binary_write(PROTOCOL_TYPE* type, const char* name, const void* buf, void* data, unsigned int* data_len);

ZION_API int protocol_text_read(PROTOCOL_TYPE* type, const char* name, const char* data, void* buf);
ZION_API int protocol_text_write(PROTOCOL_TYPE* type, const char* name, const void* buf, char* data, unsigned int data_len);

ZION_API int protocol_file_read(PROTOCOL_TYPE* type, const char* name, const char* filename, void* buf);
ZION_API int protocol_file_write(PROTOCOL_TYPE* type, const char* name, const void* buf, const char* filename);

#ifdef __cplusplus
}
#endif

#endif
