#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/dbapi.h>
#include <skates/mempool.h>

static void do_test();

int main(int argc, char* argv[])
{
	mempool_init();
	dbapi_init(NULL, 100*1024);

	do_test();

	dbapi_final();
	mempool_final();
	return(0);
}

void do_test()
{
	DBAPI_HANDLE handle;

	handle = dbapi_connect("provider=sqlite;dbname=aaaaaa");
	if(handle==NULL) {
		printf("Failed to dbapi_connect()\n");
		return;
	}

	for(;;) {
		int ret;
		char line[1000], cmd[50];

		printf(">");
		gets(line);
		if(strcmp(line, "quit")==0) break;
		if(sscanf(line, "%s", cmd)!=1) continue;

		if(strcmp(cmd, "begin")==0) {
			ret = dbapi_begin(handle);
			if(ret==ERR_NOERROR) { printf("done\n"); continue; }
			printf("SQLERR: %d %s\n", dbapi_get_errcode(handle), dbapi_get_errmsg(handle));
			continue;
		}
		if(strcmp(cmd, "commit")==0) {
			ret = dbapi_commit(handle);
			if(ret==ERR_NOERROR) { printf("done\n"); continue; }
			printf("SQLERR: %d %s\n", dbapi_get_errcode(handle), dbapi_get_errmsg(handle));
			continue;
		}
		if(strcmp(cmd, "rollback")==0) {
			ret = dbapi_rollback(handle);
			if(ret==ERR_NOERROR) { printf("done\n"); continue; }
			printf("SQLERR: %d %s\n", dbapi_get_errcode(handle), dbapi_get_errmsg(handle));
			continue;
		}
		if(strcmp(cmd, "select")==0) {
			DBAPI_RECORDSET* rs;
			int col, row;
			unsigned int colw[100];
			char fmt[100];
			memset(colw, 0, sizeof(colw));
			
			ret = dbapi_query(handle, line, &rs, 100);
			if(ret==ERR_NOT_FOUND) { printf("not found\n"); continue; }
			if(ret!=ERR_NOERROR) {
				printf("SQLERR: %d %s\n", dbapi_get_errcode(handle), dbapi_get_errmsg(handle));
				continue;
			}

			for(col=0; col<dbapi_recordset_col_count(rs); col++) {
				if(strlen(dbapi_recordset_get_fieldname(rs, col))>colw[col]) colw[col] = strlen(dbapi_recordset_get_fieldname(rs, col));
				for(row=0; row<dbapi_recordset_row_count(rs); row++) 
					if(strlen(dbapi_recordset_get(rs, row, col))>colw[col]) colw[col] = strlen(dbapi_recordset_get(rs, row, col));

				sprintf(fmt, "%%%ds", colw[col]);
				printf(fmt, dbapi_recordset_get_fieldname(rs, col));
			}
			printf("\n");

			for(row=0; row<dbapi_recordset_row_count(rs); row++) {
				for(col=0; col<dbapi_recordset_col_count(rs); col++) {
					sprintf(fmt, "%%%ds", colw[col]);
					printf(fmt, dbapi_recordset_get(rs, row, col));
				}
				printf("\n");
			}
		}

		ret = dbapi_execute(handle, line);
		if(ret==ERR_NOERROR) { printf("done\n"); continue; }
		printf("SQLERR: %d %s\n", dbapi_get_errcode(handle), dbapi_get_errmsg(handle));
	}

	dbapi_release(handle);
}
