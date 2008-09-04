#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include "../inc/skates.h"
#include "../inc/appbox_svc.h"

static void init_daemon(const char* pid);
static void signal_proc(int signal);
static void wait_signal();

int appbox_run_debug(APPBOXMAIN_PROC start, APPBOXMAIN_PROC stop)
{
	int ret;

	ret = start();
	if(ret!=ERR_NOERROR) return ret;
	printf("press any key to exit.\n");
	getchar();
	ret = stop();
	if(ret!=ERR_NOERROR) return ret;

	return ERR_NOERROR;
}

int appbox_run_daemon(APPBOXMAIN_PROC start, APPBOXMAIN_PROC stop, const char* pidfile)
{
	int ret;

	init_daemon(pidfile);
        
	ret = start();
	if(ret!=ERR_NOERROR) return ret;
	wait_signal();
	ret = stop();
	if(ret!=ERR_NOERROR) return ret;

	return ERR_NOERROR;;
}

int appbox_install(const char* name, const char* dispname, const char* args)
{
	printf("not support\n");
	return ERR_UNKNOWN;
}

int appbox_uninstall(const char* name)
{
	printf("not support\n");
	return ERR_UNKNOWN;
}

void init_daemon(const char* _pid)
{
	int pid, fd, ret;
	struct sigaction sa;

	sa.sa_handler = SIG_IGN;
	ret = sigaction(SIGPIPE, &sa, NULL);
	assert(ret==0);

    pid = fork();
    if(pid!=0) exit(0);
    else if(pid< 0) exit(1);
    setsid();

	if(_pid[0]!='\0') {
		FILE* fp;
		fp = fopen(_pid, "wt");
		if(fp==NULL) return;
		fprintf(fp, "%d\n", getpid());
		fclose(fp);
	}

	fd = open("/dev/null", O_RDWR, 0);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	ret = dup2(fd, STDIN_FILENO);
	if(ret==-1) assert(0);
	ret = dup2(fd, STDOUT_FILENO);
	if(ret==-1) assert(0);
	ret = dup2(fd, STDERR_FILENO);
	if(ret==-1) assert(0);
	close(fd);
}

void signal_proc(int signal)
{
}

void wait_signal()
{
	sigset_t set;
	int res, signo;

	signal(SIGINT, signal_proc);
	signal(SIGTERM, signal_proc);
	signal(SIGPIPE, signal_proc);

	for(;;) {
		sigfillset(&set);
		sigdelset(&set, SIGSEGV);
		sigdelset(&set, SIGBUS);
		res = sigwait(&set, &signo);
		if(res==0 && (signo==SIGINT || signo==SIGQUIT || signo==SIGTERM)) break;
	}
}

#ifdef APPBOX_MAIN

#ifndef _DEBUG
static int flag_daemon		= 1;
static char config_path[100] = "../conf/config.conf";
static char pid_file[100] = "appbox.pid";
#else
static int flag_daemon		= 0;
static char config_path[100] = "../conf/config.conf";
static char pid_file[100] = "appbox.pid";
#endif

static void print_usage();
static void parse_cmdline(int argc, char* argv[]);

int appbox_main(int argc, char* argv[])
{
	int ret;

	parse_cmdline(argc-1, argv+1);

	ret = appbox_main_load(config_path);
	if(ret!=ERR_NOERROR) return ret;

	if(flag_daemon) {
		ret = appbox_run_daemon(appbox_main_start, appbox_main_stop, pid_file);
	} else {
		ret = appbox_run_debug(appbox_main_start, appbox_main_stop);
	}
	if(ret!=ERR_NOERROR) return ret;

	return ERR_NOERROR;
}

void print_usage()
{
	printf("usage : appbox_main [-daemon] [-debug] [-config=file] [-pid=file]\n");
	printf("\n");
}

void parse_cmdline(int argc, char* argv[])
{
	int ret;

	ret = appbox_args_parse(argc, argv);
	if(ret!=ERR_NOERROR) {
		print_usage();
		exit(-1);
	}

	if(appbox_args_get("daemon", NULL))		flag_daemon = 1;
	if(appbox_args_get("debug", NULL))		flag_daemon = 0;
	strcpy(config_path, appbox_args_get("config", "../conf/config.conf");
	strcpy(pid_file, appbox_args_get("pid", "appbox.pid");
	strtrim(pid_file);
	strltrim(pid_file);
}

#endif

