#include <skates/skates.h>

#include <sailing/pool.hpp>

#include "authsvr.hpp"

int authsvr_ipauth(const SOCK_ADDR* sa)
{
	return 1;
}

int authsvr_accountinfo_get(DBAPI_HANDLE handle, const char* account, AUTHSVR_ACCOUNT_INFO* info)
{
	return ERR_NOERROR;
}
