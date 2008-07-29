#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "../inc/skates.h"
#include "../inc/octopus.h"
#include "../inc/appbox_main.h"

#define MODULE_MAXCOUNT			30

typedef int (* MODULE_ENTRY)(int reason);

typedef struct MODULE_INFO {
	char			name[100];
	os_library_t	dlhandle;
	MODULE_ENTRY	entry;
	int				ret;
} MODULE_INFO;

typedef struct CONFIG_ITEM {
	char	name[40];
	int		type;
	char	value[300];
} CONFIG_ITEM;

// 
static CONSOLE_INSTANCE* con_instance = NULL;
static MODULE_INFO modules[MODULE_MAXCOUNT];
static int module_count;
static CONFIG_ITEM configs[1000];
static int config_count = 0;

#ifdef APPBOX_MAIN
char config_syslog[200] = "console://";
char config_dbglog[200] = "console://";
int config_syslog_enable = 1;
int	config_dbglog_enable = 1;
int	config_worker_count = 5;
int	config_timer_interval = 100;
char config_module_list[300] = "";
SOCK_ADDR config_rpc_endpoint = { 0xffffffff, 0xffff };
SOCK_ADDR config_con_endpoint = { 0xffffffff, 0xffff };
unsigned int config_con_maxconns = 10;
unsigned int config_con_maxhooker = 100;
#endif

APPBOX_SETTING_BEGIN(appbox_settings)
	APPBOX_SETTING_STRING("syslog", config_syslog, sizeof(config_syslog))
	APPBOX_SETTING_STRING("dbglog", config_dbglog, sizeof(config_dbglog))
	APPBOX_SETTING_INTEGER("syslog_enable", config_syslog_enable)
	APPBOX_SETTING_INTEGER("dbglog_enable", config_dbglog_enable)
	APPBOX_SETTING_INTEGER("worker_count", config_worker_count)
	APPBOX_SETTING_INTEGER("timer_interval", config_timer_interval)
	APPBOX_SETTING_STRING("module_list", config_module_list, sizeof(config_module_list))
	APPBOX_SETTING_ENDPOINT("rpc_endpoint", config_rpc_endpoint)
	APPBOX_SETTING_ENDPOINT("con_endpoint", config_con_endpoint)
	APPBOX_SETTING_INTEGER("con_maxconns", config_con_maxconns)
	APPBOX_SETTING_INTEGER("con_maxhooker", config_con_maxhooker)
APPBOX_SETTING_END(appbox_settings)

static int appbox_quit(int ret);
static const CONFIG_ITEM* get_config_item(const char* name);

static int appbox_console_set(CONSOLE_CONNECTION* conn, const char* name, const char* line);
static int appbox_console_get(CONSOLE_CONNECTION* conn, const char* name, const char* line);
static int appbox_console_coredump(CONSOLE_CONNECTION* conn, const char* name, const char* line);
static int appbox_console_threadpool(CONSOLE_CONNECTION* conn, const char* name, const char* line);

int appbox_config_load(const char* filename)
{
	FILE* fp;
	int lnum, type;
	char line[400];
	char *ttype;
	char *tvalue;

	fp = fopen(filename, "rt");
	if(fp==NULL) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to open config(%s).", filename);
		return ERR_UNKNOWN;
	}

	fgets(line, sizeof(line), fp);
	lnum = 1;
	while(!feof(fp)) {
		for(type=0; type<(int)strlen(line); type++) {
			if(line[type]<' ') line[type] = ' ';
		}
		strtrim(line);
		strltrim(line);
		if(line[0]!='\0' && line[0]!='#') {
			ttype = strstr(line, " ");
			tvalue = NULL;

			if(ttype!=NULL) {
				*(ttype++) = '\0';
				strltrim(ttype);
				tvalue = strstr(ttype, " ");
				if(tvalue!=NULL) {
					*(tvalue++) = '\0';
					strltrim(tvalue);
				} else {
					tvalue = "";
				}

				type = CONFIGDATA_UNKNOWN;
				if(strcmp(ttype, "I")==0) type = CONFIGDATA_INTEGER;
				if(strcmp(ttype, "F")==0) type = CONFIGDATA_FLOAT;
				if(strcmp(ttype, "S")==0) type = CONFIGDATA_STRING;
				if(strcmp(ttype, "B")==0) type = CONFIGDATA_BINARY;
				if(strcmp(ttype, "E")==0) type = CONFIGDATA_ENDPOINT;
			}

			if(tvalue==NULL || type==CONFIGDATA_UNKNOWN) {
				SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to load config(%s), error in line %d.", filename, lnum);
				fclose(fp);
				return ERR_UNKNOWN;
			}

			strcpy(configs[config_count].name, line);
			configs[config_count].type = type;
			strcpy(configs[config_count].value, tvalue);
			config_count++;
		}

		lnum++;
		fgets(line, sizeof(line), fp);
	}

	fclose(fp);
	return ERR_NOERROR;
}

int appbox_config_init()
{
	int ret;
	ret = appbox_config_get(MODULE_NAME, appbox_settings);
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to appbox_getconfigs(), ret=%d", ret);
	return ret;
}

int appbox_config_get(const char* module_name, APPBOX_SETTING* tab)
{
	const CONFIG_ITEM* item;
	int loopi = 0;
	char key[100];
	
	while(tab[loopi].name != NULL)	{
		sprintf(key, "%s.%s", module_name, tab[loopi].name);
		item = get_config_item(key);
		if(item==NULL) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to get config(%s), not exist.", key);
			return ERR_UNKNOWN;
		}
		if(item->type!=tab[loopi].type) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to get config(%s), invalid type.", key);
			return ERR_UNKNOWN;
		}

		switch(tab[loopi].type) {
		case CONFIGDATA_STRING:
			if(tab[loopi].maxlen<=strlen(item->value)) {
				SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to get config(%s), too long.", key);
				return ERR_UNKNOWN;
			}
			strcpy((char*)tab[loopi].ptr, item->value);
			break;		
		case CONFIGDATA_INTEGER:
			*((int*)(tab[loopi].ptr)) = atoi(item->value);
			break;		
		case CONFIGDATA_FLOAT:
			*((int*)(tab[loopi].ptr)) = atoi(item->value);
			break;				
		case CONFIGDATA_BINARY:
			if(strlen(item->value)%2>0) {
				SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to get config(%s), invalid data.", key);
				return ERR_UNKNOWN;
			}
			if(strlen(item->value)/2>tab[loopi].maxlen) {
				SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to get config(%s), too long.", key);
				return ERR_UNKNOWN;
			}
			hex2bin(item->value, tab[loopi].ptr, (int)strlen(item->value)/2);
			break;						
		case CONFIGDATA_ENDPOINT:
			if(sock_str2addr(item->value, (SOCK_ADDR*)tab[loopi].ptr)==NULL) {
				SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to get config(%s), invalid data.", key);
				return ERR_UNKNOWN;
			}
			break;
		}

		loopi++;
	}
	
	return ERR_NOERROR;
}

int appbox_init()
{
	int ret;

	if(config_syslog[0]!='\0') {
		ret = syslog_open(config_syslog);
		if(ret!=ERR_NOERROR) {
			printf("Failed to open syslog or dbglog.\n");
			return appbox_quit(ERR_UNKNOWN);
		}
	}
	if(config_dbglog[0]!='\0') {
		ret = dbglog_open(config_dbglog);
		if(ret!=ERR_NOERROR) {
			printf("Failed to open syslog or dbglog.\n");
			return appbox_quit(ERR_UNKNOWN);
		}
	}
	SYSLOG(LOG_INFO, MODULE_NAME, "== SYSLOG START ==");
	DBGLOG(LOG_INFO, MODULE_NAME, "== DBGLOG START ==");

	mempool_init();
	sock_init();
	ret = fdwatch_init();
	if(ret!=ERR_NOERROR) { SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_init() fail, ret=%d", ret); return appbox_quit(ERR_UNKNOWN); }

	if(config_con_endpoint.ip!=0xffffffff && config_con_endpoint.port!=0xffff) {
		con_instance = console_create(&config_con_endpoint, config_con_maxconns, config_con_maxhooker);
		if(con_instance==NULL) {
			char ep[40];
			sock_addr2str(&config_con_endpoint, ep);
			SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to create console instance at %s.", ep);
			return appbox_quit(ERR_UNKNOWN);
		}

		ret = appbox_reg_command(MODULE_NAME, "set", appbox_console_set);
		assert(ret==ERR_NOERROR);
		ret = appbox_reg_command(MODULE_NAME, "get", appbox_console_get);
		assert(ret==ERR_NOERROR);
		ret = appbox_reg_command(MODULE_NAME, "coredump", appbox_console_coredump);
		assert(ret==ERR_NOERROR);
		ret = appbox_reg_command(MODULE_NAME, "threadpool_info", appbox_console_threadpool);
		assert(ret==ERR_NOERROR);
		ret = appbox_reg_command(MODULE_NAME, "threadpool_status", appbox_console_threadpool);
		assert(ret==ERR_NOERROR);
	}

	ret = dbapi_init(NULL, 0);
	if(ret!=ERR_NOERROR) { SYSLOG(LOG_ERROR, MODULE_NAME, "dbapi_init() fail, ret=%d", ret); return appbox_quit(ERR_UNKNOWN); }
	ret = threadpool_init(config_worker_count);
	if(ret!=ERR_NOERROR) { SYSLOG(LOG_ERROR, MODULE_NAME, "threadpool_init() fail, ret=%d", ret); return appbox_quit(ERR_UNKNOWN); }
	ret = timer_init(config_timer_interval);
	if(ret!=ERR_NOERROR) { SYSLOG(LOG_ERROR, MODULE_NAME, "timer_init() fail, ret=%d", ret); return appbox_quit(ERR_UNKNOWN); }
	ret = rpcnet_init();
	if(ret!=ERR_NOERROR) { SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_init() fail, ret=%d", ret); return appbox_quit(ERR_UNKNOWN); }
	ret = rpcfun_init();
	if(ret!=ERR_NOERROR) { SYSLOG(LOG_ERROR, MODULE_NAME, "rpcfun_init() fail, ret=%d", ret); return appbox_quit(ERR_UNKNOWN); }
	if(config_rpc_endpoint.ip!=0xffffffff && config_rpc_endpoint.port!=0xffff) {
		ret = rpcnet_bind(&config_rpc_endpoint);
		if(ret!=ERR_NOERROR) { SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_bind() fail, ret=%d", ret); return appbox_quit(ERR_UNKNOWN); }
	}

#ifndef APPBOX_WITHOUT_OCTOPUS
	gameroom_init();
	fes_userctx_init();
#endif

	return ERR_NOERROR;
}

int appbox_final()
{
	int ret;

#ifndef APPBOX_WITHOUT_OCTOPUS
	fes_userctx_final();
	gameroom_final();
#endif

	if(config_rpc_endpoint.ip!=0xffffffff && config_rpc_endpoint.port!=0xffff) {
		ret = rpcnet_unbind();
		if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_unbind() fail, ret=%d", ret);
	}
	ret = rpcfun_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "rpcfun_final() fail, ret=%d", ret);
	ret = rpcnet_shutdown();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_shutdown() fail, ret=%d", ret);
	ret = rpcnet_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_final() fail, ret=%d", ret);
	ret = timer_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "timer_final() fail, ret=%d", ret);
	ret = threadpool_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "threadpool_final() fail, ret=%d", ret);
	ret = dbapi_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "dbapi_final() fail, ret=%d", ret);

	if(con_instance!=NULL) {
		ret = appbox_unreg_command(MODULE_NAME, "set", appbox_console_set);
		assert(ret==ERR_NOERROR);
		ret = appbox_unreg_command(MODULE_NAME, "get", appbox_console_get);
		assert(ret==ERR_NOERROR);
		ret = appbox_unreg_command(MODULE_NAME, "coredump", appbox_console_coredump);
		assert(ret==ERR_NOERROR);
		ret = appbox_unreg_command(MODULE_NAME, "threadpool_info", appbox_console_threadpool);
		assert(ret==ERR_NOERROR);
		ret = appbox_unreg_command(MODULE_NAME, "threadpool_status", appbox_console_threadpool);
		assert(ret==ERR_NOERROR);

		ret = console_destroy(con_instance);
		if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "console_destroy() fail, ret=%d", ret);
	}

	ret = fdwatch_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_final() fail, ret=%d", ret);
	sock_final();
	mempool_final();

	SYSLOG(LOG_INFO, MODULE_NAME, "== SYSLOG END ==");
	DBGLOG(LOG_INFO, MODULE_NAME, "== DBGLOG END ==");
	if(dbglog_close()!=ERR_NOERROR) {
	}
	if(syslog_close()!=ERR_NOERROR) {
	}

	return ERR_NOERROR;
}

int appbox_quit(int ret)
{
	DBGLOG(LOG_INFO, MODULE_NAME, "== DBGLOG END ==");
	SYSLOG(LOG_INFO, MODULE_NAME, "== SYSLOG END ==");
	dbglog_close();
	syslog_close();
	return ret;
}

int appbox_load_modules()
{
	int count, i, ret;
	char filename[100];
	char name[20];
	char* cur;
	char* tok;

	for(i = 0; i <MODULE_MAXCOUNT; i++)	{
		modules[i].dlhandle = NULL;
		modules[i].ret = ERR_UNKNOWN;
	}

	// load dynamic library & find module_entry
	cur = config_module_list;
	tok = strstr(cur, ";");
	count = 0;
	while(*cur!='\0') {
		if(tok==NULL) {
			strcpy(name, cur);
		} else {
			memcpy(name, cur, tok-cur);
			name[tok-cur] = '\0';
		}
		strtrim(name);
		if(name[0]!='\0') {
			strcpy(modules[count].name, name);
			sprintf(filename, "./modules/%s%s", name, OS_LIBARAY_PREFIX);

			ret = os_library_open(&modules[count].dlhandle, filename);
			if(ret==0) {
				modules[count].entry = (MODULE_ENTRY)os_library_get(modules[count].dlhandle, "module_entry");
				if(modules[count].entry==NULL) {
					SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to find module_entry(%s)", filename);
				} else {
					modules[count].ret = ERR_UNKNOWN;
				}
			} else {
				SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to dlopen(%s), errno = %d, desc = %s", filename, ret, os_library_error());
			}
			if(ret!=0 || modules[count].entry==NULL) {
				if(ret==0) {
					os_library_close(modules[count].dlhandle);
				}
				for(; count>=0; count--) {
					os_library_close(modules[count].dlhandle);
				}
				return ERR_UNKNOWN;
			}
		}

		count++;
		if(count>=sizeof(modules)/sizeof(modules[0]) || tok==NULL) break;
		cur = tok + 1;
	}
	module_count = count;

	for(i=0; i<module_count; i++) {
		modules[i].ret = modules[i].entry(ENTRY_REASON_ATTACH);
		if(modules[i].ret!=ERR_NOERROR) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to init \"%s\" module, ret = %d", modules[i].name, modules[i].ret);
			return modules[i].ret;
		} else {
			DBGLOG(LOG_INFO, MODULE_NAME, "load \"%s\" module...done", modules[i].name);
		}
	}

	return ERR_NOERROR;
}

int appbox_unload_modules()
{
	int i, ret;

	for(i=module_count-1; i>=0; i--) {
		if(modules[i].ret!=ERR_NOERROR) continue;
		modules[i].ret = modules[i].entry(ENTRY_REASON_DETACH);
		if(modules[i].ret!=ERR_NOERROR) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to detach \"%s\" module, ret = %d", modules[i].name, modules[i].ret);
		} else {
			DBGLOG(LOG_INFO, MODULE_NAME, "unload \"%s\" module...done", modules[i].name);
		}
	}

	for(i=module_count-1; i>=0; i--) {
		ret = os_library_close(modules[i].dlhandle);
		if(ret!=0) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to unload \"%s\" module, ret = %d : %s", modules[i].name, modules[i].ret, os_library_error());
		}
	}

	return ERR_NOERROR;
}

int appbox_reg_command(const char* module, const char* name, CONSOLE_CALLBACK callback)
{
	if(strlen(module)+strlen(name)+1>CONSOLE_HOOKERNAME_LEN) return ERR_INVALID_PARAMETER;

	if(con_instance!=NULL) {
		char fullname[CONSOLE_HOOKERNAME_LEN+1];
		sprintf(fullname, "%s.%s", module, name);
		return console_hook(con_instance, fullname, callback);
	} else {
		return ERR_NOERROR;
	}
}

int appbox_unreg_command(const char* module, const char* name, CONSOLE_CALLBACK callback)
{
	if(strlen(module)+strlen(name)+1>CONSOLE_HOOKERNAME_LEN) return ERR_INVALID_PARAMETER;

	if(con_instance!=NULL) {
		char fullname[CONSOLE_HOOKERNAME_LEN+1];
		sprintf(fullname, "%s.%s", module, name);
		return console_unhook(con_instance, fullname, callback);
	} else {
		return ERR_NOERROR;
	}
}

const CONFIG_ITEM* get_config_item(const char* name)
{
	int i;

	for(i=0; i<config_count; i++) {
		if(strcmp(name, configs[i].name)==0) {
			return &configs[i];
		}
	}

	return NULL;
}

#ifdef APPBOX_MAIN

int appbox_main_load(const char* config_path)
{
	int ret;

	ret = appbox_config_load(config_path);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to appbox_config_load(%s), ret=%d", config_path, ret);
		return ret;
	}
	ret = appbox_config_init();
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to appbox_config_init(), ret=%d", ret);
		return ret;
	}

	return ERR_NOERROR;
}

int appbox_main_start()
{
	int ret;
	ret = appbox_init();
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to appbox_init(), ret=%d\n", ret);
		return ret;
	}
	ret = appbox_load_modules();
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to appbox_load_modules(), ret=%d\n", ret);
		ret = appbox_final();
		if(ret!=ERR_NOERROR) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to appbox_final(), ret=%d\n", ret);
		}
		return ret;
	}

	return ret;
}

int appbox_main_stop()
{
	int ret;

	ret = appbox_unload_modules();
	if(ret!=ERR_NOERROR) {
		printf("Failed to appbox_unload_modules(), ret=%d\n", ret);
	}
	ret = appbox_final();
	if(ret!=ERR_NOERROR) {
		printf("Failed to appbox_final(), ret=%d\n", ret);
	}

	return ERR_NOERROR;
}

#endif

int appbox_console_set(CONSOLE_CONNECTION* conn, const char* name, const char* line)
{
	const char* cur;
	char cname[100];
	char cvalue[100];

	cur = strget_space(line, cname, sizeof(cname));
	if(cur==NULL) {
		console_puts(conn, ERR_INVALID_PARAMETER, "invalid parameter");
		return ERR_NOERROR;
	}
	strncpy(cvalue, cur, sizeof(cvalue));
	strtrim(cvalue);
	if(cvalue[0]=='\0') {
		console_puts(conn, ERR_INVALID_PARAMETER, "invalid parameter");
		return ERR_NOERROR;
	}

	if(strcmp(cname, "syslog")==0) {
		if(strcmp(cvalue, "on")) {
			config_syslog_enable = 1;
		} else if(strcmp(cvalue, "off")) {
			config_syslog_enable = 0;
		} else {
			console_puts(conn, ERR_INVALID_PARAMETER, "invalid value");
			return ERR_NOERROR;
		}
		console_puts(conn, ERR_NOERROR, "done");
		return ERR_NOERROR;
	}

	if(strcmp(cname, "dbglog")==0) {
		if(strcmp(cvalue, "on")) {
			config_dbglog_enable = 1;
		} else if(strcmp(cvalue, "off")) {
			config_dbglog_enable = 0;
		} else {
			console_puts(conn, ERR_INVALID_PARAMETER, "invalid value");
			return ERR_NOERROR;
		}
		console_puts(conn, ERR_NOERROR, "done");
		return ERR_NOERROR;
	}

	console_puts(conn, ERR_INVALID_PARAMETER, "invalid name");
	return ERR_NOERROR;
}

int appbox_console_get(CONSOLE_CONNECTION* conn, const char* name, const char* line)
{
	const char* cur;
	char cname[100];
	char out[100];

	cur = strget_space(line, cname, sizeof(cname));
	if(cur==NULL) {
		console_puts(conn, ERR_INVALID_PARAMETER, "invalid parameter");
		return ERR_NOERROR;
	}

	if(strcmp(cname, "syslog")==0) {
		sprintf(out, "syslog %s", config_syslog_enable?"on":"off");
		console_puts(conn, ERR_NOERROR, out);
		return ERR_NOERROR;
	}

	if(strcmp(cname, "dbglog")==0) {
		sprintf(out, "dbglog %s", config_dbglog_enable?"on":"off");
		console_puts(conn, ERR_NOERROR, out);
		return ERR_NOERROR;
	}

	return ERR_UNKNOWN;
}

int appbox_console_threadpool(CONSOLE_CONNECTION* conn, const char* name, const char* line)
{
	if(strcmp(name, "threadpool_info")==0) {
		return ERR_NOERROR;
	}

	if(strcmp(name, "threadpool_status")==0) {
		THREADPOOL_STATUS status;
		threadpool_getstatus(&status);
		console_print(conn, ERR_NOERROR, "queue_size %d", status.queue_size);
		console_print(conn, ERR_NOERROR, "count %d", status.count);
		console_print(conn, ERR_NOERROR, "busy %d", status.busy);
		return ERR_NOERROR;
	}

	return ERR_UNKNOWN;
}

int appbox_console_coredump(CONSOLE_CONNECTION* conn, const char* name, const char* line)
{
	int* v = NULL;
	assert(0);
	*v = 100;
	return ERR_NOERROR;
}
