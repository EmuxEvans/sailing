#include <string.h>

#include "RPC.h"

class CRPCClient : public IRPCClient
{
public:
	CRPCClient() {}
	virtual ~CRPCClient() {}

	virtual unsigned int GetIp() {
		return 0;
	}
	virtual unsigned short GetPort() {
		return 0;
	}

	virtual bool SendCmd(unsigned int nSource, unsigned int nCmd, const void* pData, unsigned int nSize) {
		return false;
	}
};

class CRPCServer : public IRPCServer
{
public:
	virtual ~CRPCServer() {}
	virtual void Release() {
	}

	virtual bool Start(unsigned int nRPCIp, unsigned short nRPCPort) {
		return false;
	}
	virtual bool Stop() {
		return false;
	}
	virtual bool Wait() {
		return false;
	}

	IRPCClient* GetClient(unsigned int nRPCIp, unsigned short nRPCPort) {
		return NULL;
	}
};

IRPCServer* RPC_Create(IRPCCallback* pCallback)
{
	return NULL;
}
