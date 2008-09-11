#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include "../../inc/skates/skates.h"
#include "../../inc/skates/appbox_svc.h"
#include "../../inc/skates/appbox_args.h"

static void init_daemon(const char* pid);
static void signal_proc(int signal);
static void wait_signal();

static int flag_daemon = 0;
static char pid_file[100] = "";

int appbox_service(APPBOX_SERVICE* svc, int argc, char* argv[])
{
	int ret;

	ret = appbox_args_parse(argc, argv);
	if(ret!=ERR_NOERROR) {
		svc->svc_usage();
		return ret;
	}

	if(appbox_args_get("daemon", NULL))
		flag_daemon = 1;
	if(appbox_args_get("debug", NULL))
		flag_daemon = 0;

	strcpy(pid_file, appbox_args_get("pid", "appbox.pid"));

	ret = svc->svc_init();
	if(ret!=ERR_NOERROR)
		return ret;

	if(flag_daemon) {
		ret = appbox_run_daemon(svc->svc_start, svc->svc_stop, pid_file);
	} else {
		ret = appbox_run_debug(svc->svc_start, svc->svc_stop);
	}
	if(ret!=ERR_NOERROR)
		return ret;

	ret = svc->svc_final();
	if(ret!=ERR_NOERROR)
		return ret;

	return ERR_NOERROR;
}

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

int appbox_install(const char* name, const char* args)
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
