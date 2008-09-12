#include <string.h>
#include <stdlib.h>

#include "../../inc/skates/os.h"
#include "../../inc/skates/errcode.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/hashmap.h"
#include "../../inc/skates/mempool.h"
#include "../../inc/skates/dbapi.h"
#include "dbapi_provider.h"

#include <mysql/mysql.h>

typedef struct DBAPI_MYSQL {
	DBAPI_CONNECTION	con;
	DBAPI_PARAMETER		param[5];
	int					inuse;

	MYSQL				mysql;
	void*				sock;
} DBAPI_MYSQL;

static int dbapi_mysql_init();
static int dbapi_mysql_final();
static DBAPI_HANDLE dbapi_mysql_connect(DBAPI_PARAMETER* param, int count);
static int dbapi_mysql_disconnect(DBAPI_HANDLE handle);
static int dbapi_mysql_release(DBAPI_HANDLE handle);
static int dbapi_mysql_begin(DBAPI_HANDLE handle);
static int dbapi_mysql_commit(DBAPI_HANDLE handle);
static int dbapi_mysql_rollback(DBAPI_HANDLE handle);
static int dbapi_mysql_execute(DBAPI_HANDLE handle,const char *sql);
static int dbapi_mysql_query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET *rs, int row_max);
static int dbapi_mysql_get_errcode(DBAPI_HANDLE handle);
static const char* dbapi_mysql_get_errmsg(DBAPI_HANDLE handle);
DBAPI_PROVIDER dbapi_mysql_provider =
{
	0x00000001,
	"mysql",
	dbapi_mysql_init,
	dbapi_mysql_final,
	dbapi_mysql_connect,
	dbapi_mysql_disconnect,
	dbapi_mysql_release,
	dbapi_mysql_begin,
	dbapi_mysql_commit,
	dbapi_mysql_rollback,
	dbapi_mysql_execute,
	dbapi_mysql_query,
	dbapi_mysql_get_errcode,
	dbapi_mysql_get_errmsg
};

static DBAPI_MYSQL mysql_list[100];
static os_mutex_t mysql_mutex;

static DBAPI_MYSQL* mysql_alloc()
{
	int ret;
	os_mutex_lock(&mysql_mutex);
	for(ret=0; ret<sizeof(mysql_list)/sizeof(mysql_list[0]); ret++)
		if(!mysql_list[ret].inuse) { mysql_list[ret].inuse = 1; break; }
	os_mutex_unlock(&mysql_mutex);
	if(ret==sizeof(mysql_list)/sizeof(mysql_list[0])) return NULL;
	return &mysql_list[ret];
}

static void mysql_free(DBAPI_MYSQL* conn)
{
	os_mutex_lock(&mysql_mutex);
	conn->inuse = 0;
	os_mutex_unlock(&mysql_mutex);
}

int dbapi_mysql_init()
{
	memset(mysql_list, 0, sizeof(mysql_list));
	os_mutex_init(&mysql_mutex);

	my_init();

	return ERR_NOERROR;
}

int dbapi_mysql_final()
{
	os_mutex_destroy(&mysql_mutex);

	return ERR_NOERROR;
}

DBAPI_HANDLE dbapi_mysql_connect(DBAPI_PARAMETER* param, int count)
{
	DBAPI_MYSQL* conn;
	const char* host		= dbapi_parameter_get(param, count, "host");
	const char* user		= dbapi_parameter_get(param, count, "user");
	const char* passwd		= dbapi_parameter_get(param, count, "password");
	const char* db			= dbapi_parameter_get(param, count, "database");
	const char* port		= dbapi_parameter_get(param, count, "port");
	const char* unix_socket	= dbapi_parameter_get(param, count, "unix_sock");

	conn = mysql_alloc();
	if(conn==NULL) return NULL;

	mysql_init(&conn->mysql);
	conn->sock = mysql_real_connect(&conn->mysql, host, user, passwd, db, port?atoi(port):0, unix_socket, CLIENT_FOUND_ROWS);
	if(conn->sock==NULL) {
		mysql_free(conn);
		return NULL;
	}

	conn->con.dbi = &dbapi_mysql_provider;
	return (DBAPI_HANDLE) &(conn->con); 
}

int dbapi_mysql_disconnect(DBAPI_HANDLE handle) 
{
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;

	mysql_close(conn->sock);

	mysql_free(conn);
	return ERR_NOERROR;
}

int dbapi_mysql_release(DBAPI_HANDLE handle)
{
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
	(void)conn;
	return ERR_NOERROR;
}

int dbapi_mysql_begin(DBAPI_HANDLE handle)
{
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
    if(!mysql_query(conn->sock, "begin")) return ERR_UNKNOWN;
	return ERR_NOERROR;
}

int dbapi_mysql_commit(DBAPI_HANDLE handle)
{
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
    if(!mysql_query(conn->sock, "commit")) return ERR_UNKNOWN;
	return ERR_NOERROR;
}

int dbapi_mysql_rollback(DBAPI_HANDLE handle)
{
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
    if(!mysql_query(conn->sock, "rollback")) return ERR_UNKNOWN;
	return ERR_NOERROR;
}

int dbapi_mysql_execute(DBAPI_HANDLE handle,const char *sql)
{
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
    if(!mysql_query(conn->sock, sql)) return ERR_UNKNOWN;
    if(mysql_affected_rows(conn->sock)==0) return ERR_NOT_FOUND;
	return ERR_NOERROR;
}

int dbapi_mysql_query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET *rs, int row_max)
{
	int ret;
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
    MYSQL_RES *res;
    MYSQL_ROW row;
	int row_idx, col_count, col_idx;

    //if(mysql_query(conn->sock, sql)!=0)
    ret = mysql_query(conn->sock, sql);
	if(ret!=0)
		return ERR_UNKNOWN;

	res = mysql_use_result(conn->sock);
	if(res==NULL)
		return ERR_UNKNOWN;

	col_count = mysql_field_count(conn->sock);
	ret = dbapi_recordset_set(rs, row_max, col_count);
    if(ret!=ERR_NOERROR) {
		mysql_free_result(res);
        return ret;
	}

	for(col_idx=0; col_idx<col_count; col_idx++) {
		MYSQL_FIELD* field;

		field = mysql_fetch_field_direct(res, col_idx);
		if(field==NULL)
			continue;

		ret = dbapi_recordset_set_fieldname(rs, col_idx, field->name);
        if(ret!=ERR_NOERROR) {
			mysql_free_result(res);
            return ERR_NO_ENOUGH_MEMORY;
        }
	}

	for(row_idx=0; row_idx<row_max; row_idx++) {
		row = mysql_fetch_row(res);
		if(row==NULL) {
			if(mysql_errno(conn->sock)) return ERR_UNKNOWN;
			break;
		}

		for(col_idx=0; col_idx<col_count; col_idx++) {
			if(row[col_idx]) {
				ret = dbapi_recordset_put(rs, row_idx, col_idx, row[col_idx], 0);
				if(ret!=ERR_NOERROR) {
					mysql_free_result(res);
					return ERR_NO_ENOUGH_MEMORY;
				}
			}
		}
	}

	mysql_free_result(res);

	if(row_idx==0) return ERR_NOT_FOUND;

	return ERR_NOERROR;
}

int dbapi_mysql_get_errcode(DBAPI_HANDLE handle)
{
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
	return mysql_errno(conn->sock);
}

const char* dbapi_mysql_get_errmsg(DBAPI_HANDLE handle)
{
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
	(void)conn;
	return "JUST ERROR";
}
