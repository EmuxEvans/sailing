#include "../../inc/skates/os.h"
#include "../../inc/skates/errcode.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/hashmap.h"
#include "../../inc/skates/mempool.h"
#include "../../inc/skates/dbapi.h"
#include "dbapi_provider.h"


typedef struct DBAPI_SAMPLE {
	DBAPI_CONNECTION	con;
	DBAPI_PARAMETER		param[5];
	int					inuse;

}DBAPI_SAMPLE;

static int dbapi_sample_init();
static int dbapi_sample_final();
static DBAPI_HANDLE dbapi_sample_connect(DBAPI_PARAMETER* param, int count);
static int dbapi_sample_disconnect(DBAPI_HANDLE handle);
static int dbapi_sample_release(DBAPI_HANDLE handle);
static int dbapi_sample_begin(DBAPI_HANDLE handle);
static int dbapi_sample_commit(DBAPI_HANDLE handle);
static int dbapi_sample_rollback(DBAPI_HANDLE handle);
static int dbapi_sample_execute(DBAPI_HANDLE handle,const char *sql);
static int dbapi_sample_query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET *rs, int row_max);
static int dbapi_sample_get_errcode(DBAPI_HANDLE handle);
static const char* dbapi_sample_get_errmsg(DBAPI_HANDLE handle);
extern DBAPI_PROVIDER dbapi_sample_provider =
{
	0x00000001,
	"sample",
	dbapi_sample_init,
	dbapi_sample_final,
	dbapi_sample_connect,
	dbapi_sample_disconnect,
	dbapi_sample_release,
	dbapi_sample_begin,
	dbapi_sample_commit,
	dbapi_sample_rollback,
	dbapi_sample_execute,
	dbapi_sample_query,
	dbapi_sample_get_errcode,
	dbapi_sample_get_errmsg
};

static DBAPI_SAMPLE sample_list[100];
static os_mutex_t sample_mutex;

static DBAPI_SAMPLE* sample_alloc()
{
	int ret;
	os_mutex_lock(&sample_mutex);
	for(ret=0; ret<sizeof(sample_list)/sizeof(sample_list[0]); ret++)
		if(!sample_list[ret].inuse) { sample_list[ret].inuse = 1; break; }
	os_mutex_unlock(&sample_mutex);
	if(ret==sizeof(sample_list)/sizeof(sample_list[0])) return NULL;
	return &sample_list[ret];
}

static void sample_free(DBAPI_SAMPLE* conn)
{
	os_mutex_lock(&sample_mutex);
	conn->inuse = 0;
	os_mutex_unlock(&sample_mutex);
}

int dbapi_sample_init()
{
	memset(sample_list, 0, sizeof(sample_list));
	os_mutex_init(&sample_mutex);
	return ERR_NOERROR;
}

int dbapi_sample_final()
{
	os_mutex_destroy(&sample_mutex);
	return ERR_NOERROR;
}

DBAPI_HANDLE dbapi_sample_connect(DBAPI_PARAMETER* param, int count)
{
	DBAPI_SAMPLE* conn;

	conn = sample_alloc();
	if(conn==NULL) return NULL;

	conn->con.dbi = &dbapi_sample_provider;
	return (DBAPI_HANDLE) &(conn->con); 
}

int dbapi_sample_disconnect(DBAPI_HANDLE handle) 
{
	DBAPI_SAMPLE* conn = (DBAPI_SAMPLE*)handle;

	sample_free(conn);
	return ERR_NOERROR;
}

int dbapi_sample_release(DBAPI_HANDLE handle)
{
	DBAPI_SAMPLE* conn = (DBAPI_SAMPLE*)handle;

	return ERR_NOERROR;
}

int dbapi_sample_begin(DBAPI_HANDLE handle)
{
	DBAPI_SAMPLE* conn = (DBAPI_SAMPLE*)handle;

	return ERR_NOERROR;
}

int dbapi_sample_commit(DBAPI_HANDLE handle)
{
	DBAPI_SAMPLE* conn = (DBAPI_SAMPLE*)handle;

	return ERR_NOERROR;
}

int dbapi_sample_rollback(DBAPI_HANDLE handle)
{
	DBAPI_SAMPLE* conn = (DBAPI_SAMPLE*)handle;

	return ERR_NOERROR;
}

int dbapi_sample_execute(DBAPI_HANDLE handle,const char *sql)
{
	DBAPI_SAMPLE* conn = (DBAPI_SAMPLE*)handle;

	return ERR_NOERROR;
}

int dbapi_sample_query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET *rs, int row_max)
{
	int ret;
	DBAPI_SAMPLE* conn = (DBAPI_SAMPLE*)handle;

	dbapi_recordset_set(rs, 1, 2); // row_max, col_count

	ret = dbapi_recordset_put(rs, 0, 0, "value", 6);
	if(ret!=ERR_NOERROR) return ret;
	ret = dbapi_recordset_put(rs, 0, 1, "string", 0);
	if(ret!=ERR_NOERROR) return ret;

	return ERR_NOERROR;
}

int dbapi_sample_get_errcode(DBAPI_HANDLE handle)
{
	DBAPI_SAMPLE* conn = (DBAPI_SAMPLE*)handle;
	return 0;
}

const char* dbapi_sample_get_errmsg(DBAPI_HANDLE handle)
{
	DBAPI_SAMPLE* conn = (DBAPI_SAMPLE*)handle;
	return 0;
}

