#include <stdio.h>
#include <skates/skates.h>
#include <skates/appbox_svc.h>
#include <skates/appbox_args.h>

#include "streamserver.h"

#include <stdio.h>
#include <string.h>

static SOCK_ADDR listen_ep;
char log_path[200];
LOG_HANDLE log = NULL;

static int app_init();
static int app_final();
static int app_start();
static int app_stop();
static int app_usage();

int main(int argc, char* argv[])
{
	APPBOX_SERVICE svc = {
		"stream server",
		app_init,
		app_final,
		app_start,
		app_stop,
		app_usage,
	};

	return appbox_service(&svc, argc-1, &argv[1]);
}

int app_init()
{
	sock_init();
	fdwatch_init();
	mempool_init();
	threadpool_init(10);
	network_init(1024);
	streamserver_init();

	if(!sock_str2addr(appbox_args_get("listen", "0.0.0.0:1980"), &listen_ep))
		return ERR_INVALID_PARAMETER;

	strcpy(log_path, appbox_args_get("log", "file://single@streamserver.log"));
	log = log_open(log_path);
	if(log==NULL) {
		printf("can't open log file");
	}

	return ERR_NOERROR;
}

int app_final()
{
	if(log) {
		log_close(log);
		log = NULL;
	}

	streamserver_final();
	network_final();
	threadpool_final();
	mempool_final();
	fdwatch_final();
	sock_final();
	return ERR_NOERROR;
}

int app_start()
{
	return streamserver_start(&listen_ep);
}

int app_stop()
{
	return streamserver_stop(&listen_ep);
}

int app_usage()
{
	printf("usage :\n");
	printf("	appbox_main.exe install [-servicename=name] [-servicedesc=desc] [-cmdline=line]\n");
	printf("	appbox_main.exe uninstall [-servicename=name]\n");
	printf("	appbox_main.exe service [-config=file]\n");
	printf("	appbox_main.exe debug [-config=file]\n");
	printf("\n");

	return ERR_NOERROR;
}

int app_monitor(CONSOLE_CONNECTION* conn, const char* name, const char* line)
{
	if(log) {
		log_write(log, LOG_INFO, "%15s=>%s", console_peername_str(conn), line);
	}

	return ERR_NOERROR;
}
