#ifndef _APPBOX_SVC_H_
#define _APPBOX_SVC_H_

typedef int (*APPBOXMAIN_PROC)();

typedef struct APPBOX_SERVICE {
	const char*			name;
	const char*			dispname;

	APPBOXMAIN_PROC		svc_init;
	APPBOXMAIN_PROC		svc_final;
	APPBOXMAIN_PROC		svc_start;
	APPBOXMAIN_PROC		svc_stop;
	APPBOXMAIN_PROC		svc_usage;
} APPBOX_SERVICE;

ZION_API int appbox_service(const APPBOX_SERVICE* svc, int argc, char* argv[]);
ZION_API int appbox_run_debug(APPBOXMAIN_PROC start, APPBOXMAIN_PROC stop);
ZION_API int appbox_run_daemon(APPBOXMAIN_PROC start, APPBOXMAIN_PROC stop, const char* name);
ZION_API int appbox_install(const char* name, const char* dispname, const char* args);
ZION_API int appbox_uninstall(const char* name);

#endif

