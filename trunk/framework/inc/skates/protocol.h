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


//
struct PROTOCOL_VARIABLE;
struct PROTOCOL_TYPE;
struct PROTOCOL_TABLE;
typedef struct PROTOCOL_VARIABLE	PROTOCOL_VARIABLE;
typedef struct PROTOCOL_TYPE		PROTOCOL_TYPE;
typedef struct PROTOCOL_TABLE		PROTOCOL_TABLE;

ZION_API PROTOCOL_TABLE* protocol_table_alloc(void* buf, unsigned int buf_len, int type_max, int var_max);
ZION_API void protocol_table_free(PROTOCOL_TABLE* table);
ZION_API void protocol_table_clear(PROTOCOL_TABLE* table);

ZION_API int protocol_parse_pfile(const char* text, PROTOCOL_TABLE* table);
ZION_API int protocol_generate_cfile(const PROTOCOL_TABLE* table, const char* name, char* inc, unsigned int inc_len, char* src, unsigned int src_len);

ZION_API int protocol_binary_read(PROTOCOL_TABLE* table, const char* name, const void* data, unsigned int data_len, void* buf);
ZION_API int protocol_binary_write(PROTOCOL_TABLE* table, const char* name, const void* buf, void* data, unsigned int* data_len);

ZION_API int protocol_text_read(PROTOCOL_TABLE* table, const char* name, const char* data, void* buf);
ZION_API int protocol_text_write(PROTOCOL_TABLE* table, const char* name, const void* buf, char* data, unsigned int data_len);

ZION_API int protocol_file_read(PROTOCOL_TABLE* table, const char* name, const char* filename, void* buf);
ZION_API int protocol_file_write(PROTOCOL_TABLE* table, const char* name, const void* buf, const char* filename);

#ifdef __cplusplus
}
#endif

#endif
