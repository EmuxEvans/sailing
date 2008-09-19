#include <stdio.h>

#include "../inc/skates/skates.h"
#include "../inc/skates/appbox_svc.h"
#include "../inc/skates/appbox_args.h"

static char mapfile[100] = "";
static char histroyfile[100] = "";
static SOCK_ADDR listen_ep;
static CONSOLE_INSTANCE* instance;
LOG_HANDLE console_log = NULL;

static int csmng_init();
static int csmng_final();
static int csmng_start();
static int csmng_stop();
static int csmng_usage();
static int csmng_monitor(CONSOLE_CONNECTION* conn, const char* name, const char* line);

int main(int argc, char* argv[])
{
	APPBOX_SERVICE svc = {
		"csmng",
		csmng_init,
		csmng_final,
		csmng_start,
		csmng_stop,
		csmng_usage,
	};

	return appbox_service(&svc, argc, argv);
}

int csmng_init()
{
	sock_init();
	fdwatch_init();

	if(!sock_str2addr(appbox_args_get("listen", "0.0.0.0:1980"), &listen_ep))
		return ERR_INVALID_PARAMETER;

	strcpy(mapfile, appbox_args_get("map", "csmng.map"));
	strcpy(histroyfile, appbox_args_get("histroy", "file://single@csmng.histroy"));
	console_log = log_open(histroyfile);
	if(console_log==NULL) {
		printf("can't open histroy file");
	}

	return ERR_NOERROR;
}

int csmng_final()
{
	if(console_log) {
		log_close(console_log);
		console_log = NULL;
	}

	fdwatch_final();
	sock_final();
	return ERR_NOERROR;
}

int csmng_start()
{
	console_hook(NULL, NULL, csmng_monitor);

	instance = console_create_csmng(&listen_ep, 100, 20, 1000, mapfile);
	if(instance==NULL) {
		printf("Failed to console_create_csmng()\n");
		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

int csmng_stop()
{
	console_destroy(instance);
	console_unhook(NULL, NULL, csmng_monitor);
	return ERR_NOERROR;
}

int csmng_usage()
{
	printf("usage :\n");
	printf("	appbox_main.exe install [-servicename=name] [-servicedesc=desc] [-cmdline=line]\n");
	printf("	appbox_main.exe uninstall [-servicename=name]\n");
	printf("	appbox_main.exe service [-config=file]\n");
	printf("	appbox_main.exe debug [-config=file]\n");
	printf("\n");

	return ERR_NOERROR;
}

int csmng_monitor(CONSOLE_CONNECTION* conn, const char* name, const char* line)
{
	if(console_log) {
		log_write(console_log, LOG_INFO, "%15s=>%s", console_peername_str(conn), line);
	}

	return ERR_NOERROR;
}
