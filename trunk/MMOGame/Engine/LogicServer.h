#pragma once

class ILogicServer
{
public:
	virtual ~ILogicServer() { }

	virtual bool Start(unsigned int nIp, unsigned int nPort) = 0;
	virtual void Stop() = 0;
	virtual void Wait() = 0;
};

class ILogicServerCallback
{
public:
	virtual ~ILogicServerCallback() { }

	virtual void OnStart() = 0;
	virtual void OnStop() = 0;
	virtual void OnCommand(const CmdData* pData) = 0;
};

class ILogicClient
{
public:
	virtual ~ILogicClient() { }

	virtual bool SendCommand(const CmdData* pData) = 0;
};

ILogicServer* CreateLogicServer(ILogicServerCallback* pCallback);
ILogicClient* AcquieLogicClient(unsigned int nIp, unsigned int nPort);
