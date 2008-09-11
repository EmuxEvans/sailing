#include <string.h>
#include <assert.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/mempool.h"
#include "../../inc/skates/hashmap.h"
#include "../../inc/skates/dbapi.h"

#include "dbapi_provider.h"

// STATIC CONFIG : Start
#define DBAPI_GROUP_COUNT		30
// STATIC CONFIG : End

static DBAPI_GROUP		dbapi_groups[DBAPI_GROUP_COUNT];
static os_mutex_t		dbapi_conn_mtx;
static HASHMAP_HANDLE	group_conn_map;
static MEMPOOL_HANDLE	dbapi_rs_pool;
static int				dbapi_rs_maxsize;

#ifdef DBAPI_SQLRELAY_EANBLE
extern DBAPI_PROVIDER dbapi_sqlrelay_provider;
#endif

extern DBAPI_PROVIDER dbapi_sqlite_provider;

static DBAPI_PROVIDER* dbapi_provider_list[20] = {
&dbapi_sqlite_provider,

#ifdef DBAPI_SQLRELAY_EANBLE
&dbapi_sqlrelay_provider,
#endif

NULL
};

static DBAPI_PROVIDER* get_provider(const char* connstr)
{
	int i;
	for(i=0; i<sizeof(dbapi_provider_list)/sizeof(dbapi_provider_list[0])-1; i++) {
		if (dbapi_provider_list[i] == NULL) return NULL;
		if(memcmp(connstr, dbapi_provider_list[i]->name, strlen(dbapi_provider_list[i]->name))==0) {
			return dbapi_provider_list[i];
		}
	}
	return NULL;
}

int dbapi_init(const char* libpath, int maxsize)
{
	int i;

	memset(dbapi_groups, 0, sizeof(dbapi_groups));
	os_mutex_init(&dbapi_conn_mtx);
	group_conn_map = hashmap_create(5, NULL, 0);
	dbapi_rs_maxsize = maxsize;
	dbapi_rs_pool = mempool_create("DBAPI_RECORDSET", sizeof(DBAPI_RECORDSET)+dbapi_rs_maxsize, 0);
	if(group_conn_map==NULL || dbapi_rs_pool==NULL) {
		if(group_conn_map!=NULL)	hashmap_destroy(group_conn_map);
		if(dbapi_rs_pool!=NULL)	mempool_destroy(dbapi_rs_pool);
		os_mutex_destroy(&dbapi_conn_mtx);
		return ERR_UNKNOWN;
	}
	// call all providers' Init()
	for (i=0; i<sizeof(dbapi_provider_list)/sizeof(dbapi_provider_list[0]); ++i) {
		if (dbapi_provider_list[i] == NULL) break;
		if (dbapi_provider_list[i]->Init() != ERR_NOERROR) {
			while (--i >= 0) dbapi_provider_list[i]->Final();
			hashmap_destroy(group_conn_map);
			mempool_destroy(dbapi_rs_pool);
			os_mutex_destroy(&dbapi_conn_mtx);
			return ERR_UNKNOWN;
		}
	}

	return ERR_NOERROR;
}

int dbapi_final()
{
	int i;
	DBAPI_HANDLE handle;

	for(i=0; i<sizeof(dbapi_groups)/sizeof(dbapi_groups[0]); i++) {
		if(dbapi_groups[i].connstr[0]=='\0') continue;
		handle = rlist_get_userdata(rlist_front(&dbapi_groups[i].free_list));
		rlist_remove(&dbapi_groups[i].conn_list, &handle->conn_item);
		rlist_remove(&dbapi_groups[i].free_list, &handle->free_item);
		handle->dbi->Disconnect(handle);
	}

	// call all providers' Final()
	for (i=0; i<sizeof(dbapi_provider_list)/sizeof(dbapi_provider_list[0]); ++i) {
		if(dbapi_provider_list[i]==NULL) continue;
		dbapi_provider_list[i]->Final();
	}

	mempool_destroy(dbapi_rs_pool);
	hashmap_destroy(group_conn_map);
	os_mutex_destroy(&dbapi_conn_mtx);
	return ERR_NOERROR;
}

int dbapi_register_provider(DBAPI_PROVIDER* p)
{
	return ERR_NOERROR;
}

int dbapi_unregister_provider(DBAPI_PROVIDER* p)
{
	return ERR_NOERROR;
}

DBAPI_HANDLE dbapi_connect(const char* connstr)
{
	DBAPI_PROVIDER* provider;
	DBAPI_GROUP* group;
	DBAPI_HANDLE conn;
	DBAPI_PARAMETER param[15];

	if(dbapi_crack_connstr(connstr, param, (int)sizeof(param)/sizeof(param[0]))!=ERR_NOERROR) {
		return NULL;
	}
	
	provider = get_provider(dbapi_parameter_get(param, (int)sizeof(param)/sizeof(param[0]), "provider"));
	if(provider==NULL) return NULL;

	os_mutex_lock(&dbapi_conn_mtx);
	group = (DBAPI_GROUP*)hashmap_get(group_conn_map, connstr, (unsigned int)strlen(connstr));
	if(group==NULL) {
		int i;
		for(i=0; i<sizeof(dbapi_groups)/sizeof(dbapi_groups[0]); i++) {
			if(dbapi_groups[i].connstr[0]=='\0') break;
		}
		if(i<sizeof(dbapi_groups)/sizeof(dbapi_groups[0])) {
			group = &dbapi_groups[i];
			strcpy(group->connstr, connstr);
			rlist_init(&group->free_list);
			rlist_init(&group->conn_list);
			hashmap_add(group_conn_map, &group->item, group->connstr, (unsigned int)strlen(group->connstr));
		}
	}
	if(group!=NULL && !rlist_empty(&group->free_list)) {
		conn = (DBAPI_HANDLE)rlist_get_userdata(rlist_front(&group->free_list));
		assert(conn!=NULL);
		rlist_pop_front(&group->free_list);
		rlist_clear(&conn->free_item, conn);
	} else {
		conn = NULL;
	}
	os_mutex_unlock(&dbapi_conn_mtx);
	if(conn!=NULL) return conn;

	conn = provider->Connect(param, sizeof(param)/sizeof(param[0]));
	if(conn==NULL) return NULL;

	os_mutex_lock(&dbapi_conn_mtx);
	conn->group = group;
	rlist_clear(&conn->conn_item, conn);
	rlist_clear(&conn->free_item, conn);
	rlist_push_back(&group->conn_list, &conn->conn_item);
	os_mutex_unlock(&dbapi_conn_mtx);

	return conn;
}

int dbapi_release(DBAPI_HANDLE handle)
{
	int ret;

	assert(handle->free_item.next==NULL && handle->free_item.prev==NULL && handle->free_item.ptr==handle);
	ret = handle->dbi->Release(handle);

	assert(!handle->in_transaction);
	if(handle->in_transaction) dbapi_rollback(handle);

	os_mutex_lock(&dbapi_conn_mtx);
	if(ret==ERR_NOERROR) {
		// add to free_list
		rlist_push_front(&handle->group->free_list, &handle->free_item);
	} else {
		// remove from conn_list
		rlist_remove(&handle->group->conn_list, &handle->conn_item);
		handle->dbi->Disconnect(handle);
	}
	os_mutex_unlock(&dbapi_conn_mtx);

	return ret;
}

int dbapi_begin(DBAPI_HANDLE handle)
{
	assert(!handle->in_transaction);
	handle->in_transaction = 1;
	return handle->dbi->Begin(handle);
}

int dbapi_commit(DBAPI_HANDLE handle)
{
	assert(handle->in_transaction);
	handle->in_transaction = 0;
	return handle->dbi->Commit(handle);
}

int dbapi_rollback(DBAPI_HANDLE handle)
{
	assert(handle->in_transaction);
	handle->in_transaction = 0;
	return handle->dbi->Rollback(handle);
}

int dbapi_execute(DBAPI_HANDLE handle, const char* sql)
{
	return handle->dbi->Execute(handle, sql);
}

int dbapi_query(DBAPI_HANDLE handle, const char* sql, DBAPI_RECORDSET **rs, int row_max)
{
	int ret;
	*rs = mempool_alloc(dbapi_rs_pool);
	if(*rs==NULL) return ERR_NO_ENOUGH_MEMORY;
	ret = handle->dbi->Query(handle, sql, *rs, row_max);
	if(ret!=ERR_NOERROR) {
		mempool_free(dbapi_rs_pool, *rs);
		*rs = NULL;
	}
	return ret;
}

int dbapi_get_errcode(DBAPI_HANDLE handle)
{
	return handle->dbi->GetErrorCode(handle);
}

const char* dbapi_get_errmsg(DBAPI_HANDLE handle)
{
	return handle->dbi->GetErrorMsg(handle);
}

int dbapi_recordset_set(DBAPI_RECORDSET* rs, int row_max, int col_count)
{
	if(sizeof(char*)*col_count + sizeof(char*)*col_count*row_max > (unsigned int)dbapi_rs_maxsize)
		return ERR_NO_ENOUGH_MEMORY;

	rs->row_max = row_max;
	rs->row_count = 0;
	rs->col_count = col_count;
	rs->buf = (char*)(rs+1);
	rs->buf_len = sizeof(char*)*col_count + sizeof(char*)*col_count*row_max;
	rs->fields_name = (char**)rs->buf;
	rs->rows = (char**)(rs->buf+sizeof(char*)*col_count);
	memset(rs->buf, 0, sizeof(char*)*col_count + sizeof(char*)*col_count*row_max);

	return ERR_NOERROR;
}

int dbapi_recordset_free(DBAPI_RECORDSET* recordset)
{
	mempool_free(dbapi_rs_pool, recordset);
	return ERR_NOERROR;
}

int dbapi_recordset_set_fieldname(DBAPI_RECORDSET* rs, int col, const char* name)
{
	int len;

	assert(col>=0 && col<rs->col_count);
	if(col<0 || col>=rs->col_count) return ERR_INVALID_PARAMETER;
	assert(rs->fields_name[col]==NULL);
	if(rs->fields_name[col]!=NULL) return ERR_INVALID_PARAMETER;

	/*if(len<=0)*/ len = (int)strlen(name) + 1;
	if(rs->buf_len+len>dbapi_rs_maxsize) return ERR_NO_ENOUGH_MEMORY;

	rs->fields_name[col] = rs->buf + rs->buf_len;
	memcpy(rs->buf+rs->buf_len, name, len);
	rs->buf_len += len;

	return ERR_NOERROR;
}

const char* dbapi_recordset_get_fieldname(DBAPI_RECORDSET* rs, int col)
{
	assert(col>=0 && col<rs->col_count);
	if(col<0 || col>=rs->col_count) return NULL;
	if(rs->fields_name[col]==NULL) return "";
	return rs->fields_name[col];
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
	if(recordset->rows[row*recordset->col_count+col]==NULL) return "";
	return recordset->rows[row*recordset->col_count+col];
}

int dbapi_recordset_put(DBAPI_RECORDSET* rs, int row, int col, const char* value, int len)
{
	assert(row>=0 && row<rs->row_max);
	if(row<0 || row>=rs->row_max) return ERR_INVALID_PARAMETER;
	assert(col>=0 && col<rs->col_count);
	if(col<0 || col>=rs->col_count) return ERR_INVALID_PARAMETER;
	assert(rs->rows[rs->col_count*row+col]==NULL);
	if(rs->rows[rs->col_count*row+col]!=NULL) return ERR_UNKNOWN;

	if(len<=0) len = (int)strlen(value) + 1;
	if(rs->buf_len+len>dbapi_rs_maxsize) return ERR_NO_ENOUGH_MEMORY;
	if(value!=rs->buf+rs->buf_len)
		memcpy(rs->buf+rs->buf_len, value, len);
	rs->rows[row*rs->col_count+col] = rs->buf+rs->buf_len;
	rs->buf_len += len;
	if(row>=rs->row_count) rs->row_count = row + 1;
	return ERR_NOERROR;
}

char* dbapi_recordset_getbuf(DBAPI_RECORDSET* recordset)
{
	return recordset->buf+recordset->buf_len;
}

int dbapi_recordset_getbuflen(DBAPI_RECORDSET* recordset)
{
	return dbapi_rs_maxsize - recordset->buf_len;
}

int dbapi_crack_connstr(const char* connstr, DBAPI_PARAMETER* list, int count)
{
	int i=0;
	char str[1024];
	char *head, *tail;

	if(strlen(connstr) >= sizeof(str)) return ERR_INVALID_PARAMETER;

	memset(str, 0, sizeof(str));
	strcpy(str, connstr);
	memset(list, 0, sizeof(*list)*count);

	head = tail = str;
	for (i=0; i<count; ++i) {
		// get name
		tail = strchr(head, '=');
		if(tail==NULL) return ERR_INVALID_PARAMETER;
		*tail = '\0';
		if(strlen(head) >= sizeof(list[i].name)) return ERR_INVALID_PARAMETER;
		strcpy(list[i].name, head);
		// get value
		head = tail+1;
		tail = strchr(head, ';');
		if(tail==NULL) tail = head + strlen(head);
		*tail = '\0';
		if(strlen(head) >= sizeof(list[i].value)) return ERR_INVALID_PARAMETER;
		strcpy(list[i].value, head);

		head = tail+1;
		if(*head=='\0') break;
	}

	return ERR_NOERROR;
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
