#include "../../inc/skates/os.h"
#include "../../inc/skates/errcode.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/hashmap.h"
#include "../../inc/skates/mempool.h"
#include "../../inc/skates/dbapi.h"
#include "dbapi_provider.h"

#include <mysql.h>

typedef struct DBAPI_MYSQL {
	DBAPI_CONNECTION	con;
	DBAPI_PARAMETER		param[5];

	MYSQL				mysql;
	void*				sock;
}DBAPI_MYSQL;

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
extern DBAPI_PROVIDER dbapi_mysql_provider =
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
	ossl_clean_mt();

	os_mutex_destroy(&mysql_mutex);

	return ERR_NOERROR;
}

DBAPI_HANDLE dbapi_mysql_connect(DBAPI_PARAMETER* param, int count);
{
	int ret;
	DBAPI_MYSQL* conn;
	const char* host		= dbapi_parameter_get(conn->param, sizeof(conn->param)/sizeof(conn->param[0]), "host");
	const char* user		= dbapi_parameter_get(conn->param, sizeof(conn->param)/sizeof(conn->param[0]), "user");
	const char* passwd		= dbapi_parameter_get(conn->param, sizeof(conn->param)/sizeof(conn->param[0]), "password");
	const char* db			= dbapi_parameter_get(conn->param, sizeof(conn->param)/sizeof(conn->param[0]), "database");
	const char* port		= dbapi_parameter_get(conn->param, sizeof(conn->param)/sizeof(conn->param[0]), "port");
	const char* unix_socket	= dbapi_parameter_get(conn->param, sizeof(conn->param)/sizeof(conn->param[0]), "unix_sock");

	conn = mysql_alloc();
	if(conn==NULL) return NULL;

	ret = dbapi_crack_connstr(connstr, conn->param, sizeof(conn->param)/sizeof(conn->param[0]));
	if(ret!=ERR_NOERROR) { mysql_free(conn); return NULL; }


	mysql_init(&conn->mysql);
	conn->sock = mysql_real_connect(&conn->mysql, host, user, password, db, port?atoi(port):0, unix_socket, CLIENT_FOUND_ROWS);
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

	return ERR_NOERROR;
}

int dbapi_mysql_begin(DBAPI_HANDLE handle)
{
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
    if(mysql_query(conn->sock, "begin")) return ERR_UNKNOWN;
	return ERR_NOERROR;
}

int dbapi_mysql_commit(DBAPI_HANDLE handle)
{
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
    if(mysql_query(conn->sock, "commit")) return ERR_UNKNOWN;
	return ERR_NOERROR;
}

int dbapi_mysql_rollback(DBAPI_HANDLE handle)
{
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
    if(mysql_query(conn->sock, "rollback")) return ERR_UNKNOWN;
	return ERR_NOERROR;
}

int dbapi_mysql_execute(DBAPI_HANDLE handle,const char *sql)
{
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
    if(mysql_query(conn->sock, sql)) return ERR_UNKNOWN;
    if(mysql_affected_rows(db->sock)==0) return ERR_NOT_FOUND;
	return ERR_NOERROR;
}

int dbapi_mysql_query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET *rs, int row_max)
{
	int ret;
	DBAPI_MYSQL* conn = (DBAPI_MYSQL*)handle;
    MYSQL_RES *res;
    MYSQL_ROW row;
	int row_idx, col_count, col_idx;

    if(mysql_query(conn->sock, sql)) return ERR_UNKNOWN;
	res = mysql_use_result(db->sock);
	if(res==NULL) return ERR_UNKNOWN;

	col_count = mysql_field_count(db->sock);
	row_idx = col_idx = 0;

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
	return "JUST ERROR";
}
