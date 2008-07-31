#include <string.h>
#include <assert.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/rlist.h"
#include "../inc/mempool.h"
#include "../inc/hashmap.h"
#include "../inc/dbapi.h"

#include "dbapi_provider.h"

// STATIC CONFIG : Start
#define DBAPI_GROUP_COUNT		30
// STATIC CONFIG : End

static DBAPI_GROUP		array[DBAPI_GROUP_COUNT];
static os_mutex_t		conn_mtx;
static HASHMAP_HANDLE	conn_map;
static MEMPOOL_HANDLE	rs_pool;
static int				rs_maxsize;

#ifdef DBAPI_SQLRELAY_EANBLE
extern DBAPI_PROVIDER dbapi_sqlrelay_provider;
#endif

static DBAPI_PROVIDER* provider_list[] = {
#ifdef DBAPI_SQLRELAY_EANBLE
&dbapi_sqlrelay_provider,
#endif
NULL
};

static DBAPI_PROVIDER* get_provider(const char* connstr)
{
	int i;
	for(i=0; i<sizeof(provider_list)/sizeof(provider_list[0])-1; i++) {
		if (provider_list[i] == NULL) return NULL;
		if(memcmp(connstr, provider_list[i]->name, strlen(provider_list[i]->name))==0) {
			return provider_list[i];
		}
	}
	return NULL;
}

int dbapi_init(const char* libpath, int maxsize)
{
	int i;

	memset(array, 0, sizeof(array));
	os_mutex_init(&conn_mtx);
	conn_map = hashmap_create(5, NULL, 0);
	rs_maxsize = maxsize;
	rs_pool = mempool_create("DBAPI_RECORDSET", sizeof(DBAPI_RECORDSET)+rs_maxsize, 0);
	if(conn_map==NULL || rs_pool==NULL) {
		if(conn_map!=NULL)	hashmap_destroy(conn_map);
		if(rs_pool!=NULL)	mempool_destroy(rs_pool);
		os_mutex_destroy(&conn_mtx);
		return ERR_UNKNOWN;
	}
	// call all providers' Init()
	for (i=0; i<sizeof(provider_list)/sizeof(provider_list[0]); ++i) {
		if (provider_list[i] == NULL) break;
		if (provider_list[i]->Init() != ERR_NOERROR) {
			while (--i >= 0) provider_list[i]->Final();
			hashmap_destroy(conn_map);
			mempool_destroy(rs_pool);
			os_mutex_destroy(&conn_mtx);
			return ERR_UNKNOWN;
		}
	}

	return ERR_NOERROR;
}

int dbapi_final()
{
	int i;
	DBAPI_HANDLE handle;

	for(i=0; i<sizeof(array)/sizeof(array[0]); i++) {
		if(array[i].connstr[0]=='\0') continue;
		handle = rlist_get_userdata(rlist_front(&array[i].free_list));
		rlist_remove(&array[i].conn_list, &handle->conn_item);
		rlist_remove(&array[i].free_list, &handle->free_item);
		handle->dbi->Disconnect(handle);
	}

	// call all providers' Final()
	for (i=0; i<sizeof(provider_list)/sizeof(provider_list[0]); ++i) {
		if (provider_list[i] == NULL) break;
		provider_list[i]->Final();
	}

	mempool_destroy(rs_pool);
	hashmap_destroy(conn_map);
	os_mutex_destroy(&conn_mtx);
	return ERR_NOERROR;
}

DBAPI_HANDLE dbapi_connect(const char* connstr)
{
	DBAPI_PROVIDER* provider;
	DBAPI_GROUP* group;
	DBAPI_HANDLE conn;
	DBAPI_PARAMETER param[16];

	memset(param, 0, sizeof(param));
	if(dbapi_crack_connstr(connstr, param, (int)sizeof(param)/sizeof(param[0]))!=ERR_NOERROR) {
		return NULL;
	}
	
	provider = get_provider(dbapi_parameter_get(param, (int)sizeof(param)/sizeof(param[0]), "provider"));
	if(provider==NULL) return NULL;

	os_mutex_lock(&conn_mtx);
	group = (DBAPI_GROUP*)hashmap_get(conn_map, connstr, (unsigned int)strlen(connstr));
	if(group!=NULL && !rlist_empty(&group->free_list)) {
		conn = (DBAPI_HANDLE)rlist_get_userdata(rlist_front(&group->free_list));
		assert(conn!=NULL);
		rlist_pop_front(&group->free_list);
		rlist_clear(&conn->free_item, conn);
	} else {
		conn = NULL;
	}
	os_mutex_unlock(&conn_mtx);
	if(conn!=NULL) return conn;

	conn = provider->Connect(connstr);
	if(conn==NULL) return NULL;

	os_mutex_lock(&conn_mtx);
	group = (DBAPI_GROUP*)hashmap_get(conn_map, connstr, (unsigned int)strlen(connstr));
	if(group==NULL) {
		int i;
		for(i=0; i<sizeof(array)/sizeof(array[0]); i++) {
			if(array[i].connstr[0]=='\0') {
				group = &array[i];
			}
		}
		if(group!=NULL) {
			strcpy(group->connstr, connstr);
			rlist_init(&group->free_list);
			rlist_init(&group->conn_list);
			hashmap_add_unsafe(conn_map, &group->item, group->connstr, (unsigned int)strlen(group->connstr));
		}
	}
	if(group!=NULL) {
		conn->group = group;
		rlist_clear(&conn->conn_item, conn);
		rlist_clear(&conn->free_item, conn);
		rlist_push_back(&group->conn_list, &conn->conn_item);
	} else {
		// ????
		provider->Disconnect(conn);
		conn = NULL;
	}
	os_mutex_unlock(&conn_mtx);

	return conn;
}

int dbapi_release(DBAPI_HANDLE handle)
{
	int ret;
	assert(handle->free_item.next==NULL && handle->free_item.prev==NULL && handle->free_item.ptr==handle);
	ret = handle->dbi->Release(handle);
	os_mutex_lock(&conn_mtx);
	if(ret==ERR_NOERROR) {
		// add to free_list
		rlist_push_front(&handle->group->free_list, &handle->free_item);
	} else {
		// remove from conn_list
		rlist_remove(&handle->group->conn_list, &handle->conn_item);
		handle->dbi->Disconnect(handle);
	}
	os_mutex_unlock(&conn_mtx);
	return ret;
}

int dbapi_begin(DBAPI_HANDLE handle)
{
	return handle->dbi->Begin(handle);
}

int dbapi_commit(DBAPI_HANDLE handle)
{
	return handle->dbi->Commit(handle);
}

int dbapi_rollback(DBAPI_HANDLE handle)
{
	return handle->dbi->Rollback(handle);
}

int dbapi_execute(DBAPI_HANDLE handle, const char* sql)
{
	return handle->dbi->Execute(handle, sql);
}

int dbapi_query(DBAPI_HANDLE handle, const char* sql, DBAPI_RECORDSET **result_set, int row_max)
{
	return handle->dbi->Query(handle, sql, result_set, row_max);
}

int dbapi_get_errcode(DBAPI_HANDLE handle)
{
	return handle->dbi->GetErrorCode(handle);
}

const char* dbapi_get_errmsg(DBAPI_HANDLE handle)
{
	return handle->dbi->GetErrorMsg(handle);
}

DBAPI_RECORDSET* dbapi_recordset_alloc(int col_count, int row_max)
{
	DBAPI_RECORDSET* rs;
	rs = (DBAPI_RECORDSET*)mempool_alloc(rs_pool);
	if(rs==NULL) return NULL;
	rs->row_max = row_max;
	rs->row_count = 0;
	rs->col_count = col_count;
	rs->buf = (char*)(rs+1);
	rs->buf_len = sizeof(char*)*col_count*row_max;
	rs->fields_name = NULL;
	rs->rows = (char**)rs->buf;
	return rs;
}

int dbapi_recordset_free(DBAPI_RECORDSET* recordset)
{
	mempool_free(rs_pool, recordset);
	return ERR_NOERROR;
}

int dbapi_recordset_row_count(DBAPI_RECORDSET* recordset)
{
	return recordset->row_count;
}

int dbapi_recordset_col_count(DBAPI_RECORDSET* recordset)
{
	return recordset->col_count;
}

const char* dbapi_recordset_get(DBAPI_RECORDSET* recordset, int row, int col)
{
	assert(col<recordset->col_count && row<recordset->row_count && col>=0 && row>=0);
	if(col>=recordset->col_count || row>=recordset->row_count || col<0 || row<0) return NULL;
	return recordset->rows[row*recordset->col_count+col];
}

int dbapi_recordset_put(DBAPI_RECORDSET* recordset, int col, int row, const char* value, int len)
{
	assert(row>=0 && row<recordset->row_max);
	if(row<0 || row>=recordset->row_max) { assert(0); return ERR_INVALID_PARAMETER; }
	assert(col>=0 && col<recordset->col_count);
	if(col<0 || col>=recordset->col_count) { assert(0); return ERR_INVALID_PARAMETER; }
	if(len<=0) len = (int)strlen(value) + 1;
	if(recordset->buf_len+len>rs_maxsize) return ERR_OUT_OF_RANGE;
	if(value!=recordset->buf+recordset->buf_len)
		memcpy(recordset->buf+recordset->buf_len, value, len);
	recordset->rows[row*recordset->col_count+col] = recordset->buf+recordset->buf_len;
	recordset->buf_len += len;
	if(row>=recordset->row_count) recordset->row_count = row + 1;
	return ERR_NOERROR;
}

char* dbapi_recordset_getbuf(DBAPI_RECORDSET* recordset)
{
	return recordset->buf+recordset->buf_len;
}

int dbapi_recordset_getbuflen(DBAPI_RECORDSET* recordset)
{
	return rs_maxsize - recordset->buf_len;
}

int dbapi_crack_connstr(const char* connstr, DBAPI_PARAMETER* list, int count)
{
	int i=0;
	char str[1024];
	char *head, *tail;

	if (strlen(connstr) >= sizeof(str)) return ERR_INVALID_PARAMETER;
	strcpy(str, connstr);

	head = tail = str;
	for (i=0; i<count; ++i) {
		// get name
		if (NULL == (tail = strchr(head,'='))) return ERR_NO_DATA;
		(*tail) = '\0';
		if (strlen(head) >= sizeof(list[i].name)) return ERR_INVALID_PARAMETER;
		strcpy(list[i].name, head);
		// get value
		head = tail+1;
		if (NULL == (tail = strchr(head, ';'))) return ERR_NO_DATA;
		(*tail) = '\0';
		if (strlen(head) >= sizeof(list[i].value)) return ERR_INVALID_PARAMETER;
		strcpy(list[i].value, head);
		
		head = tail+1;
		if ((*head) == '\0') break;
	}

	return ((*head)!='\0') ? ERR_NO_ENOUGH_MEMORY : ERR_NOERROR;
}

const char* dbapi_parameter_get(const DBAPI_PARAMETER* list, int count, const char* name)
{
	int i;
	for(i=0; i<count; i++) {
		if(strcmp(name, list[i].name)==0) {
			return list[i].value;
		}
	}
	return NULL;
}
