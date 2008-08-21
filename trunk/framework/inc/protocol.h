#ifndef __PROTOCOL_INCLUDE_
#define __PROTOCOL_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

struct PROTOCOL_PARSER;
typedef struct PROTOCOL_PARSER PROTOCOL_PARSER;

typedef struct PROTOCOL_CALLBACK {
	int (*new_node_begin)(void* ptr, const char* name);
	int (*new_node_end)(void* ptr);
	int (*new_const)(void* ptr, const char* type, const char* name, const char* value);
	int (*new_field_def)(void* ptr, const char* mode, const char* type, const char* name, const char* value);
	int (*new_array_def)(void* ptr, const char* mode, const char* type, const char* name, const char* count);
	int (*new_field)(void* ptr, const char* name, const char* value);
	int (*new_array)(void* ptr, const char* name);
	int (*new_begin)(void* ptr);
	int (*new_item)(void* ptr, const char* value);
	int (*new_end)(void* ptr);

	char *mode, *type, *name, *value;
	int mode_len, type_len, name_len, value_len;
} PROTOCOL_CALLBACK;

ZION_API int protocol_parse(const char* buf, PROTOCOL_CALLBACK* callback, void* ptr);
ZION_API void protocol_break(PROTOCOL_PARSER* p);

#ifdef __cplusplus
}
#endif

#endif
