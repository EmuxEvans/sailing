#ifndef __PROTOCOL_INCLUDE_
#define __PROTOCOL_INCLUDE_

struct PROTOCOL_PARSER;
typedef struct PROTOCOL_PARSER PROTOCOL_PARSER;

typedef struct PROTOCOL_CALLBACK {
	void (*new_node)(void* ptr, const char* name);
	void (*new_field_def)(void* ptr, const char* mode, const char* name, const char* type, const char* value);
	void (*new_array_def)(void* ptr, const char* name, const char* type, const char* count);
	void (*new_field)(void* ptr, const char* name, const char* value);
	void (*new_array)(void* ptr, const char* name);
	void (*new_array_start)(void* ptr);
	void (*new_array_item)(void* ptr);
	void (*new_array_end)(void* ptr);
} PROTOCOL_CALLBACK;

ZION_API int protocol_parse(const char* buf, PROTOCOL_CALLBACK* callback, void* ptr);
ZION_API void protocol_break(PROTOCOL_PARSER* p);

#endif
