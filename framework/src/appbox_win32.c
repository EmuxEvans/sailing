#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/skates.h"
#include "../inc/appbox_svc.h"
#include "../inc/appbox_args.h"

static HANDLE	quit_event = NULL;
static SERVICE_STATUS_HANDLE service_status = NULL;

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

int appbox_service(const APPBOX_SERVICE* svc, int argc, char* argv[])
{
	int ret, debug_mode=1;
	char svr_args[1000];

	if(argc>=2) {
		if(strcmp(argv[1], "install")==0) {
			svc_make_args(svr_args, argc-2, &argv[2]);
			return appbox_install(svc->name, svc->dispname, svr_args);
		}
		if(strcmp(argv[1], "uninstall")==0) {
			return appbox_uninstall(svc->name);
		}
	}

	if(argc>=2 && strcmp(argv[1], "service")==0) {
		debug_mode = 0;
		ret = appbox_args_parse(argc-2, &argv[2]);
	} else if(argc>=2 && strcmp(argv[1], "debug")==0) {
		debug_mode = 1;
		ret = appbox_args_parse(argc-2, &argv[2]);
	} else {
		debug_mode = 1;
		ret = appbox_args_parse(argc-1, &argv[1]);
	}

	ret = svc->svc_init();
	if(ret!=ERR_NOERROR) return ret;

	if(debug_mode) {
		ret = appbox_run_debug(svc->svc_start, svc->svc_stop);
	} else {
		ret = appbox_run_daemon(svc->svc_start, svc->svc_stop, NULL);
	}
	if(ret!=ERR_NOERROR) return ret;

	ret = svc->svc_final();
	if(ret!=ERR_NOERROR) return ret;

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

int appbox_install(const char* name, const char* dispname, const char* args)
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

	hService = CreateService(hSCM, name, dispname, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, szFilePath, NULL, NULL, NULL, NULL, NULL);
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
	char szMsg[1000], *pMsg = szMsg;
	HANDLE report_handle;
	SERVICE_STATUS_HANDLE service_handle;
	SERVICE_STATUS service_status;
	int ret;

	quit_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(quit_event==NULL) return;

	report_handle = RegisterEventSource(NULL, svc_name);
	if(report_handle==NULL) return;

	service_handle = RegisterServiceCtrlHandler(svc_name, ServiceHandler);
	if(service_handle==NULL) {
		sprintf(szMsg, "Failed to RegisterServiceCtrlHandler(\"%s\"), ret=%x", svc_name, GetLastError());
		ReportEvent(report_handle, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);
		DeregisterEventSource(report_handle);
		return;

	}

	sprintf(szMsg, "APPBOX Service starting, Name=%s", svc_name);
	ReportEvent(report_handle, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);

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
		sprintf(szMsg, "Failed to start service(%s), ret=%x", svc_name, ret);
		ReportEvent(report_handle, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);
		service_status.dwCurrentState			= SERVICE_STOPPED;
		service_status.dwWin32ExitCode			= (DWORD)ret; 
		SetServiceStatus(service_handle, &service_status);
		return;
	}

	sprintf(szMsg, "APPBOX Service is runing, Name=%s", svc_name);
	ReportEvent(report_handle, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);

	service_status.dwCurrentState				= SERVICE_RUNNING;
	service_status.dwControlsAccepted			= SERVICE_ACCEPT_STOP;
	SetServiceStatus(service_handle, &service_status);

	WaitForSingleObject(quit_event, INFINITE);

	sprintf(szMsg, "APPBOX Service stoping, Name=%s", svc_name);
	ReportEvent(report_handle, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);

	service_status.dwCurrentState				= SERVICE_STOP_PENDING;
	service_status.dwControlsAccepted			= 0;
	SetServiceStatus(service_handle, &service_status);

	ret = svc_stop();
	if(ret!=ERR_NOERROR) {
		sprintf(szMsg, "Failed to stop service(%s), ret=%x", svc_name, ret);
		ReportEvent(report_handle, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);
		service_status.dwCurrentState			= SERVICE_STOPPED;
		service_status.dwWin32ExitCode			= (DWORD)ret; 
		SetServiceStatus(service_handle, &service_status);
		return;
	}

	sprintf(szMsg, "APPBOX Service stoped, Name=%s", svc_name);
	ReportEvent(report_handle, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);

	service_status.dwCurrentState				= SERVICE_STOPPED;
	service_status.dwControlsAccepted			= 0;
	SetServiceStatus(service_handle, &service_status);

	CloseHandle(quit_event);
	quit_event = NULL;
}
