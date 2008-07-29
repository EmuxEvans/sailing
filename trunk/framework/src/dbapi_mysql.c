#include <assert.h>

#include <winsock2.h>
#include <mysql.h>

#include "../inc/os.h"
#include "../inc/errcode.h"
#include "../inc/rlist.h"
#include "../inc/hashmap.h"
#include "../inc/mempool.h"
#include "../inc/dbapi.h"
#include "dbapi_provider.h"

typedef struct DBAPI_MYSQL {
	DBAPI_CONNECTION	con;
	MYSQL*				mysql;
	DBAPI_PARAMETER		param[5];
	int					errcode;
} DBAPI_MYSQL;

static int MySQL_Init(void);
static int MySQL_Final(void);
static DBAPI_HANDLE MySQL_Connect(const char *connstr);
static int MySQL_Disconnect(DBAPI_HANDLE handle);
static int MySQL_Release(DBAPI_HANDLE handle);
static int MySQL_Begin(DBAPI_HANDLE handle);
static int MySQL_Commit(DBAPI_HANDLE handle);
static int MySQL_Rollback(DBAPI_HANDLE handle);
static int MySQL_Execute(DBAPI_HANDLE handle,const char *sql);
static int MySQL_Query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET **result_set, int row_max);
static int MySQL_GetErrorCode(DBAPI_HANDLE handle);
static const char * MySQL_GetErrorMsg(DBAPI_HANDLE handle);

static MEMPOOL_HANDLE  conn_pool = NULL;

DBAPI_PROVIDER dbapi_mysql_provider = {
	1,
	"mysql",
	sizeof(DBAPI_MYSQL),
	MySQL_Init,
	MySQL_Final,
	MySQL_Connect,
	MySQL_Disconnect,
	MySQL_Release,
	MySQL_Begin,
	MySQL_Commit,
	MySQL_Rollback,
	MySQL_Execute,
	MySQL_Query,
	MySQL_GetErrorCode,
	MySQL_GetErrorMsg	
};

int MySQL_Init(void)
{
	if(!mysql_thread_safe()) return ERR_UNKNOWN;
	if(!my_init()) return ERR_UNKNOWN;
	return ERR_NOERROR;
}

int MySQL_Final(void)
{
	return ERR_NOERROR;
}

DBAPI_HANDLE MySQL_Connect(const char *connstr)
{
	return NULL;
}

int MySQL_Disconnect(DBAPI_HANDLE handle)
{
	return ERR_NOERROR;
}

int MySQL_Release(DBAPI_HANDLE handle)
{
	return ERR_NOERROR;
}

int MySQL_Begin(DBAPI_HANDLE handle)
{
	return ERR_NOERROR;
}

int MySQL_Commit(DBAPI_HANDLE handle)
{
	return ERR_NOERROR;
}

int MySQL_Rollback(DBAPI_HANDLE handle)
{
	return ERR_NOERROR;
}

int MySQL_Execute(DBAPI_HANDLE handle,const char *sql)
{
	return ERR_NOERROR;
}

int MySQL_Query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET **result_set, int row_max)
{
	return ERR_NOERROR;
}

int MySQL_GetErrorCode(DBAPI_HANDLE handle)
{
	return ERR_NOERROR;
}

const char * MySQL_GetErrorMsg(DBAPI_HANDLE handle)
{
	return "";
}

