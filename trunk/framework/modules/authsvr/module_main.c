#include <stdio.h>
#include <skates/skates.h>

#include "module_main.h"

static int module_onload();
static int module_unload();

ZION_EXPORT int module_entry(int reason)
{
	switch(reason) {
	case ENTRY_REASON_ATTACH:
		return module_onload();
	case ENTRY_REASON_DETACH:
		return module_unload();
	}
	return(ERR_UNKNOWN);
}

int module_onload()
{
	int res;

	res = load_config();
	if(res!=ERR_NOERROR) return res;

	res = module_init();
	if(res!=ERR_NOERROR) return res;

	return res;
}

int module_unload()
{
	int res;

	res = module_clean();
	if(res!=ERR_NOERROR) return res;

	return res;
}

