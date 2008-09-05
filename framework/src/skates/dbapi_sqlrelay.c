#include "../../inc/skates/os.h"
#include "../../inc/skates/errcode.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/hashmap.h"
#include "../../inc/skates/mempool.h"
#include "../../inc/skates/dbapi.h"
#include "dbapi_provider.h"


typedef struct DBAPI_SQLRELAY {
	DBAPI_CONNECTION	con;
	DBAPI_PARAMETER		param[5];
	int					inuse;

}DBAPI_SQLRELAY;

static int dbapi_sqlrelay_init();
static int dbapi_sqlrelay_final();
static DBAPI_HANDLE dbapi_sqlrelay_connect(DBAPI_PARAMETER* param, int count);
static int dbapi_sqlrelay_disconnect(DBAPI_HANDLE handle);
static int dbapi_sqlrelay_release(DBAPI_HANDLE handle);
static int dbapi_sqlrelay_begin(DBAPI_HANDLE handle);
static int dbapi_sqlrelay_commit(DBAPI_HANDLE handle);
static int dbapi_sqlrelay_rollback(DBAPI_HANDLE handle);
static int dbapi_sqlrelay_execute(DBAPI_HANDLE handle,const char *sql);
static int dbapi_sqlrelay_query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET *rs, int row_max);
static int dbapi_sqlrelay_get_errcode(DBAPI_HANDLE handle);
static const char* dbapi_sqlrelay_get_errmsg(DBAPI_HANDLE handle);
extern DBAPI_PROVIDER dbapi_sqlrelay_provider =
{
	0x00000001,
	"sqlrelay",
	dbapi_sqlrelay_init,
	dbapi_sqlrelay_final,
	dbapi_sqlrelay_connect,
	dbapi_sqlrelay_disconnect,
	dbapi_sqlrelay_release,
	dbapi_sqlrelay_begin,
	dbapi_sqlrelay_commit,
	dbapi_sqlrelay_rollback,
	dbapi_sqlrelay_execute,
	dbapi_sqlrelay_query,
	dbapi_sqlrelay_get_errcode,
	dbapi_sqlrelay_get_errmsg
};

static DBAPI_SQLRELAY sqlrelay_list[100];
static os_mutex_t sqlrelay_mutex;

static DBAPI_SQLRELAY* sqlrelay_alloc()
{
	int ret;
	os_mutex_lock(&sqlrelay_mutex);
	for(ret=0; ret<sizeof(sqlrelay_list)/sizeof(sqlrelay_list[0]); ret++)
		if(!sqlrelay_list[ret].inuse) { sqlrelay_list[ret].inuse = 1; break; }
	os_mutex_unlock(&sqlrelay_mutex);
	if(ret==sizeof(sqlrelay_list)/sizeof(sqlrelay_list[0])) return NULL;
	return &sqlrelay_list[ret];
}

static void sqlrelay_free(DBAPI_SQLRELAY* conn)
{
	os_mutex_lock(&sqlrelay_mutex);
	conn->inuse = 0;
	os_mutex_unlock(&sqlrelay_mutex);
}

int dbapi_sqlrelay_init()
{
	memset(sqlrelay_list, 0, sizeof(sqlrelay_list));
	os_mutex_init(&sqlrelay_mutex);
	return ERR_NOERROR;
}

int dbapi_sqlrelay_final()
{
	os_mutex_destroy(&sqlrelay_mutex);
	return ERR_NOERROR;
}

DBAPI_HANDLE dbapi_sqlrelay_connect(DBAPI_PARAMETER* param, int count);
{
	int ret;
	DBAPI_SQLRELAY* conn;

	conn = sqlrelay_alloc();
	if(conn==NULL) return NULL;

	conn->con.dbi = &dbapi_sqlrelay_provider;
	return (DBAPI_HANDLE) &(conn->con); 
}

int dbapi_sqlrelay_disconnect(DBAPI_HANDLE handle) 
{
	DBAPI_SQLRELAY* conn = (DBAPI_SQLRELAY*)handle;

	sqlrelay_free(conn);
	return ERR_NOERROR;
}

int dbapi_sqlrelay_release(DBAPI_HANDLE handle)
{
	DBAPI_SQLRELAY* conn = (DBAPI_SQLRELAY*)handle;

	return ERR_NOERROR;
}

int dbapi_sqlrelay_begin(DBAPI_HANDLE handle)
{
	DBAPI_SQLRELAY* conn = (DBAPI_SQLRELAY*)handle;

	return ERR_NOERROR;
}

int dbapi_sqlrelay_commit(DBAPI_HANDLE handle)
{
	DBAPI_SQLRELAY* conn = (DBAPI_SQLRELAY*)handle;

	return ERR_NOERROR;
}

int dbapi_sqlrelay_rollback(DBAPI_HANDLE handle)
{
	DBAPI_SQLRELAY* conn = (DBAPI_SQLRELAY*)handle;

	return ERR_NOERROR;
}

int dbapi_sqlrelay_execute(DBAPI_HANDLE handle,const char *sql)
{
	DBAPI_SQLRELAY* conn = (DBAPI_SQLRELAY*)handle;

	return ERR_NOERROR;
}

int dbapi_sqlrelay_query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET *rs, int row_max)
{
	int ret;
	DBAPI_SQLRELAY* conn = (DBAPI_SQLRELAY*)handle;

	dbapi_recordset_set(rs, 1, 2); // row_max, col_count

	ret = dbapi_recordset_put(rs, 0, 0, "value", 6);
	if(ret!=ERR_NOERROR) return ret;
	ret = dbapi_recordset_put(rs, 0, 1, "string", 0);
	if(ret!=ERR_NOERROR) return ret;

	return ERR_NOERROR;
}

int dbapi_sqlrelay_get_errcode(DBAPI_HANDLE handle)
{
	DBAPI_SQLRELAY* conn = (DBAPI_SQLRELAY*)handle;
	return 0;
}

const char* dbapi_sqlrelay_get_errmsg(DBAPI_HANDLE handle)
{
	DBAPI_SQLRELAY* conn = (DBAPI_SQLRELAY*)handle;
	return 0;
}
