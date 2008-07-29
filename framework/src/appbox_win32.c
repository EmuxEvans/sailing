#include <stdio.h>
#include <stdlib.h>

#include "../inc/skates.h"
#include "../inc/appbox_main.h"

static HANDLE	quit_event = NULL;
static SERVICE_STATUS_HANDLE service_status = NULL;

static void WINAPI ServiceHandler(DWORD dwCtrl);
static VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);

static char svr_name[100]="";
static char svr_dispname[100]="";
static char svr_args[100]="";
static SERVICE_TABLE_ENTRY svc_table[] = {{NULL, ServiceMain}, {NULL, NULL}};
static APPBOXMAIN_PROC svr_start;
static APPBOXMAIN_PROC svr_stop;

static int svc_parse_cmdline(int argc, char* argv[])
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

		if(	strcmp(name, "name")==0) {
			strcpy(svr_name, value);
		} else if(	strcmp(name, "dispname")==0) {
			strcpy(svr_dispname, value);
		} else if(	strcmp(name, "args")==0) {
			strcpy(svr_args, value);
		} else {
			break;
		}
	}

	if(cur!=argc)
		return ERR_INVALID_PARAMETER;

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

int appbox_run_daemon(APPBOXMAIN_PROC start, APPBOXMAIN_PROC stop, const char* p_servicename)
{
    SERVICE_TABLE_ENTRY st[] = {
        {NULL, ServiceMain},
        {NULL, NULL}
    };

    char szFilePath[_MAX_PATH];
    GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
	*strrchr(szFilePath, '\\') = '\0';
	SetCurrentDirectory(szFilePath);

	strcpy(svr_name, p_servicename);
	st[0].lpServiceName = svr_name;
	svr_start = start;
	svr_stop = stop;

	return StartServiceCtrlDispatcher(st)?ERR_NOERROR:ERR_UNKNOWN;
}

int appbox_install(const char* name, int argc, char* argv[])
{
	int ret;
	SC_HANDLE hSCM;
    char szFilePath[_MAX_PATH];
	SC_HANDLE hService;

	strcpy(svr_name, name);
	ret = svc_parse_cmdline(argc, argv);
	if(ret!=ERR_NOERROR) {
		printf("invalid parameter!\n");
		return ret;
	}
	if(svr_dispname[0]=='\0') {
		strcpy(svr_dispname, svr_name);
	}

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCM) {
		printf("Failed to OpenSCManager(), ret=%08x\n", GetLastError());
		return -1;
	}

    GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

	sprintf(szFilePath+strlen(szFilePath), " service %s", svr_args);

	hService = CreateService(hSCM, svr_name, svr_dispname, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, szFilePath, NULL, NULL, NULL, NULL, NULL);
    if(!hService) {
        CloseServiceHandle(hSCM);
		printf("Failed to CreateService(), ret=%08x\n", GetLastError());
        return -1;
    }

	CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return 0;
}

int appbox_uninstall(const char* name, int argc, char* argv[])
{
	int ret;
	SC_HANDLE hSCM;
	BOOL bResult = FALSE;
	SC_HANDLE hService;

	strcpy(svr_name, name);
	ret = svc_parse_cmdline(argc, argv);
	if(ret!=ERR_NOERROR) {
		printf("invalid parameter!\n");
		return ret;
	}

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCM) {
		printf("Failed to OpenSCManager(), ret=%08x\n", GetLastError());
		return -1;
	}

	hService = OpenService(hSCM, svr_name, DELETE);
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

	report_handle = RegisterEventSource(NULL, svr_name);
	if(report_handle==NULL) return;

	service_handle = RegisterServiceCtrlHandler(svr_name, ServiceHandler);
	if(service_handle==NULL) {
		sprintf(szMsg, "Failed to RegisterServiceCtrlHandler(\"%s\"), ret=%x", svr_name, GetLastError());
		ReportEvent(report_handle, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);
		DeregisterEventSource(report_handle);
		return;

	}

	sprintf(szMsg, "APPBOX Service starting, Name=%s", svr_name);
	ReportEvent(report_handle, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);

	service_status.dwServiceType				= SERVICE_WIN32;
	service_status.dwCurrentState				= SERVICE_START_PENDING;  
	service_status.dwControlsAccepted			= 0;
	service_status.dwWin32ExitCode				= 0; 
	service_status.dwServiceSpecificExitCode	= 0; 
	service_status.dwCheckPoint					= 0;
	service_status.dwWaitHint					= 0;
	SetServiceStatus(service_handle, &service_status);

	ret = svr_start();
	if(ret!=ERR_NOERROR) {
		sprintf(szMsg, "Failed to start service(%s), ret=%x", svr_name, ret);
		ReportEvent(report_handle, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);
		service_status.dwCurrentState			= SERVICE_STOPPED;
		service_status.dwWin32ExitCode			= (DWORD)ret; 
		SetServiceStatus(service_handle, &service_status);
		return;
	}

	sprintf(szMsg, "APPBOX Service is runing, Name=%s", svr_name);
	ReportEvent(report_handle, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);

	service_status.dwCurrentState				= SERVICE_RUNNING;
	service_status.dwControlsAccepted			= SERVICE_ACCEPT_STOP;
	SetServiceStatus(service_handle, &service_status);

	WaitForSingleObject(quit_event, INFINITE);

	sprintf(szMsg, "APPBOX Service stoping, Name=%s", svr_name);
	ReportEvent(report_handle, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);

	service_status.dwCurrentState				= SERVICE_STOP_PENDING;
	service_status.dwControlsAccepted			= 0;
	SetServiceStatus(service_handle, &service_status);

	ret = svr_stop();
	if(ret!=ERR_NOERROR) {
		sprintf(szMsg, "Failed to stop service(%s), ret=%x", svr_name, ret);
		ReportEvent(report_handle, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);
		service_status.dwCurrentState			= SERVICE_STOPPED;
		service_status.dwWin32ExitCode			= (DWORD)ret; 
		SetServiceStatus(service_handle, &service_status);
		return;
	}

	sprintf(szMsg, "APPBOX Service stoped, Name=%s", svr_name);
	ReportEvent(report_handle, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &pMsg, NULL);

	service_status.dwCurrentState				= SERVICE_STOPPED;
	service_status.dwControlsAccepted			= 0;
	SetServiceStatus(service_handle, &service_status);

	CloseHandle(quit_event);
	quit_event = NULL;
}

#ifdef APPBOX_MAIN

static char config_path[100] = "../conf/config.conf";
static int parse_cmdline(int argc, char* argv[]);
static void print_usage();

int appbox_main(int argc, char* argv[])
{
	int ret;

	if(argc>=2) {
		if(strcmp(argv[1], "install")==0)	return appbox_install("appbox", argc-2, &argv[2]);
		if(strcmp(argv[1], "uninstall")==0) return appbox_uninstall("appbox", argc-2, &argv[2]);
		if(strcmp(argv[1], "service")==0)	{
			parse_cmdline(argc-2, &argv[2]);
			ret = appbox_main_load(config_path);
			if(ret!=ERR_NOERROR) return ret;
			ret = appbox_run_daemon(appbox_main_start, appbox_main_stop, NULL);
			if(ret!=ERR_NOERROR) return ret;
			return ERR_NOERROR;
		}
		if(strcmp(argv[1], "debug")==0) {
			parse_cmdline(argc-2, &argv[2]);
			ret = appbox_main_load(config_path);
			if(ret!=ERR_NOERROR) return ret;
			ret = appbox_run_debug(appbox_main_start, appbox_main_stop);
			if(ret!=ERR_NOERROR) return ret;
			return ERR_NOERROR;
		}
	}

	parse_cmdline(argc-1, &argv[1]);
	ret = appbox_main_load(config_path);
	if(ret!=ERR_NOERROR) return ret;
	ret = appbox_run_debug(appbox_main_start, appbox_main_stop);
	if(ret!=ERR_NOERROR) return ret;
	return ERR_NOERROR;
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

		if(	strcmp(name, "config")==0) {
			strcpy(config_path, value);
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
	printf("\tappbox_main.exe install [-servicename=name] [-servicedesc=desc] [-cmdline=line]\n");
	printf("\tappbox_main.exe uninstall [-servicename=name]\n");
	printf("\tappbox_main.exe service [-servicename=name] [-config=file]\n");
	printf("\tappbox_main.exe debug [-config=fible]\n");
	printf("\n");
}

#endif
