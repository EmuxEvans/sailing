#include <stdio.h>

#include "../inc/skates.h"
#include "../inc/appbox_main.h"

static char listen[100] = "0.0.0.0:1980";
static char config[100] = "csmng.config";
static char svcname[100] = "csmng";
static SOCK_ADDR listen_ep;
static CONSOLE_INSTANCE* instance;

static int parse_cmdline(int argc, char* argv[]);
static void print_usage();
static int main_start();
static int main_stop();

int main(int argc, char* argv[])
{
	int ret;

	if(argc>=2) {
		if(strcmp(argv[1], "install")==0)	return appbox_install("csmng", argc-2, &argv[2]);
		if(strcmp(argv[1], "uninstall")==0) return appbox_uninstall("csmng", argc-2, &argv[2]);

		if(strcmp(argv[1], "service")==0)	{
			parse_cmdline(argc-2, &argv[2]);
			ret = appbox_run_daemon(main_start, main_stop, svcname);
			return ret;
		}
		if(strcmp(argv[1], "debug")==0) {
			parse_cmdline(argc-2, &argv[2]);
			ret = appbox_run_debug(main_start, main_stop);
			return ret;
		}
	}

	parse_cmdline(argc-1, &argv[1]);
	return appbox_run_debug(main_start, main_stop);
}

int parse_cmdline(int argc, char* argv[])
{
	char name[100], value[100];
	char* s, *flag;
	int cur;

	for(cur=0; cur<argc; cur++) {
		s = argv[cur];
		if(s[0]!='-') break;
		flag = strchr(s, '=');
		if(flag==NULL) {
			strcpy(name, s+1);
			value[0] = '\0';
		} else {
			*flag = '\0';
			strcpy(name, s+1);
			strcpy(value, flag+1);
		}

		if(	strcmp(name, "listen")==0) {
			strcpy(listen, value);
		} else if(	strcmp(name, "config")==0) {
			strcpy(config, value);
		} else if(	strcmp(name, "svcname")==0) {
			strcpy(svcname, value);
		} else {
			break;
		}
	}

	if(cur!=argc) {
		print_usage();
		return ERR_INVALID_PARAMETER;
	}

	return ERR_NOERROR;
}

void print_usage()
{
	printf("usage :\n");
	printf("	appbox_main.exe install [-servicename=name] [-servicedesc=desc] [-cmdline=line]\n");
	printf("	appbox_main.exe uninstall [-servicename=name]\n");
	printf("	appbox_main.exe service [-config=file]\n");
	printf("	appbox_main.exe debug [-config=file]\n");
	printf("\n");
}

int main_start()
{
	if(!sock_str2addr(listen, &listen_ep))
		return ERR_INVALID_PARAMETER;

	sock_init();
	fdwatch_init();

	instance = console_create_csmng(&listen_ep, 100, 20, 1000, config);
	if(instance==NULL) {
		printf("Failed to console_create_csmng()\n");
		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

int main_stop()
{
	console_destroy(instance);

	fdwatch_final();
	sock_final();

	return ERR_NOERROR;
}
