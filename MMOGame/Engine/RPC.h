#pragma once

class IRPCCallback
{
public:
	virtual ~IRPCCallback() {}

	virtual void OnRPCCmd(unsigned int nSource, unsigned int nCmd, const void* pData, unsigned int nSize) = 0;
};

class IRPCClient
{
public:
	virtual ~IRPCClient() {}

	virtual unsigned int GetIp() = 0;
	virtual unsigned short GetPort() = 0;

	virtual bool SendCmd(unsigned int nSource, unsigned int nCmd, const void* pData, unsigned int nSize) = 0;
};

class IRPCServer
{
public:
	virtual ~IRPCServer() {}
	virtual void Release() = 0;

	virtual bool Start(unsigned int nRPCIp, unsigned short nRPCPort) = 0;
	virtual bool Stop() = 0;
	virtual bool Wait() = 0;

	virtual IRPCClient* GetClient(unsigned int nRPCIp, unsigned short nRPCPort) = 0;
};

IRPCServer* RPC_Create(IRPCCallback* pCallback);
