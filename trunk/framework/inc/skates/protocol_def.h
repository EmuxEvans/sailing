#ifndef __PROTOCOL_DEFINE_INCLUDE__
#define __PROTOCOL_DEFINE_INCLUDE__

#ifdef __cplusplus
extern "C" {
#endif

#define PROTOCOL_NAME(name) __protocol_0_0_1_##name
#define PROTOCOL_ARRAY_SIZE(name)	__array_size_##name

#define PROTOCOL_TYPE_ARRAY			(0x0100)
#define PROTOCOL_TYPE_BASE			0x0001
#define PROTOCOL_TYPE_OBJECT		0x0002
#define PROTOCOL_TYPE_BASE_ARRAY	(PROTOCOL_TYPE_BASE|PROTOCOL_TYPE_ARRAY)
#define PROTOCOL_TYPE_OBJECT_ARRAY	(PROTOCOL_TYPE_OBJECT|PROTOCOL_TYPE_ARRAY)

struct PROTOCOL_VARIABLE;
struct PROTOCOL_TYPE;
struct PROTOCOL_TABLE;
typedef struct PROTOCOL_VARIABLE	PROTOCOL_VARIABLE;
typedef struct PROTOCOL_TYPE		PROTOCOL_TYPE;
typedef struct PROTOCOL_TABLE		PROTOCOL_TABLE;

struct PROTOCOL_VARIABLE {
	char*					name;
	char*					type;
	char*					maxlen;
	char*					def_value;
	PROTOCOL_TYPE*			obj_type;
};

struct PROTOCOL_TYPE {
	char*					name;
	PROTOCOL_VARIABLE*		var_list;
	int						var_count;
};

#ifdef __cplusplus
}
#endif

#endif
