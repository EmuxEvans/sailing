#pragma once

class ITCPConnection
{
public:
	virtual ~ITCPConnection() {}

	virtual void SetUserData(void* pUserData) = 0;
	virtual void* GetUserData() = 0;

	virtual void Disconnect() = 0;
	virtual void SendData(const void* pData, unsigned int nSize) = 0;
};

class ITCPCallback
{
public:
	virtual ~ITCPCallback() {}

	virtual void OnConnect(ITCPConnection* pConnection) = 0;
	virtual void OnData(ITCPConnection* pConnection, const void* pData, unsigned nSize) = 0;
	virtual void OnDisconnect(ITCPConnection* pConnection) = 0;
};

class ITCPServer
{
public:
	virtual ~ITCPServer() {}
	virtual void Release() = 0;

	virtual bool Start(ITCPCallback* pCallback) = 0;
	virtual bool Stop() = 0;
	virtual bool Wait() = 0;

	virtual bool AddListenPort(unsigned int nTCPIp, unsigned int nTCPPort) = 0;
};

ITCPServer* TCPServer_Create();
