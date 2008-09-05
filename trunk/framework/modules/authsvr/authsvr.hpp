#ifndef _AUTHSVR_H_
#define _AUTHSVR_H_

// ip auth
int authsvr_ipauth(const SOCK_ADDR* sa);


// account info
typedef struct AUTHSVR_ACCOUNT_INFO {
	char password[30];
} AUTHSVR_ACCOUNT_INFO;

int authsvr_accountinfo_get(DBAPI_HANDLE handle, const char* account, AUTHSVR_ACCOUNT_INFO* info);


//


#endif
