#ifndef __PROTOCOL_DEFINE_INCLUDE__
#define __PROTOCOL_DEFINE_INCLUDE__

#ifdef __cplusplus
extern "C" {
#endif

#define PROTOCOL_NAME(name) __protocol_0_0_1_##name
#define PROTOCOL_ARRAY_SIZE(name)	__array_size_##name

#define PROTOCOL_STRUCT_OFFSET(type, var)			((unsigned int)&(((type*)NULL)->var))

#define PROTOCOL_TYPE_ARRAY			0x0100
#define PROTOCOL_TYPE_OBJECT		0x00ff
#define PROTOCOL_TYPE_CHAR			0x0001
#define PROTOCOL_TYPE_SHORT			0x0002
#define PROTOCOL_TYPE_INT			0x0003
#define PROTOCOL_TYPE_LONG			0x0004
#define PROTOCOL_TYPE_BYTE			0x0005
#define PROTOCOL_TYPE_WORD			0x0006
#define PROTOCOL_TYPE_DWORD			0x0007
#define PROTOCOL_TYPE_QWORD			0x0008
#define PROTOCOL_TYPE_FLOAT			0x0009

struct PROTOCOL_VARIABLE;
struct PROTOCOL_TYPE;
struct PROTOCOL_TABLE;
typedef struct PROTOCOL_VARIABLE	PROTOCOL_VARIABLE;
typedef struct PROTOCOL_TYPE		PROTOCOL_TYPE;
typedef struct PROTOCOL_TABLE		PROTOCOL_TABLE;

struct PROTOCOL_VARIABLE {
	char*					name;
	int						type;
	PROTOCOL_TYPE*			obj_type;
	unsigned int			maxlen;
	unsigned int			offset;
};

struct PROTOCOL_TYPE {
	char*					name;
	PROTOCOL_VARIABLE*		var_list;
	int						var_count;
	unsigned int			size;
};

#ifdef __cplusplus
}
#endif

#endif
