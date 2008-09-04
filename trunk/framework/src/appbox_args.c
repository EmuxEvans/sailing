
#include <string.h>
#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/appbox_args.h"

static struct {
	char name[100];
	char value[100];
} appbox_args[20];
static int appbox_args_count = 0;

int appbox_args_parse(int argc, char* argv[])
{
	char* s, *flag;
	int cur;

	for(cur=0; cur<argc; cur++) {
		s = argv[cur];
		if(s[0]!='-') break;
		flag = strchr(s, '=');
		if(flag==NULL) {
			strcpy(appbox_args[cur].name, s+1);
			appbox_args[cur].value[0] = '\0';
		} else {
			*flag = '\0';
			strcpy(appbox_args[cur].name, s+1);
			strcpy(appbox_args[cur].value, flag+1);
		}
	}

	appbox_args_count = cur;
	return cur!=argc?ERR_INVALID_PARAMETER:ERR_NOERROR;
}

const char* appbox_args_get(const char* name, const char* defvalue)
{
	int i;
	for(i=0; i<appbox_args_count; i++) {
		if(strcmp(name, appbox_args[i].name)==0)
			return appbox_args[i].value;
	}
	return defvalue;
}
