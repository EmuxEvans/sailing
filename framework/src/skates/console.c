#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/sock.h"
#include "../../inc/skates/console.h"
#include "../../inc/skates/rlist.h"
#include "../../inc/skates/fdwatch.h"
#include "../../inc/skates/misc.h"

#define CONSOLE_SERVER_COUNT	100

struct CONSOLE_CONNECTION {
	SOCK_HANDLE			sock;
	FDWATCH_ITEM		fdi;
	CONSOLE_INSTANCE*	instance;
	SOCK_ADDR			sa;
	char				sa_str[40];
};

typedef struct CONSOLE_HOOKER {
	char name[CONSOLE_HOOKERNAME_LEN+1];
	CONSOLE_CALLBACK func;
} CONSOLE_HOOKER;

typedef struct CONSOLE_MAP_ITEM {
	char	name[CONSOLE_ITEM_NAME_LEN+1];
	char	addr[CONSOLE_ITEM_ADDR_LEN+1];
} CONSOLE_MAP_ITEM;

typedef struct CONSOLE_SERVER {
	char				name[100];
	char				ep[100];
	CONSOLE_CONNECTION*	conn;
} CONSOLE_SERVER;

struct CONSOLE_INSTANCE {
	SOCK_HANDLE				sock;
	FDWATCH_ITEM			fdi;
	SOCK_ADDR				sa;
	FDWATCH_HANDLE			fdh;
	os_mutex_t				mtx;
	os_thread_t				th;
	char					config_path[200];
	unsigned int			maxconns;
	unsigned int			maxhooker;
	unsigned int			maxitems;
	CONSOLE_CONNECTION*		conns;
	CONSOLE_HOOKER*			hookers;
	CONSOLE_MAP_ITEM*		items;
	CONSOLE_SERVER*			servers;
};

static CONSOLE_CALLBACK console_global_hook = NULL;

static int load_config(CONSOLE_INSTANCE* instance, const char* config_file);
static int save_config(CONSOLE_INSTANCE* instance, const char* config_file);
static int insert_item(CONSOLE_INSTANCE* instance, const char* name, const char* addr);
static int remove_item(CONSOLE_INSTANCE* instance, const char* name);

static unsigned int ZION_CALLBACK console_thread(void* arg);
static void accept_func(FDWATCH_ITEM* item, int events);
static void conn_func(FDWATCH_ITEM* item, int events);

static int csmng_func(CONSOLE_CONNECTION* conn, const char* name, const char* line);
static int remote_func(CONSOLE_CONNECTION* conn, const char* line);

CONSOLE_INSTANCE* console_create(SOCK_ADDR* sa, unsigned int maxconns, unsigned int maxhooker)
{
	return console_create_csmng(sa, maxconns, maxhooker, 0, NULL);
}

CONSOLE_INSTANCE* console_create_csmng(SOCK_ADDR* sa, unsigned int maxconns, unsigned int maxhooker, unsigned int maxitems, const char* config_path)
{
	SOCK_HANDLE sock;
	CONSOLE_INSTANCE* instance;
	unsigned int i;
	unsigned int maxserver;

	maxserver = (maxitems>0?CONSOLE_SERVER_COUNT:0);

	sock = sock_bind(sa, SOCK_REUSEADDR);
	if(sock==SOCK_INVALID_HANDLE) return NULL;

	instance = (CONSOLE_INSTANCE*)malloc(sizeof(CONSOLE_INSTANCE)+sizeof(CONSOLE_CONNECTION)*maxconns+sizeof(CONSOLE_HOOKER)*maxhooker+sizeof(CONSOLE_MAP_ITEM)*maxitems+sizeof(CONSOLE_SERVER)*maxserver);
	if(instance==NULL) {
		sock_unbind(sock);
		return NULL;
	}

	memset(instance, 0, sizeof(CONSOLE_INSTANCE)+sizeof(CONSOLE_CONNECTION)*maxconns+sizeof(CONSOLE_HOOKER)*maxhooker+sizeof(CONSOLE_MAP_ITEM)*maxitems+sizeof(CONSOLE_SERVER)*maxserver);
	instance->sock		= sock;
	memcpy(&instance->sa, sa, sizeof(SOCK_ADDR));
	instance->maxconns	= maxconns;
	instance->maxhooker	= maxhooker;
	instance->maxitems	= maxitems;
	instance->conns		= (CONSOLE_CONNECTION*)(instance+1);
	instance->hookers	= (CONSOLE_HOOKER*)(instance->conns+maxconns);
	if(maxhooker==0) {
		instance->items		= NULL;
		instance->servers	= NULL;
	} else {
		instance->items		= (CONSOLE_MAP_ITEM*)(instance->hookers+maxhooker);
		instance->servers	= (CONSOLE_SERVER*)(instance->items+maxitems);
	}
	os_mutex_init(&instance->mtx);
	for(i=0; i<instance->maxconns; i++) instance->conns[i].sock = SOCK_INVALID_HANDLE;

	if(instance->maxitems>0) {
		if(config_path!=NULL) {
			if(load_config(instance, config_path)!=ERR_NOERROR) {
				printf("Failed to load_config(\"%s\")\n", config_path);
			}
			strcpy(instance->config_path, config_path);
		}

		console_hook(instance, "csmng.table.list", csmng_func);
		console_hook(instance, "csmng.table.add", csmng_func);
		console_hook(instance, "csmng.table.delete", csmng_func);
		console_hook(instance, "csmng.table.clear", csmng_func);
		console_hook(instance, "csmng.table.load", csmng_func);
		console_hook(instance, "csmng.table.save", csmng_func);
		console_hook(instance, "csmng.servers.register", csmng_func);
		console_hook(instance, "csmng.servers.list", csmng_func);
	}

	instance->fdh = fdwatch_create();
	if(instance->fdh==NULL) {
		sock_unbind(sock);
		free(instance);
		return NULL;
	}

	fdwatch_set(&instance->fdi, instance->sock, FDWATCH_READ, accept_func, instance);
	if(fdwatch_add(instance->fdh, &instance->fdi)!=ERR_NOERROR) {
		os_mutex_destroy(&instance->mtx);
		fdwatch_destroy(instance->fdh);
		sock_unbind(instance->sock);
		free(instance);
		return NULL;
	}

	if(os_thread_begin(&instance->th, console_thread, instance)!=0) {
		fdwatch_remove(instance->fdh, &instance->fdi);
		os_mutex_destroy(&instance->mtx);
		fdwatch_destroy(instance->fdh);
		sock_unbind(instance->sock);
		free(instance);
		return NULL;
	}

	return instance;
}

int console_destroy(CONSOLE_INSTANCE* instance)
{
	unsigned int i;

	save_config(instance, instance->config_path);

	fdwatch_break(instance->fdh);
	os_thread_wait(instance->th, NULL);

	//close all connection
	for(i=0; i<instance->maxconns; i++) {
		if(instance->conns[i].sock==SOCK_INVALID_HANDLE) continue;
		fdwatch_remove(instance->fdh, &instance->conns[i].fdi);
		sock_disconnect(instance->conns[i].sock);
		sock_close(instance->conns[i].sock);
	}

	os_thread_close(instance->th);
	fdwatch_remove(instance->fdh, &instance->fdi);
	os_mutex_destroy(&instance->mtx);
	fdwatch_destroy(instance->fdh);
	sock_unbind(instance->sock);
	free(instance);
	return ERR_NOERROR;
}

int console_hook(CONSOLE_INSTANCE* instance, const char* name, CONSOLE_CALLBACK func)
{
	unsigned int i;

	if(name==NULL) {
		console_global_hook = func;
		return ERR_NOERROR;
	}

	if(strlen(name)>CONSOLE_HOOKERNAME_LEN) return ERR_INVALID_PARAMETER;
	if(func==NULL) return ERR_INVALID_PARAMETER;

	os_mutex_lock(&instance->mtx);
	for(i=0; i<instance->maxhooker; i++) {
		if(instance->hookers[i].name[0]=='\0') {
			strcpy(instance->hookers[i].name, name);
			instance->hookers[i].func = func;
			break;
		}
	}
	os_mutex_unlock(&instance->mtx);

	return i==instance->maxhooker?ERR_FULL:ERR_NOERROR;
}

int console_unhook(CONSOLE_INSTANCE* instance, const char* name, CONSOLE_CALLBACK func)
{
	unsigned int i;

	if(console_global_hook==func) {
		console_global_hook = NULL;
		return ERR_NOERROR;
	}

	if(strlen(name)>CONSOLE_HOOKERNAME_LEN) return ERR_INVALID_PARAMETER;
	if(func==NULL) return ERR_INVALID_PARAMETER;

	os_mutex_lock(&instance->mtx);
	for(i=0; i<instance->maxhooker; i++) {
		if(strcmp(name, instance->hookers[i].name)==0) {
			memset(instance->hookers[i].name, 0, sizeof(instance->hookers[i].name));
			instance->hookers[i].func = NULL;
			break;
		}
	}
	os_mutex_unlock(&instance->mtx);

	return i==instance->maxhooker?ERR_NOT_FOUND:ERR_NOERROR;
}

int console_puts(CONSOLE_CONNECTION* conn, int code, const char* str)
{
	int ret;
	char code_str[15];
	sprintf(code_str, "%d ", code);
	ret = sock_writebuf(conn->sock, code_str, (int)strlen(code_str));
	if(ret!=ERR_NOERROR) return ret;
	return sock_writeline(conn->sock, str);
}

int console_print(CONSOLE_CONNECTION* conn, int code, const char* fmt, ...)
{
	char buf[2*1024];
	int count;
	va_list argptr;

	sprintf(buf, "%d ", code);
	count = strlen(buf);

	va_start(argptr, fmt);
	count = vsnprintf(buf+count, sizeof(buf)-count, fmt, argptr);
	va_end(argptr); 

	return sock_writeline(conn->sock, buf);
}

const SOCK_ADDR* console_peername(CONSOLE_CONNECTION* conn)
{
	return &conn->sa;
}

const char* console_peername_str(CONSOLE_CONNECTION* conn)
{
	return conn->sa_str;
}

unsigned int ZION_CALLBACK console_thread(void* arg)
{
	CONSOLE_INSTANCE* instance = (CONSOLE_INSTANCE*)arg;
	fdwatch_loop(instance->fdh);
	return 0;
}

static void accept_func(FDWATCH_ITEM* item, int events)
{
	CONSOLE_INSTANCE* instance;
	SOCK_HANDLE sock;
	unsigned int i;
	int ret;
	SOCK_ADDR sa;

	instance = (CONSOLE_INSTANCE*)fdwatch_getptr(item);
	sock = sock_accept(instance->sock, &sa);
	if(sock==SOCK_INVALID_HANDLE) return;

	for(i=0; i<instance->maxconns; i++) {
		if(instance->conns[i].sock==SOCK_INVALID_HANDLE) break;
	}
	if(i==instance->maxconns) {
		char line[100];
		sprintf(line, "%d full", -ERR_FULL);
		sock_writeline(sock, line);
		sock_writeline(sock, "");
		sock_disconnect(sock);
		sock_close(sock);
		return;
	}

	fdwatch_set(&instance->conns[i].fdi, sock, FDWATCH_READ, conn_func, &instance->conns[i]);
	ret = fdwatch_add(instance->fdh, &instance->conns[i].fdi);
	if(ret!=ERR_NOERROR) {
		char line[100];
		sprintf(line, "%d Failed to fdwatch_add", -ret);
		instance->conns[i].sock = SOCK_INVALID_HANDLE;
		sock_writeline(sock, line);
		sock_writeline(sock, "");
		sock_disconnect(sock);
		sock_close(sock);
		return;
	}

	instance->conns[i].sock = sock;
	instance->conns[i].instance = instance;
	memcpy(&instance->conns[i].sa, &sa, sizeof(sa));
	sock_addr2str(&sa, instance->conns[i].sa_str);
}

static void conn_func(FDWATCH_ITEM* item, int events)
{
	CONSOLE_CONNECTION* conn;
	char line[300];
	char name[CONSOLE_HOOKERNAME_LEN+1];
	const char* end;
	unsigned int i;
	int ret;

	conn = (CONSOLE_CONNECTION*)fdwatch_getptr(item);

	ret = sock_readline(conn->sock, line, sizeof(line));
	if(ret!=ERR_NOERROR) {

		if(conn->instance->maxitems>0) {
			for(i=0; i<CONSOLE_SERVER_COUNT; i++) {
				if(conn->instance->servers[i].conn==conn) {
					memset(&conn->instance->servers[i], 0, sizeof(conn->instance->servers[i]));
				}
			}
		}

		fdwatch_remove(conn->instance->fdh, &conn->fdi);
		sock_close(conn->sock);
		conn->sock = SOCK_INVALID_HANDLE;
		conn->instance = NULL;
		return;
	}

	end = strget_space(line, name, sizeof(name));
	if(end==NULL) return;
	strltrim(name);

	if(console_global_hook) {
		console_global_hook(conn, NULL, line);
	}

	if(conn->instance->maxitems>0 && (strcmp(name, "csmng.callremote")==0 || strcmp(name, "@")==0)) {
		if(remote_func(conn, end)!=ERR_NOERROR) {
			fdwatch_remove(conn->instance->fdh, &conn->fdi);
			sock_close(conn->sock);
			conn->sock = SOCK_INVALID_HANDLE;
			conn->instance = NULL;
			return;
		}
	}

	os_mutex_lock(&conn->instance->mtx);

	for(i=0; i<conn->instance->maxhooker; i++) {
		if(strcmp(name, conn->instance->hookers[i].name)==0) break;
	}
	if(i==conn->instance->maxhooker) {
		os_mutex_unlock(&conn->instance->mtx);
		sprintf(line, "%d command \"%s\" not found", -ERR_NOT_FOUND, name);
		sock_writeline(conn->sock, line);
		sock_writeline(conn->sock, "");
		return;
	}

	if(conn->instance->hookers[i].func(conn, name, end)!=ERR_NOERROR) {
		os_mutex_unlock(&conn->instance->mtx);
		fdwatch_remove(conn->instance->fdh, &conn->fdi);
		sock_close(conn->sock);
		conn->sock = SOCK_INVALID_HANDLE;
		conn->instance = NULL;
		return;
	}

	os_mutex_unlock(&conn->instance->mtx);

	sock_writeline(conn->sock, "");
}

int csmng_func(CONSOLE_CONNECTION* conn, const char* name, const char* line)
{
	int ret;
	char buf[1000];

	if(strcmp(name, "csmng.table.list")==0) {
		unsigned int i, count;

		for(i=count=0; i<conn->instance->maxitems; i++) {
			if(conn->instance->items[i].name[0]=='\0') continue;
			sprintf(buf, "name=%s addr=%s", conn->instance->items[i].name, conn->instance->items[i].addr);
			ret = console_puts(conn, ERR_NOERROR, buf);
			if(ret!=ERR_NOERROR) return ret;
			count++;
		}
		if(count==0) {
			return console_puts(conn, ERR_NO_DATA, "the list is empty");
		}
		return ERR_NOERROR;
	} else if(strcmp(name, "csmng.table.add")==0) {
		char name[CONSOLE_ITEM_NAME_LEN+1];
		char addr[CONSOLE_ITEM_ADDR_LEN+1];
		SOCK_ADDR sa;
		const char* buf;

		buf = strget_space(line, name, sizeof(name));
		if(buf==NULL)
			return console_puts(conn, ERR_INVALID_PARAMETER, "invalid parameter");
		buf = strget_space(buf, addr, sizeof(addr));
		if(buf==NULL)
			return console_puts(conn, ERR_INVALID_PARAMETER, "invalid parameter");
		if(!sock_str2addr(addr, &sa))
			return console_puts(conn, ERR_INVALID_PARAMETER, "invalid parameter");

		return console_puts(conn, insert_item(conn->instance, name, addr), "");
	} else if( strcmp(name, "csmng.table.delete")==0 ) {
		char name[CONSOLE_ITEM_NAME_LEN+1];
		const char* buf;

		buf = strget_space(line, name, sizeof(name));
		if(buf==NULL) return console_puts(conn, ERR_INVALID_PARAMETER, "invalid parameter");

		return console_puts(conn, remove_item(conn->instance, name), "");
	} else if(strcmp(name, "csmng.table.clear")==0) {
		memset(conn->instance->items, 0, sizeof(*conn->instance->items)*conn->instance->maxitems);
		return console_puts(conn, ERR_NOERROR, "execute sucess");
	} else if(strcmp(name, "csmng.table.save")==0) {
		int ret;
		if(conn->instance->config_path[0]=='\0') {
			ret = save_config(conn->instance, conn->instance->config_path);
		} else {
			ret = ERR_UNKNOWN;
		}
		return console_puts(conn, ret, "");
	} else if(strcmp(name, "csmng.table.load")==0) {
		int ret;
		if(conn->instance->config_path[0]=='\0') {
			ret = load_config(conn->instance, conn->instance->config_path);
		} else {
			ret = ERR_UNKNOWN;
		}
		return console_puts(conn, ret, "");
	} else if(strcmp(name, "csmng.servers.register")==0) {
		int i;
		char name[100];
		char ep[100];
		const char* t;

		if((t=strget_space(line, name, sizeof(name)))==NULL || (t=strget_space(line, ep, sizeof(ep)))==NULL) {
			return console_puts(conn, ERR_INVALID_PARAMETER, "invalid parameter");
		}

		for(i=0; i<CONSOLE_SERVER_COUNT; i++) {
			if(strcmp(conn->instance->servers[i].name, name)==0) {
				strcpy(conn->instance->servers[i].name, name);
				strcpy(conn->instance->servers[i].ep, ep);
				conn->instance->servers[i].conn = conn;
				break;
			}
		}
		if(i==CONSOLE_SERVER_COUNT) {
			for(i=0; i<CONSOLE_SERVER_COUNT; i++) {
				if(strcmp(conn->instance->servers[i].name, "")==0) {
					strcpy(conn->instance->servers[i].name, name);
					strcpy(conn->instance->servers[i].ep, ep);
					conn->instance->servers[i].conn = conn;
					break;
				}
			}
		}

		return console_puts(conn, ERR_NOERROR, "");
	} else if(strcmp(name, "csmng.servers.list")==0) {
		int i, count = 0;
		for(i=0; i<CONSOLE_SERVER_COUNT; i++) {
			if(conn->instance->servers[i].name[0]!='\0') {
				sprintf(buf, "%s %s", conn->instance->servers[i].name, conn->instance->servers[i].ep);
				console_puts(conn, ERR_NOERROR, buf);
				count++;
			}
		}
		if(count==0) {
			console_puts(conn, ERR_NOT_FOUND, "not found");
		}
	} else {
		sprintf(buf, "the command \"%s\" not found", name);
		return console_puts(conn, ERR_UNKNOWN, buf);
	} 

	return ERR_NOERROR;
}

int remote_func(CONSOLE_CONNECTION* conn, const char* line)
{
	char name[CONSOLE_ITEM_NAME_LEN+1];
	const char* end;
	unsigned int i;
	SOCK_ADDR addr;
	SOCK_HANDLE fd;
	int ret;
	char buf[1000];

	end = strget_space(line, name, sizeof(name));
	if(end==NULL) return console_puts(conn, ERR_INVALID_PARAMETER, "");

	for(i=0; i<conn->instance->maxitems; i++) {
		if(strcmp(name, conn->instance->items[i].name)==0) break;
	}
	if(i==conn->instance->maxitems) {
		return console_puts(conn, ERR_NOT_FOUND, "");
	}

	sock_str2addr(conn->instance->items[i].addr, &addr);
	fd = sock_connect(&addr, SOCK_WAIT);
	if(fd!=SOCK_INVALID_HANDLE) {
		return console_puts(conn, ERR_UNKNOWN, "can't connect to the server");
	}

	ret = sock_writeline(fd, end);
	if(ret!=ERR_NOERROR) return ret;

	for(;;) {
		ret = sock_readline(fd, buf, sizeof(buf));
		if(ret!=ERR_NOERROR) return ret;
		ret = sock_writeline(conn->sock, buf);
		if(ret!=ERR_NOERROR) return ret;
		ret = console_puts(conn, ERR_NOERROR, buf);

		if(strcmp(buf, "")==0) break;
	}
	
	sock_disconnect(fd);
	sock_close(fd);
	return ERR_NOERROR;
}

int load_config(CONSOLE_INSTANCE* instance, const char* config_file)
{
	FILE* fp;
	unsigned int i;

	fp = fopen(config_file, "rt");
	if(fp==NULL) return ERR_UNKNOWN;

	for(i=0; i<instance->maxitems; i++) {
		if(fscanf(fp, "%s %s", instance->items[i].name, instance->items[i].addr)!=3) {
			break;
		}
	}

	if(!feof(fp)) {
		fclose(fp);
		return ERR_UNKNOWN;
	}

	fclose(fp);
	return ERR_NOERROR;
}

int save_config(CONSOLE_INSTANCE* instance, const char* config_file)
{
	FILE* fp;
	unsigned int i;

	fp = fopen(config_file, "wt");
	if(fp==NULL) return ERR_UNKNOWN;

	for(i=0; i<instance->maxitems; i++) {
		if(instance->items[i].name=='\0') continue;
		fprintf(fp, "%s %s\n", instance->items[i].name, instance->items[i].addr);
	}

	fclose(fp);
	return ERR_NOERROR;
}

int insert_item(CONSOLE_INSTANCE* instance, const char* name, const char* addr)
{
	unsigned int i;

	for(i=0; i<instance->maxitems; i++) {
		if(strcmp(instance->items[i].name, name)==0) return ERR_EXISTED;
	}

	for(i=0; i<instance->maxitems; i++) {
		if(instance->items[i].name[0]=='\0') break;
	}
	if(i==instance->maxitems) return ERR_FULL;

	strcpy(instance->items[i].name, name);
	strcpy(instance->items[i].addr, addr);

	return ERR_NOERROR;
}

int remove_item(CONSOLE_INSTANCE* instance, const char* name)
{
	unsigned int i;

	for(i=0; i<instance->maxitems; i++) {
		if(strcmp(instance->items[i].name, name)==0) {
			memset(&instance->items[i], 0, sizeof(instance->items[i]));
			return ERR_NOERROR;
		}
	}

	return ERR_NOT_FOUND;
}
