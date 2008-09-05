#include <skates/skates.h>
#include <sailing/pool.hpp>

#include "module_main.h"
#include "authsvr.hpp"

static SOCK_ADDR authsvr_bindeps[10];
static unsigned int authsvr_bindeps_count;
static char authsvr_dbconnstr[100];
static int authsvr_dbthread_count;

APPBOX_SETTING_BEGIN(setting)
APPBOX_SETTING_ARRAY_ENDPOINT("bind_eps", authsvr_bindeps, authsvr_bindeps_count, sizeof(authsvr_bindeps)/sizeof(authsvr_bindeps[0]))
APPBOX_SETTING_STRING("dbconnstr", authsvr_dbconnstr, sizeof(authsvr_dbconnstr))
APPBOX_SETTING_INTEGER("dbthread_count", authsvr_dbthread_count)
APPBOX_SETTING_END(setting)

sailing::pool<int, 100> aaa;

int load_config()
{
	char buf[256];
	int ret;

	sprintf(buf, "sailing.%s", MODULE_NAME); 
	ret = appbox_config_get(buf, setting);
	if(ret!=ERR_NOERROR) {
		SYSLOG(LOG_ERROR, MODULE_NAME, "Failed to get configs.");
		return ret;
	}

	return ERR_NOERROR;
}

int module_init()
{
	return ERR_NOERROR;
}

int module_clean()
{
	return ERR_NOERROR;
}
