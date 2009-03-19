#pragma once

class ISGClientCallback;
class ISGClient;

class ISGClientCallback
{
public:
	virtual ~ISGClientCallback() {}

	virtual void OnConnect(ISGClient* pClient) = 0;
	virtual void OnData(ISGClient* pClient, const void* pData, unsigned int nSize) = 0;
	virtual void OnDisconnect(ISGClient* pClient) = 0;
};

class ISGClient
{
public:
	virtual ~ISGClient() {}

	virtual void Release() = 0;

	virtual int GetIndex() = 0;

	virtual bool Available() = 0;
	virtual bool Connect(const char* pAddr) = 0;
	virtual bool SendData(const void* pData, unsigned int nSize) = 0;
	virtual void Disconnect() = 0;
	virtual bool Wait() = 0;

	virtual void SetUserData(void* pUserData) = 0;
	virtual void* GetUserData() = 0;
};

ISGClient* CreateSGClient(ISGClientCallback* pCallback, int nConn);
