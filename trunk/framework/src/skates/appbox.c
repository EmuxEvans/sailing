#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "../../inc/skates/skates.h"

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
static int appbox_quit_flag = 0;
static os_thread_t cs_thread_id;
static CONSOLE_INSTANCE* con_instance = NULL;
static MODULE_INFO modules[MODULE_MAXCOUNT];
static int module_count;
static CONFIG_ITEM configs[1000];
static int config_count = 0;

static int config_serverid = -1;
static char config_appname[200] = "appname";
static char config_syslog[200] = "console://";
static char config_dbglog[200] = "console://";
static int  config_syslog_enable = 1;
static int	config_dbglog_enable = 1;
static int	config_worker_count = 5;
static int  config_network_downbuf_size = 1000;
static int	config_timer_interval = 100;
static char config_module_list[300] = "";
static SOCK_ADDR config_rpc_endpoint = { 0xffffffff, 0xffff };
static SOCK_ADDR config_cs_masterep = { 0xffffffff, 0xffff };
static SOCK_ADDR config_cs_endpoint = { 0xffffffff, 0xffff };
static unsigned int config_cs_maxconns = 10;
static unsigned int config_cs_maxhooker = 100;
static int config_dymempool_min = 0;
static int config_dymempool_max = 0;

APPBOX_SETTING_BEGIN(appbox_settings)
	APPBOX_SETTING_INTEGER("serverid", config_serverid)
	APPBOX_SETTING_STRING("appname", config_appname, sizeof(config_appname))
	APPBOX_SETTING_STRING("module_list", config_module_list, sizeof(config_module_list))
	APPBOX_SETTING_STRING("syslog.path", config_syslog, sizeof(config_syslog))
	APPBOX_SETTING_INTEGER("syslog.enable", config_syslog_enable)
	APPBOX_SETTING_STRING("dbglog.path", config_dbglog, sizeof(config_dbglog))
	APPBOX_SETTING_INTEGER("dbglog.enable", config_dbglog_enable)
	APPBOX_SETTING_INTEGER("threadpool.count", config_worker_count)
	APPBOX_SETTING_INTEGER("network.downbuf_size", config_network_downbuf_size)
	APPBOX_SETTING_INTEGER("timer.interval", config_timer_interval)
	APPBOX_SETTING_ENDPOINT("rpc.endpoint", config_rpc_endpoint)
	APPBOX_SETTING_ENDPOINT("console.master", config_cs_masterep)
	APPBOX_SETTING_ENDPOINT("console.endpoint", config_cs_endpoint)
	APPBOX_SETTING_INTEGER("console.maxconns", config_cs_maxconns)
	APPBOX_SETTING_INTEGER("console.maxhooker", config_cs_maxhooker)
	APPBOX_SETTING_INTEGER("dymempool.min", config_dymempool_min)
	APPBOX_SETTING_INTEGER("dymempool.max", config_dymempool_max)
APPBOX_SETTING_END(appbox_settings)

static int appbox_quit(int ret);
static const CONFIG_ITEM* get_config_item(const char* name);

static int appbox_console_set(CONSOLE_CONNECTION* conn, const char* name, const char* line);
static int appbox_console_get(CONSOLE_CONNECTION* conn, const char* name, const char* line);
static int appbox_console_threadpool(CONSOLE_CONNECTION* conn, const char* name, const char* line);
static int appbox_console_coredump(CONSOLE_CONNECTION* conn, const char* name, const char* line);
static unsigned int ZION_CALLBACK cs_thread_proc(void* arg);

int appbox_get_id()
{
	return config_serverid;
}

const char* appbox_get_name()
{
	return config_appname;
}

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
				if(strcmp(ttype, "AI")==0) type = CONFIGDATA_ARRAY_INTEGER;
				if(strcmp(ttype, "AF")==0) type = CONFIGDATA_ARRAY_FLOAT;
				if(strcmp(ttype, "AE")==0) type = CONFIGDATA_ARRAY_ENDPOINT;
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
			*tab[loopi].len = (unsigned int)strlen(item->value)/2;
			hex2bin(item->value, tab[loopi].ptr, *tab[loopi].len);
			break;						
		case CONFIGDATA_ENDPOINT:
			if(sock_str2addr(item->value, (SOCK_ADDR*)tab[loopi].ptr)==NULL) {
				SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to get config(%s), invalid data.", key);
				return ERR_UNKNOWN;
			}
			break;
		case CONFIGDATA_ARRAY_INTEGER:
			{
				unsigned int i;
				const char* next = item->value;
				for(i=0; i<tab[loopi].maxlen; i++) {
					next = strget2int_space(next, (int*)tab[loopi].ptr);
					if(!next) break;
				}
				*tab[loopi].len = i;
			}
			break;
		case CONFIGDATA_ARRAY_FLOAT:
			{
				unsigned int i;
				const char* next = item->value;
				for(i=0; i<tab[loopi].maxlen; i++) {
					next = strget2float_space(next, (float*)tab[loopi].ptr);
					if(!next) break;
				}
				*tab[loopi].len = i;
			}
			break;
		case CONFIGDATA_ARRAY_ENDPOINT:
			{
				unsigned int i;
				const char* next = item->value;
				char value[100];
				for(i=0; i<tab[loopi].maxlen; i++) {
					next = strget_space(next, value, sizeof(value));
					if(!next) break;
					if(!sock_str2addr(value, (SOCK_ADDR*)tab[loopi].ptr + i)) break;
				}
				*tab[loopi].len = i;
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
	SYSLOG(LOG_INFO, MODULE_NAME, "** SYSLOG START **");
	DBGLOG(LOG_INFO, MODULE_NAME, "** DBGLOG START **");

	SYSLOG(LOG_INFO, MODULE_NAME, "APPNAME=%s", config_appname);
	SYSLOG(LOG_INFO, MODULE_NAME, "PROCESS=%d", os_process_getid());

	mempool_init();
	sock_init();
	ret = fdwatch_init();
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_init() fail, ret=%d", ret);
		return appbox_quit(ERR_UNKNOWN);
	}

	if(config_cs_endpoint.ip!=0xffffffff && config_cs_endpoint.port!=0xffff) {
		con_instance = console_create(&config_cs_endpoint, config_cs_maxconns, config_cs_maxhooker);
		if(con_instance==NULL) {
			char ep[40];
			sock_addr2str(&config_cs_endpoint, ep);
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

	ret = os_thread_begin(&cs_thread_id, cs_thread_proc, NULL);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to os_thread_begin(cs_thread_proc), ret=%d", ret);
		return appbox_quit(ERR_UNKNOWN);
	}

	network_init(config_network_downbuf_size);

	ret = dymempool_init(config_dymempool_min, config_dymempool_max);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "dymempool_init() fail, ret=%d", ret);
		return appbox_quit(ERR_UNKNOWN);
	}
	ret = dbapi_init(NULL, 10*1024);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "dbapi_init() fail, ret=%d", ret);
		return appbox_quit(ERR_UNKNOWN);
	}
	ret = threadpool_init(config_worker_count);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "threadpool_init() fail, ret=%d", ret);
		return appbox_quit(ERR_UNKNOWN);
	}
	ret = timer_init(config_timer_interval);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "timer_init() fail, ret=%d", ret);
		return appbox_quit(ERR_UNKNOWN);
	}
	ret = rpcnet_init();
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_init() fail, ret=%d", ret);
		return appbox_quit(ERR_UNKNOWN);
	}
	ret = rpcfun_init();
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "rpcfun_init() fail, ret=%d", ret);
		return appbox_quit(ERR_UNKNOWN);
	}
	if(config_rpc_endpoint.ip!=0xffffffff && config_rpc_endpoint.port!=0xffff) {
		ret = rpcnet_bind(&config_rpc_endpoint);
		if(ret!=ERR_NOERROR) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_bind() fail, ret=%d", ret);
			return appbox_quit(ERR_UNKNOWN);
		}
	}

	return ERR_NOERROR;
}

int appbox_final()
{
	int ret;

	appbox_quit_flag = 1;

	if(config_rpc_endpoint.ip!=0xffffffff && config_rpc_endpoint.port!=0xffff) {
		ret = rpcnet_unbind();
		if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_unbind() fail, ret=%d", ret);
	}
	ret = rpcfun_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "rpcfun_final() fail, ret=%d", ret);
	ret = rpcnet_shutdown();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_shutdown() fail, ret=%d", ret);
	ret = rpcnet_final();
	network_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "rpcnet_final() fail, ret=%d", ret);
	ret = timer_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "timer_final() fail, ret=%d", ret);
	ret = threadpool_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "threadpool_final() fail, ret=%d", ret);
	ret = dbapi_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "dbapi_final() fail, ret=%d", ret);
	ret = dymempool_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "dymempool_final() fail, ret=%d", ret);

	ret = os_thread_wait(cs_thread_id, NULL);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to os_thread_begin(cs_thread_proc), ret=%d", ret);
		return appbox_quit(ERR_UNKNOWN);
	}

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
		if(ret!=ERR_NOERROR) {
			SYSLOG(LOG_ERROR, MODULE_NAME, "console_destroy() fail, ret=%d", ret);
		}
	}

	ret = fdwatch_final();
	if(ret!=ERR_NOERROR) SYSLOG(LOG_ERROR, MODULE_NAME, "fdwatch_final() fail, ret=%d", ret);
	sock_final();
	mempool_final();

	SYSLOG(LOG_INFO, MODULE_NAME, "** SYSLOG END **");
	DBGLOG(LOG_INFO, MODULE_NAME, "** DBGLOG END **");
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
					modules[count].ret = ERR_NOERROR;
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
	if(strcmp(name, MODULE_NAME".threadpool_info")==0) {
		return ERR_NOERROR;
	}

	if(strcmp(name, MODULE_NAME".threadpool_status")==0) {
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

unsigned int ZION_CALLBACK cs_thread_proc(void* arg)
{
	SOCK_HANDLE sock;
	int count, ret, c_try;

	sock = SOCK_INVALID_HANDLE;
	c_try = count = 0;
	while(!appbox_quit_flag) {
		if(count>0) {
			count--;
			os_sleep(100);
			continue;
		}

		if(sock==SOCK_INVALID_HANDLE && config_cs_masterep.ip!=0 && config_cs_masterep.port!=0) {
			sock = sock_connect(&config_cs_masterep, 0);
			if(sock!=SOCK_INVALID_HANDLE) {
				SYSLOG(LOG_INFO, MODULE_NAME, "cs_master connected");
				c_try = 0;
			} else {
				if(c_try==0) {
					SYSLOG(LOG_INFO, MODULE_NAME, "Failed to connect(cs_master)");
					c_try = 1;
				}
			}
		}

		if(sock!=SOCK_INVALID_HANDLE) {
			char line[100];
			sprintf(line, "csmng.register %d %s", appbox_get_id(), appbox_get_name());
			sock_addr2str(&config_cs_endpoint, line+strlen(line));
			ret = sock_writeline(sock, "ping");
			if(ret==ERR_NOERROR) {
				for(;;) {
					ret = sock_readline(sock, line, sizeof(line));
					if(ret!=ERR_NOERROR)
						break;
					if(line[0]=='\0') break;
				}
				
			}
			if(ret!=ERR_NOERROR) {
				sock_close(sock);
				sock = SOCK_INVALID_HANDLE;
			} else {
				SYSLOG(LOG_INFO, MODULE_NAME, "cs_master disconnected");
			}
		}

		count = 50;
	}

	if(sock!=SOCK_INVALID_HANDLE) {
		sock_disconnect(sock);
		sock_close(sock);
		SYSLOG(LOG_INFO, MODULE_NAME, "cs_master disconnected, for quit");
	}

	return 0;
}
