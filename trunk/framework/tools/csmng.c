#include <stdio.h>

#include "../inc/skates.h"
#include "../inc/appbox_svc.h"
#include "../inc/appbox_args.h"

static char mapfile[100] = "";
static SOCK_ADDR listen_ep;
static CONSOLE_INSTANCE* instance;

static int csmng_init();
static int csmng_final();
static int csmng_start();
static int csmng_stop();
static int csmng_usage();

int main(int argc, char* argv[])
{
	APPBOX_SERVICE svc = {
		"csmng",
		"Console Manager",
		csmng_init,
		csmng_final,
		csmng_start,
		csmng_stop,
		csmng_usage
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

	return ERR_NOERROR;
}

int csmng_final()
{
	fdwatch_final();
	sock_final();
	return ERR_NOERROR;
}

int csmng_start()
{
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