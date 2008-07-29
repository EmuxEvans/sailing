#ifndef __DBAPI_INCLUDE_
#define __DBAPI_INCLUDE_

struct DBAPI_CONNECTION;
typedef struct DBAPI_CONNECTION DBAPI_CONNECTION;
typedef DBAPI_CONNECTION* DBAPI_HANDLE;

struct DBAPI_RECORDSET;
typedef struct DBAPI_RECORDSET DBAPI_RECORDSET;

ZION_API int dbapi_init(const char* libpath, int rs_maxsize);
ZION_API int dbapi_final();

ZION_API DBAPI_HANDLE dbapi_connect(const char* connstr);
ZION_API int dbapi_release(DBAPI_HANDLE handle);

ZION_API int dbapi_begin(DBAPI_HANDLE handle);
ZION_API int dbapi_commit(DBAPI_HANDLE handle);
ZION_API int dbapi_rollback(DBAPI_HANDLE handle);

ZION_API int dbapi_execute(DBAPI_HANDLE handle, const char* sql);
ZION_API int dbapi_query(DBAPI_HANDLE handle, const char* sql, DBAPI_RECORDSET **result_set, int row_max);

ZION_API int dbapi_get_errcode(DBAPI_HANDLE handle);
ZION_API const char* dbapi_get_errmsg(DBAPI_HANDLE handle);

ZION_API int dbapi_recordset_free(DBAPI_RECORDSET* recordset);

ZION_API int dbapi_recordset_row_count(DBAPI_RECORDSET* recordset);
ZION_API int dbapi_recordset_col_count(DBAPI_RECORDSET* recordset);
ZION_API const char* dbapi_recordset_get(DBAPI_RECORDSET* recordset, int row, int col);

#endif

