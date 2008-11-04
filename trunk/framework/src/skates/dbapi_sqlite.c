#include <string.h>

#include "../../inc/skates/os.h"
#include "../../inc/skates/errcode.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/hashmap.h"
#include "../../inc/skates/mempool.h"
#include "../../inc/skates/dbapi.h"
#include "dbapi_provider.h"

#include "sqlite/sqlite3.h"

typedef struct DBAPI_SQLITE {
	DBAPI_CONNECTION	con;
	int					inuse;

    sqlite3*			db;
	int					errcode;
	char				errmsg[100];
}DBAPI_SQLITE;

static int dbapi_sqlite_init();
static int dbapi_sqlite_final();
static DBAPI_HANDLE dbapi_sqlite_connect(DBAPI_PARAMETER* param, int count);
static int dbapi_sqlite_disconnect(DBAPI_HANDLE handle);
static int dbapi_sqlite_release(DBAPI_HANDLE handle);
static int dbapi_sqlite_begin(DBAPI_HANDLE handle);
static int dbapi_sqlite_commit(DBAPI_HANDLE handle);
static int dbapi_sqlite_rollback(DBAPI_HANDLE handle);
static int dbapi_sqlite_execute(DBAPI_HANDLE handle,const char *sql);
static int dbapi_sqlite_query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET *rs, int row_max);
static int dbapi_sqlite_get_errcode(DBAPI_HANDLE handle);
static const char* dbapi_sqlite_get_errmsg(DBAPI_HANDLE handle);
DBAPI_PROVIDER dbapi_sqlite_provider =
{
	0x00000001,
	"sqlite",
	dbapi_sqlite_init,
	dbapi_sqlite_final,
	dbapi_sqlite_connect,
	dbapi_sqlite_disconnect,
	dbapi_sqlite_release,
	dbapi_sqlite_begin,
	dbapi_sqlite_commit,
	dbapi_sqlite_rollback,
	dbapi_sqlite_execute,
	dbapi_sqlite_query,
	dbapi_sqlite_get_errcode,
	dbapi_sqlite_get_errmsg
};

static DBAPI_SQLITE sqlite_list[100];
static os_mutex_t sqlite_mutex;

static int _set_errcode(DBAPI_SQLITE* conn, int errcode);

static DBAPI_SQLITE* sqlite_alloc()
{
	int ret;
	os_mutex_lock(&sqlite_mutex);
	for(ret=0; ret<sizeof(sqlite_list)/sizeof(sqlite_list[0]); ret++)
		if(!sqlite_list[ret].inuse) { sqlite_list[ret].inuse = 1; break; }
	os_mutex_unlock(&sqlite_mutex);
	if(ret==sizeof(sqlite_list)/sizeof(sqlite_list[0])) return NULL;
	return &sqlite_list[ret];
}

static void sqlite_free(DBAPI_SQLITE* conn)
{
	os_mutex_lock(&sqlite_mutex);
	conn->inuse = 0;
	os_mutex_unlock(&sqlite_mutex);
}

int dbapi_sqlite_init()
{
	memset(sqlite_list, 0, sizeof(sqlite_list));
	os_mutex_init(&sqlite_mutex);
	return ERR_NOERROR;
}

int dbapi_sqlite_final()
{
	os_mutex_destroy(&sqlite_mutex);
	return ERR_NOERROR;
}

DBAPI_HANDLE dbapi_sqlite_connect(DBAPI_PARAMETER* param, int count)
{
	int ret;
	DBAPI_SQLITE* conn;
	const char* dbname = dbapi_parameter_get(param, count, "dbname");

	conn = sqlite_alloc();
	if(conn==NULL) return NULL;

	ret = sqlite3_open(dbname, &conn->db);
	if(ret!=SQLITE_OK) { sqlite_free(conn); return NULL; }

	conn->con.dbi = &dbapi_sqlite_provider;
	return (DBAPI_HANDLE) &(conn->con); 
}

int dbapi_sqlite_disconnect(DBAPI_HANDLE handle) 
{
	DBAPI_SQLITE* conn = (DBAPI_SQLITE*)handle;

	sqlite3_close(conn->db);

	sqlite_free(conn);
	return ERR_NOERROR;
}

int dbapi_sqlite_release(DBAPI_HANDLE handle)
{
//	DBAPI_SQLITE* conn = (DBAPI_SQLITE*)handle;

	return ERR_NOERROR;
}

int dbapi_sqlite_begin(DBAPI_HANDLE handle)
{
	int ret;
	DBAPI_SQLITE* conn = (DBAPI_SQLITE*)handle;
	ret = sqlite3_exec(conn->db, "begin transaction", NULL, NULL, NULL);
	return _set_errcode(conn, ret);
}

int dbapi_sqlite_commit(DBAPI_HANDLE handle)
{
	int ret;
	DBAPI_SQLITE* conn = (DBAPI_SQLITE*)handle;
	ret = sqlite3_exec(conn->db, "commit transaction", NULL, NULL, NULL);
	return _set_errcode(conn, ret);
}

int dbapi_sqlite_rollback(DBAPI_HANDLE handle)
{
	int ret;
	DBAPI_SQLITE* conn = (DBAPI_SQLITE*)handle;
	ret = sqlite3_exec(conn->db, "rollback transaction", NULL, NULL, NULL);
	return _set_errcode(conn, ret);
}

int dbapi_sqlite_execute(DBAPI_HANDLE handle,const char *sql)
{
	int ret;
	DBAPI_SQLITE* conn = (DBAPI_SQLITE*)handle;
	ret = sqlite3_exec(conn->db, sql, NULL, NULL, NULL);
	return _set_errcode(conn, ret);
}

int dbapi_sqlite_query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET *rs, int row_max)
{
	int ret;
	DBAPI_SQLITE* conn = (DBAPI_SQLITE*)handle;

    sqlite3_stmt *stmt;
    const char *next;
	int row_idx, col_count, col_idx;

    ret = sqlite3_prepare(conn->db, sql, -1, &stmt, &next);
    if(ret!=SQLITE_OK) return _set_errcode(conn, ret);

	col_count = sqlite3_column_count(stmt);
	ret = dbapi_recordset_set(rs, row_max, col_count);
	if(ret!=ERR_NOERROR)
		return ret;

	for(col_idx=0; col_idx<col_count; col_idx++) {
		ret = dbapi_recordset_set_fieldname(rs, col_idx, sqlite3_column_name(stmt, col_idx));
		if(ret!=ERR_NOERROR) {
			sqlite3_finalize(stmt);
			return ERR_NO_ENOUGH_MEMORY;
		}
	}

	for(row_idx=0; row_idx<row_max; row_idx++) {
		ret = sqlite3_step(stmt);
		if(ret!=SQLITE_ROW) {
            if(ret!=SQLITE_DONE) {
                sqlite3_finalize(stmt);
				return _set_errcode(conn, ret);
            }
            break;
        }

		for(col_idx=0; col_idx<col_count; col_idx++) {
			switch(sqlite3_column_type(stmt, col_idx)) {
            case SQLITE_INTEGER:
            case SQLITE_FLOAT:
            case SQLITE_TEXT:
            case SQLITE_BLOB:
				ret = dbapi_recordset_put(rs, row_idx, col_idx, (char*)sqlite3_column_text(stmt, col_idx), 0);
				if(ret!=ERR_NOERROR) {
					sqlite3_finalize(stmt);
					return ERR_NO_ENOUGH_MEMORY;
				}
                break;
            case SQLITE_NULL:
                break;
            default:
                break;
			}
		}
	}
	sqlite3_finalize(stmt);
	
	if(row_idx==0) return ERR_NOT_FOUND;

	return ERR_NOERROR;
}

int dbapi_sqlite_get_errcode(DBAPI_HANDLE handle)
{
	DBAPI_SQLITE* conn = (DBAPI_SQLITE*)handle;
	return conn->errcode;
}

const char* dbapi_sqlite_get_errmsg(DBAPI_HANDLE handle)
{
	DBAPI_SQLITE* conn = (DBAPI_SQLITE*)handle;
	return conn->errmsg;
}

int _set_errcode(DBAPI_SQLITE* conn, int errcode)
{
	conn->errcode = errcode;
	conn->errmsg[0] = '\0';
	return errcode;
}
