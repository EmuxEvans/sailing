#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sqlrelay/sqlrclientwrapper.h>

#include "../inc/os.h"
#include "../inc/errcode.h"
#include "../inc/rlist.h"
#include "../inc/hashmap.h"
#include "../inc/mempool.h"
#include "../inc/dbapi.h"
#include "dbapi_provider.h"

typedef struct DBAPI_SQLREALY
{
	DBAPI_CONNECTION  con;
	sqlrcon relaycon;
	sqlrcur curcon;
	DBAPI_PARAMETER param[5];
	int errcode;
}DBAPI_SQLRELAY;


static int DB_Init(void);
static int DB_Final(void);
static DBAPI_HANDLE DB_Connect(const char *connstr);
static int DB_Disconnect(DBAPI_HANDLE handle);
static int DB_Release(DBAPI_HANDLE handle);
static int DB_Begin(DBAPI_HANDLE handle);
static int DB_Commit(DBAPI_HANDLE handle);
static int DB_Rollback(DBAPI_HANDLE handle);
static int DB_Execute(DBAPI_HANDLE handle,const char *sql);
static int DB_Query(DBAPI_HANDLE handle, const char *sql, DBAPI_RECORDSET **result_set, int row_max);
static int DB_GetErrorCode(DBAPI_HANDLE handle);
static const char * DB_GetErrorMsg(DBAPI_HANDLE handle);

static MEMPOOL_HANDLE  mem_handle;


DBAPI_PROVIDER dbapi_sqlrelay_provider =
{
	1,
	"sqlrelay",
	1,
	DB_Init,
	DB_Final,
	DB_Connect,
	DB_Disconnect,
	DB_Release,
	DB_Begin,
	DB_Commit,
	DB_Rollback,
	DB_Execute,
	DB_Query,
	DB_GetErrorCode,
	DB_GetErrorMsg	
};


int DB_Init(void)
{
	mem_handle = NULL;
	mem_handle = mempool_create(sizeof(DBAPI_SQLRELAY), 0);
	if(mem_handle == NULL)
	{
		return ERR_UNKNOWN;
	}
	else
	{
		return ERR_NOERROR;
	}
}

int DB_Final(void)
{
	if(mem_handle != NULL)
	{
		mempool_destroy(mem_handle);
	}

	return ERR_NOERROR ;
}


DBAPI_HANDLE DB_Connect(const char * connstr)
{
	char *ip = NULL;
	char *port = NULL;
	char *user = NULL;
	char *pwd =NULL;
	DBAPI_SQLRELAY *handle = (DBAPI_SQLRELAY*)mempool_alloc(mem_handle);
	if (handle == NULL) return NULL;

	if (dbapi_crack_connstr(connstr, handle->param, sizeof(handle->param)) != ERR_NOERROR) return NULL;
	
	ip =(char*) dbapi_parameter_get(handle->param, 5, "ip");
	port = (char*)dbapi_parameter_get(handle->param, 5, "port");	
	user =(char*) dbapi_parameter_get(handle->param, 5, "user");
	pwd = (char*)dbapi_parameter_get(handle->param, 5, "pwd");

	if(ip==NULL || port==NULL || user==NULL || pwd==NULL)
	{
		mempool_free(mem_handle, handle);
		return NULL;
	}
	handle->relaycon = sqlrcon_alloc(ip, atoi(port), "", user, pwd, 0, 1);

	handle->curcon=sqlrcur_alloc(handle->relaycon);
	handle->errcode = ERR_NOERROR;
	handle->con.dbi = &dbapi_sqlrelay_provider;
	return (DBAPI_HANDLE) &(handle->con); 
}

int DB_Disconnect(DBAPI_HANDLE handle) 
{
	DBAPI_SQLRELAY *p=(DBAPI_SQLRELAY*)handle;
	sqlrcur_free(p->curcon);
	sqlrcon_free(p->relaycon);
	//handle alloc memory int the connect function
	mempool_free(mem_handle, handle);	
	p=NULL;
	return ERR_NOERROR ;
}

int DB_Release(DBAPI_HANDLE handle)
{
	DBAPI_SQLRELAY *p=(DBAPI_SQLRELAY*)handle;
	sqlrcon_endSession(p->relaycon);
	return ERR_NOERROR;
}

int DB_Begin(DBAPI_HANDLE handle)
{
	return ERR_NOERROR ;
	//THERE IS NO FUNTION TO CARRY OUT this function ,so this function do nothing 

}

int DB_Commit(DBAPI_HANDLE handle)
{
	DBAPI_SQLRELAY *p=(DBAPI_SQLRELAY*)handle;
	p->errcode =  sqlrcon_commit(p->relaycon);
	return p->errcode;
	//1 means success,0 means fail,-1 means an error 
}

int DB_Rollback(DBAPI_HANDLE handle)
{
	DBAPI_SQLRELAY *p=(DBAPI_SQLRELAY*)handle;
	p->errcode = sqlrcon_rollback(p->relaycon);
	return p->errcode;
	//1 means success,0 means fail,-1 means an error 
}

int DB_Execute(DBAPI_HANDLE handle,const char *sql)
{
	DBAPI_SQLRELAY *p=(DBAPI_SQLRELAY*)handle;
	p->errcode = ERR_NOERROR;
	assert(sql);
	if(!sql)
	{
		p->errcode = ERR_UNKNOWN;
	}
	else
	{	
		if(!sqlrcur_sendQuery(p->curcon, sql))
		{
			p->errcode = ERR_UNKNOWN;
		}
		else
		{
			if(sqlrcur_affectedRows(p->curcon) == 0)
			{
                                p->errcode = ERR_NOT_FOUND;
                        }
		}
	}
	return p->errcode;	
}

int DB_Query(DBAPI_HANDLE handle,const char *sql,DBAPI_RECORDSET **result_set,  int row_max)
{
	char* pBuffer = NULL;
	int colnum=0;
	DBAPI_SQLRELAY *p=(DBAPI_SQLRELAY*)handle;
	p->errcode = ERR_NOERROR;
	assert(sql);
	if(!sql)
	{
		p->errcode = ERR_UNKNOWN;
		*result_set = NULL;
		return p->errcode;
	}

	//select return the recordset;
	if( !sqlrcur_sendQuery(p->curcon,sql) )
	{
		p->errcode = ERR_UNKNOWN;
		*result_set = NULL;
		 return p->errcode;
	}	
	
	//get column number of result
	colnum=sqlrcur_colCount(p->curcon);
	//get row number of result
	int rownum=sqlrcur_rowCount(p->curcon);
		
	if(rownum == 0)
	{	
		p->errcode = ERR_NOT_FOUND;
		*result_set = NULL;
		 return p->errcode;
	}
	

	//you need  to buffer the result
	*result_set=dbapi_recordset_alloc(colnum,row_max);
	//assert(recordset);
	if(!*result_set)
	{
		//alloc error
		p->errcode = ERR_UNKNOWN;
		return p->errcode;
	}

	int row,col;
	for(row=0; row<(row_max > rownum? rownum:row_max ); row++)
	{
		for(col=0; col<colnum; col++)
		{
			pBuffer=(char*)sqlrcur_getFieldByIndex(p->curcon, row, col);
			//insert the recordset
			if(dbapi_recordset_put(*result_set, col, row, pBuffer, strlen(pBuffer)+1) == ERR_OUT_OF_RANGE)
			{
				p->errcode = ERR_OUT_OF_RANGE;
				dbapi_recordset_free(*result_set);
				return p->errcode;
			}
		}
	}
	p->errcode = ERR_NOERROR;
	return p->errcode;
	
}

int DB_GetErrorCode(DBAPI_HANDLE handle)
{
	DBAPI_SQLRELAY *p=(DBAPI_SQLRELAY*)handle;
	return p->errcode;
}

const char* DB_GetErrorMsg(DBAPI_HANDLE handle)
{
	DBAPI_SQLRELAY *p=(DBAPI_SQLRELAY*)handle;     
	return sqlrcur_errorMessage(p->curcon);
}

