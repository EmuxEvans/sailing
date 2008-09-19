#include <stdio.h>
#include <stdlib.h>

#include "../../inc/skates/skates.h"
#include "../../inc/skates/appbox_args.h"
#include "../../inc/skates/appbox_svc.h"

static int appbox_main_init();
static int appbox_main_final();
static int appbox_main_start();
static int appbox_main_stop();
static int appbox_main_usage();

int main(int argc, char* argv[])
{
	APPBOX_SERVICE svc = {
		"appbox",
		appbox_main_init,
		appbox_main_final,
		appbox_main_start,
		appbox_main_stop,
		appbox_main_usage,
	};

	return appbox_service(&svc, argc, argv);
}

int appbox_main_init()
{
	int ret;
	const char* config_path;

	config_path = appbox_args_get("config", "../conf/appbox.conf");
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

int appbox_main_final()
{
	return ERR_NOERROR;
}

int appbox_main_start()
{
	int ret;
	ret = appbox_init();
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to appbox_init(), ret=%d", ret);
		return ret;
	}
	ret = appbox_load_modules();
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to appbox_load_modules(), ret=%d", ret);
		appbox_final();
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

int appbox_main_usage()
{
	printf("usage :\n");
	printf("\tappbox_main.exe install [-servicename=name] [-servicedesc=desc] [-cmdline=line]\n");
	printf("\tappbox_main.exe uninstall [-servicename=name]\n");
	printf("\tappbox_main.exe service [-servicename=name] [-config=file]\n");
	printf("\tappbox_main.exe debug [-config=fible]\n");
	printf("\n");
	return ERR_NOERROR;
}
