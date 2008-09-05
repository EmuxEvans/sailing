#ifndef __DBAPI_PROVIDE_INCLUDE_
#define __DBAPI_PROVIDE_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#define DBAPI_CONNSTR_MAXLEN	170

typedef struct DBAPI_PARAMETER {
	char	name[20];
	char	value[30];
} DBAPI_PARAMETER;

struct DBAPI_RECORDSET {
	int row_max, row_count, col_count;
	char* buf;
	int buf_len;
	char** fields_name;
	char** rows;
};

typedef struct DBAPI_GROUP {
	HASHMAP_ITEM				item;
	char						connstr[DBAPI_CONNSTR_MAXLEN];
	RLIST_HEAD					free_list;
	RLIST_HEAD					conn_list;
} DBAPI_GROUP;

typedef struct DBAPI_PROVIDER {
	unsigned int version;
	const char* name;

	int (*Init)();
	int (*Final)();

	DBAPI_HANDLE (*Connect)(DBAPI_PARAMETER* param, int count);
	int (*Disconnect)(DBAPI_HANDLE handle);
	int (*Release)(DBAPI_HANDLE handle);

	int (*Begin)(DBAPI_HANDLE handle);
	int (*Commit)(DBAPI_HANDLE handle);
	int (*Rollback)(DBAPI_HANDLE handle);

	int (*Execute)(DBAPI_HANDLE handle, const char* sql);
	int (*Query)(DBAPI_HANDLE handle, const char* sql, DBAPI_RECORDSET* rs, int row_max);

	int (*GetErrorCode)(DBAPI_HANDLE handle);
	const char* (*GetErrorMsg)(DBAPI_HANDLE handle);
} DBAPI_PROVIDER;

struct DBAPI_CONNECTION {
	DBAPI_PROVIDER*	dbi;
	DBAPI_GROUP*				group;
	RLIST_ITEM					conn_item;
	RLIST_ITEM					free_item;

	int							in_transaction;
};

ZION_API int dbapi_register_provider(DBAPI_PROVIDER* p);
ZION_API int dbapi_unregister_provider(DBAPI_PROVIDER* p);

ZION_API int dbapi_crack_connstr(const char* connstr, DBAPI_PARAMETER* list, int count);
ZION_API const char* dbapi_parameter_get(const DBAPI_PARAMETER* list, int count, const char* name);

ZION_API int dbapi_recordset_set(DBAPI_RECORDSET* rs, int row_max, int col_count);

ZION_API int dbapi_recordset_set_fieldname(DBAPI_RECORDSET* rs, int col, const char* name);

ZION_API int dbapi_recordset_put(DBAPI_RECORDSET* recordset, int row, int col, const char* value, int len);
ZION_API char* dbapi_recordset_getbuf(DBAPI_RECORDSET* recordset);
ZION_API int dbapi_recordset_getbuflen(DBAPI_RECORDSET* recordset);

#ifdef __cplusplus
}
#endif

#endif

