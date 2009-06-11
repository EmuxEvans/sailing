#pragma once

class ISessionClient
{
public:
	virtual ~ISessionClient() { }

	virtual bool BindUID(unsigned int nUID) = 0;
	virtual bool UnbindUID(unsigned int nUID) = 0;
	virtual unsigned int GetUID() = 0;

	virtual bool SendData(const void* pData, unsigned int nLength) = 0;
	virtual void Disconnect() = 0;

	virtual void SetUserData(void* pUserData) = 0;
	virtual void* GetUserData() = 0;
};

class ISessionServer
{
public:
	virtual ~ISessionServer() { }

	virtual bool Start(unsigned int nIp, unsigned int& nPort) = 0;
	virtual void Stop() = 0;
	virtual void Wait() = 0;
};

class ISessionServerCallback
{
public:
	virtual ~ISessionServerCallback() { }

	virtual void OnConnect(ISessionClient* pClient) = 0;
	virtual void OnDisconnect(ISessionClient* pClient) = 0;
	virtual void OnData(ISessionClient* pClient, const void* pData, unsigned int nLength) = 0;
};

ISessionServer* CreateSessionServer(ISessionServerCallback* pCallback);
