#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../inc/skates/skates.h"
#include "../../inc/skates/appbox_svc.h"
#include "../../inc/skates/appbox_args.h"

static HANDLE	quit_event = NULL;
static SERVICE_STATUS_HANDLE service_status = NULL;
static HANDLE report_handle = NULL;

static void WINAPI ServiceHandler(DWORD dwCtrl);
static VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);

static char svc_name[100] = "";
static APPBOXMAIN_PROC svc_start = NULL;
static APPBOXMAIN_PROC svc_stop = NULL;

static void svc_make_args(char* svr_args, int argc, char* argv[])
{
	int i;
	svr_args[0] = '\0';
	for(i=0; i<argc; i++) {
		if(i>0) strcat(svr_args, " ");
		sprintf(svr_args+strlen(svr_args), "\"%s\"", argv[i]);
	}
}

int appbox_service(APPBOX_SERVICE* svc, int argc, char* argv[])
{
	int ret, debug_mode=1;
	char svr_args[1000];

	if(argc>=2) {
		if(strcmp(argv[1], "install")==0) {
			svc_make_args(svr_args, argc-2, &argv[2]);
			ret = appbox_args_parse(argc-2, &argv[2]);
			if(ret!=ERR_NOERROR) {
				svc->svc_usage();
				return ERR_INVALID_PARAMETER;
			}
			if(appbox_args_get("servicename", NULL)) svc->name = appbox_args_get("servicename", NULL);
			return appbox_install(svc->name, svr_args);
		}
		if(strcmp(argv[1], "uninstall")==0) {
			ret = appbox_args_parse(argc-2, &argv[2]);
			if(ret!=ERR_NOERROR) {
				svc->svc_usage();
				return ERR_INVALID_PARAMETER;
			}
			if(appbox_args_get("servicename", NULL)) svc->name = appbox_args_get("servicename", NULL);
			return appbox_uninstall(svc->name);
		}
	}

	if(argc>=2 && strcmp(argv[1], "service")==0) {
		debug_mode = 0;
		ret = appbox_args_parse(argc-2, &argv[2]);
		report_handle = RegisterEventSource(NULL, appbox_args_get("servicename", svc->name));
	} else if(argc>=2 && strcmp(argv[1], "debug")==0) {
		debug_mode = 1;
		ret = appbox_args_parse(argc-2, &argv[2]);
	} else {
		debug_mode = 1;
		ret = appbox_args_parse(argc-1, &argv[1]);
	}
	if(ret!=ERR_NOERROR) {
		appbox_service_log("invalid args");
		if(report_handle) { DeregisterEventSource(report_handle); report_handle = NULL; }
		if(debug_mode) svc->svc_usage();
		return ret;
	}
	if(appbox_args_get("servicename", NULL)) svc->name = appbox_args_get("servicename", NULL);

	ret = svc->svc_init();
	if(ret!=ERR_NOERROR) {
		appbox_service_log("failed to init %s service, return %d", svc->name, ret);
		if(report_handle) { DeregisterEventSource(report_handle); report_handle = NULL; }
		return ret;
	}

	if(debug_mode) {
		appbox_service_log("run %s service in debug mode", svc->name);
		ret = appbox_run_debug(svc->svc_start, svc->svc_stop);
	} else {
		appbox_service_log("run %s service in service mode", svc->name);
		ret = appbox_run_daemon(svc->svc_start, svc->svc_stop, svc->name);
	}
	if(ret!=ERR_NOERROR) {
		appbox_service_log("error: appbox_run() return %d", ret);
	}

	ret = svc->svc_final();
	if(ret!=ERR_NOERROR) {
		appbox_service_log("failed to final %s service, return %d", svc->name, ret);
		if(report_handle) { DeregisterEventSource(report_handle); report_handle = NULL; }
		return ret;
	}

	if(report_handle) { DeregisterEventSource(report_handle); report_handle = NULL; }
	return ERR_NOERROR;
}

int appbox_service_log(const char* fmt, ...)
{
	va_list valist;
	char buf[1000];
	char* pMsg = buf;

	if(report_handle) {
		va_start(valist, fmt);
		vsnprintf(buf, sizeof(buf), fmt, valist);
		va_end(valist);
		ReportEvent(report_handle, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);
	}

	return ERR_NOERROR;
}

int appbox_run_debug(APPBOXMAIN_PROC start, APPBOXMAIN_PROC stop)
{
	int ret;

	ret = start();
	if(ret!=ERR_NOERROR) return ret;

	printf("Press any key to exit!\n");
	getchar();

	ret = stop();
	if(ret!=ERR_NOERROR) return ret;

	return ERR_NOERROR;
}

int appbox_run_daemon(APPBOXMAIN_PROC start, APPBOXMAIN_PROC stop, const char* name)
{
    SERVICE_TABLE_ENTRY st[] = {
        {NULL, ServiceMain},
        {NULL, NULL}
    };

    char szFilePath[_MAX_PATH];
    GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
	*strrchr(szFilePath, '\\') = '\0';
	SetCurrentDirectory(szFilePath);

	strcpy(svc_name, name);
	st[0].lpServiceName = svc_name;
	svc_start = start;
	svc_stop = stop;

	return StartServiceCtrlDispatcher(st)?ERR_NOERROR:ERR_UNKNOWN;
}

int appbox_install(const char* name, const char* args)
{
	SC_HANDLE hSCM;
    char szFilePath[_MAX_PATH];
	SC_HANDLE hService;

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCM) {
		printf("Failed to OpenSCManager(), ret=%08x\n", GetLastError());
		return -1;
	}

    GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

	sprintf(szFilePath+strlen(szFilePath), " service %s", args);

	hService = CreateService(hSCM, name, "", SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, szFilePath, NULL, NULL, NULL, NULL, NULL);
    if(!hService) {
        CloseServiceHandle(hSCM);
		printf("Failed to CreateService(), ret=%08x\n", GetLastError());
        return -1;
    }

	CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return 0;
}

int appbox_uninstall(const char* name)
{
	SC_HANDLE hSCM;
	BOOL bResult = FALSE;
	SC_HANDLE hService;

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCM) {
		printf("Failed to OpenSCManager(), ret=%08x\n", GetLastError());
		return -1;
	}

	hService = OpenService(hSCM, name, DELETE);
	if(!hService) {
	    CloseServiceHandle(hSCM);
		printf("Failed to OpenService(), ret=%08x\n", GetLastError());
		return -1;
	}

	if(!DeleteService(hService)) {
		CloseServiceHandle(hService);
	    CloseServiceHandle(hSCM);
		printf("Failed to DeleteService(), ret=%08x\n", GetLastError());
		return -1;
    }

	CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return bResult;
}

void WINAPI ServiceHandler(DWORD dwCtrl)
{
	switch(dwCtrl) {
	case SERVICE_CONTROL_STOP:
         SetEvent(quit_event);
		 return;
	default:
		break;
	}
}

static VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
	SERVICE_STATUS_HANDLE service_handle;
	SERVICE_STATUS service_status;
	int ret;

	quit_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(quit_event==NULL) return;

	service_handle = RegisterServiceCtrlHandler(svc_name, ServiceHandler);
	if(service_handle==NULL) {
		appbox_service_log("Failed to RegisterServiceCtrlHandler(\"%s\"), ret=%x", svc_name, GetLastError());
		return;

	}

	appbox_service_log("APPBOX Service starting, Name=%s", svc_name);

	service_status.dwServiceType				= SERVICE_WIN32;
	service_status.dwCurrentState				= SERVICE_START_PENDING;  
	service_status.dwControlsAccepted			= 0;
	service_status.dwWin32ExitCode				= 0; 
	service_status.dwServiceSpecificExitCode	= 0; 
	service_status.dwCheckPoint					= 0;
	service_status.dwWaitHint					= 0;
	SetServiceStatus(service_handle, &service_status);

	ret = svc_start();
	if(ret!=ERR_NOERROR) {
		appbox_service_log("Failed to start service(%s), ret=%x", svc_name, ret);
		service_status.dwCurrentState			= SERVICE_STOPPED;
		service_status.dwWin32ExitCode			= (DWORD)ret; 
		SetServiceStatus(service_handle, &service_status);
		return;
	}

	appbox_service_log("APPBOX Service is runing, Name=%s", svc_name);

	service_status.dwCurrentState				= SERVICE_RUNNING;
	service_status.dwControlsAccepted			= SERVICE_ACCEPT_STOP;
	SetServiceStatus(service_handle, &service_status);

	WaitForSingleObject(quit_event, INFINITE);

	appbox_service_log("APPBOX Service stoping, Name=%s", svc_name);

	service_status.dwCurrentState				= SERVICE_STOP_PENDING;
	service_status.dwControlsAccepted			= 0;
	SetServiceStatus(service_handle, &service_status);

	ret = svc_stop();
	if(ret!=ERR_NOERROR) {
		appbox_service_log("Failed to stop service(%s), ret=%x", svc_name, ret);
		service_status.dwCurrentState			= SERVICE_STOPPED;
		service_status.dwWin32ExitCode			= (DWORD)ret; 
		SetServiceStatus(service_handle, &service_status);
		return;
	}

	appbox_service_log("APPBOX Service stoped, Name=%s", svc_name);

	service_status.dwCurrentState				= SERVICE_STOPPED;
	service_status.dwControlsAccepted			= 0;
	SetServiceStatus(service_handle, &service_status);

	CloseHandle(quit_event);
	quit_event = NULL;
}
