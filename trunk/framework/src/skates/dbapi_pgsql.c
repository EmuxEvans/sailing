#include "../../inc/skates/os.h"
#include "../../inc/skates/errcode.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/hashmap.h"
#include "../../inc/skates/mempool.h"
#include "../../inc/skates/dbapi.h"
#include "dbapi_provider.h"


typedef struct DBAPI_PGSQL {
	DBAPI_CONNECTION	con;
	DBAPI_PARAMETER		param[5];
	int					inuse;

	PGconn*				pgconn;
	int					errcode;
	char				errmsg[100];
}DBAPI_PGSQL;

static int dbapi_pgsql_init();
static int dbapi_pgsql_final();
static DBAPI_HANDLE dbapi_pgsql_connect(DBAPI_PARAMETER* param, int count);
static int dbapi_pgsql_disconnect(DBAPI_HANDLE handle);
static int dbapi_pgsql_release(DBAPI_HANDLE handle);
static int dbapi_pgsql_begin(DBAPI_HANDLE handle);
static int dbapi_pgsql_commit(DBAPI_HANDLE handle);
static int dbapi_pgsql_rollback(DBAPI_HANDLE handle);
static int dbapi_pgsql_execute(DBAPI_HANDLE handle,const char *sql);
static int dbapi_pgsql_query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET *rs, int row_max);
static int dbapi_pgsql_get_errcode(DBAPI_HANDLE handle);
static const char* dbapi_pgsql_get_errmsg(DBAPI_HANDLE handle);
extern DBAPI_PROVIDER dbapi_pgsql_provider =
{
	0x00000001,
	"pgsql",
	dbapi_pgsql_init,
	dbapi_pgsql_final,
	dbapi_pgsql_connect,
	dbapi_pgsql_disconnect,
	dbapi_pgsql_release,
	dbapi_pgsql_begin,
	dbapi_pgsql_commit,
	dbapi_pgsql_rollback,
	dbapi_pgsql_execute,
	dbapi_pgsql_query,
	dbapi_pgsql_get_errcode,
	dbapi_pgsql_get_errmsg
};

static DBAPI_PGSQL pgsql_list[100];
static os_mutex_t pgsql_mutex;

static int _set_errcode(DBAPI_PGSQL* conn, const char* errcode);

static DBAPI_PGSQL* pgsql_alloc()
{
	int ret;
	os_mutex_lock(&pgsql_mutex);
	for(ret=0; ret<sizeof(pgsql_list)/sizeof(pgsql_list[0]); ret++)
		if(!pgsql_list[ret].inuse) { pgsql_list[ret].inuse = 1; break; }
	os_mutex_unlock(&pgsql_mutex);
	if(ret==sizeof(pgsql_list)/sizeof(pgsql_list[0])) return NULL;
	return &pgsql_list[ret];
}

static void pgsql_free(DBAPI_PGSQL* conn)
{
	os_mutex_lock(&pgsql_mutex);
	conn->inuse = 0;
	os_mutex_unlock(&pgsql_mutex);
}

int dbapi_pgsql_init()
{
	memset(pgsql_list, 0, sizeof(pgsql_list));
	os_mutex_init(&pgsql_mutex);

    ossl_init_mt();

	return ERR_NOERROR;
}

int dbapi_pgsql_final()
{
    ossl_clean_mt();

	os_mutex_destroy(&pgsql_mutex);
	return ERR_NOERROR;
}

DBAPI_HANDLE dbapi_pgsql_connect(DBAPI_PARAMETER* param, int count)
{
	int ret;
	DBAPI_PGSQL* conn;
	const char* host	= dbapi_parameter_get(param, count, "host");
	const char* port	= dbapi_parameter_get(param, count, "port");
	const char* options	= dbapi_parameter_get(param, count, "options");
	const char* user	= dbapi_parameter_get(param, count, "user");
	const char* passwd	= dbapi_parameter_get(param, count, "password");
	const char* db		= dbapi_parameter_get(param, count, "database");

	conn = pgsql_alloc();
	if(conn==NULL) return NULL;

	conn->pgconn = PQsetdbLogin(host, port, options, NULL, db, user, passwd);
	if(conn->pgconn==NULL) { pgsql_free(conn); return NULL; }

	conn->con.dbi = &dbapi_pgsql_provider;
	return (DBAPI_HANDLE) &(conn->con); 
}

int dbapi_pgsql_disconnect(DBAPI_HANDLE handle) 
{
	DBAPI_PGSQL* conn = (DBAPI_PGSQL*)handle;
    PQfinish(conn->pgconn);
	pgsql_free(conn);
	return ERR_NOERROR;
}

int dbapi_pgsql_release(DBAPI_HANDLE handle)
{
	DBAPI_PGSQL* conn = (DBAPI_PGSQL*)handle;

	return ERR_NOERROR;
}

int dbapi_pgsql_begin(DBAPI_HANDLE handle)
{
	DBAPI_PGSQL* conn = (DBAPI_PGSQL*)handle;
    PGresult *res;
    
    res = PQexec(conn->pgconn, "BEGIN");
    if(res==NULL || PQresultStatus(res)!=PGRES_COMMAND_OK)
        return _set_errcode(conn, NULL);
    PQclear(res); 

	return ERR_NOERROR;
}

int dbapi_pgsql_commit(DBAPI_HANDLE handle)
{
	DBAPI_PGSQL* conn = (DBAPI_PGSQL*)handle;
    PGresult *res;
    
    res = PQexec(conn->pgconn, "COMMIT");
    if(res==NULL || PQresultStatus(res)!=PGRES_COMMAND_OK)
        return _set_errcode(conn, NULL);
    PQclear(res); 

	return ERR_NOERROR;
}

int dbapi_pgsql_rollback(DBAPI_HANDLE handle)
{
	DBAPI_PGSQL* conn = (DBAPI_PGSQL*)handle;
    PGresult *res;
    
    res = PQexec(conn->pgconn, "ROLLBACK");
    if(res==NULL || PQresultStatus(res)!=PGRES_COMMAND_OK)
        return _set_errcode(conn, NULL);
    PQclear(res); 

	return ERR_NOERROR;
}

int dbapi_pgsql_execute(DBAPI_HANDLE handle,const char *sql)
{
	DBAPI_PGSQL* conn = (DBAPI_PGSQL*)handle;
    PGresult *res;

    res = PQexec(conn->pgconn, sql);
	if(res==NULL || PQresultStatus(res)!=PGRES_COMMAND_OK) {
		_set_errcode(conn, PQresultErrorField(res, PG_DIAG_SQLSTATE));
        PQclear(res);
        return ERR_UNKNOWN;
    }
	if(atoi(PQcmdTuples(res))==0) {
	    PQclear(res);
		return ERR_NOT_FOUND;
	}
    PQclear(res);

	return ERR_NOERROR;
}

int dbapi_pgsql_query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET *rs, int row_max)
{
	int ret;
	DBAPI_PGSQL* conn = (DBAPI_PGSQL*)handle;
	int row_idx, col_count, col_idx;
    PGresult* res;

    res = PQexec(conn->pgconn, sql);
	if(ret==NULL) return _set_errcode(conn, NULL);

	if(PQresultStatus(res)!=PGRES_TUPLES_OK) {
		_set_errcode(conn, PQresultErrorField(res, PG_DIAG_SQLSTATE));
	    PQclear(res);
		return ERR_UNKNOWN;
    }

	col_count = PQnfields(res); 

	return ERR_NOERROR;
}

int dbapi_pgsql_get_errcode(DBAPI_HANDLE handle)
{
	DBAPI_PGSQL* conn = (DBAPI_PGSQL*)handle;
	return conn->errcode;
}

const char* dbapi_pgsql_get_errmsg(DBAPI_HANDLE handle)
{
	DBAPI_PGSQL* conn = (DBAPI_PGSQL*)handle;
	return conn->errmsg;
}

static char *errcode_list[]=
{
    "00000", "01000", "0100C", "01008", "01003",
    "01004", "02000", "02001", "03000", "08000",
    "08003", "08006", "08001", "08004", "08007",
    "08P01", "09000", "0A000", "0B000", "0F000",
    "0F001", "0L000", "0LP01", "0P000", "21000",
    "22000", "2202E", "22021", "22008", "22012",
    "22005", "2200B", "22022", "22015", "22018",
    "22007", "22019", "2200D", "22025", "22010",
    "22020", "22023", "2201B", "22009", "2200C",
    "2200G", "22004", "22002", "22003", "22026",
    "22001", "22011", "22027", "22024", "2200F",
    "22P01", "22P02", "22P03", "22P04", "22P05",
    "23000", "23001", "23502", "23503", "23505",
    "23514", "24000", "25000", "25001", "25002",
    "25008", "25003", "25004", "25005", "25006",
    "25007", "25P01", "25P02", "26000", "27000",
    "28000", "2B000", "2BP01", "2D000", "2F000",
    "2F005", "2F002", "2F003", "2F004", "34000",
    "38000", "38001", "38002", "38003", "38004",
    "39000", "39001", "39004", "39P01", "39P02",
    "3D000", "3F000", "40000", "40002", "40001",
    "40003", "40P01", "42000", "42601", "42501",
    "42846", "42803", "42830", "42602", "42622",
    "42939", "42804", "42P18", "42809", "42703",
    "42883", "42P01", "42P02", "42704", "42701",
    "42P03", "42P04", "42723", "42P05", "42P06",
    "42P07", "42712", "42710", "42702", "42725",
    "42P08", "42P09", "42P10", "42611", "42P11",
    "42P12", "42P13", "42P14", "42P15", "42P16",
    "42P17", "44000", "53000", "53100", "53200",
    "53300", "54000", "54001", "54011", "54023",
    "55000", "55006", "55P02", "57000", "57014",
    "57P01", "57P02", "57P03", "58030", "58P01",
    "58P02", "F0000", "F0001", "XX000", "XX001",
    "XX002", NULL
};

int _set_errcode(DBAPI_PGSQL* conn, const char* errcode)
{
	int i;

	if(errcode) {
		strcpy(conn->errmsg, errcode);
		for(i=0; errcode_list[i]; i++) {
			if(strcmp(orig_code, errcode_list[i])==0) {
				conn->errcode = i + 1;
				return ERR_UNKNOWN;
			}
		}
	} else {
		strcpy(conn->errmsg, "UNKNOWN");
	}
	conn->errcode = -1;
	return ERR_UNKNOWN;
}
