#ifndef _APPBOXMAIN_H_
#define _APPBOXMAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*APPBOXMAIN_PROC)();

ZION_API int appbox_run_debug(APPBOXMAIN_PROC start, APPBOXMAIN_PROC stop);
ZION_API int appbox_run_daemon(APPBOXMAIN_PROC start, APPBOXMAIN_PROC stop, const char*);
ZION_API int appbox_install(const char* name, int argc, char* argv[]);
ZION_API int appbox_uninstall(const char* name, int argc, char* argv[]);

#ifdef APPBOX_MAIN

ZION_API int appbox_main(int argc, char* argv[]);
ZION_API int appbox_main_load(const char* config_path);
ZION_API int appbox_main_start();
ZION_API int appbox_main_stop();

#endif

extern char config_syslog[200];
extern char config_dbglog[200];
extern int  config_syslog_enable;
extern int	config_dbglog_enable;
extern int	config_worker_count;
extern int	config_timer_interval;
extern char config_module_list[300];
extern SOCK_ADDR config_rpc_endpoint;
extern SOCK_ADDR config_con_endpoint;
extern unsigned int config_con_maxconns;
extern unsigned int config_con_maxhooker;

#ifdef __cplusplus
}
#endif

#endif
