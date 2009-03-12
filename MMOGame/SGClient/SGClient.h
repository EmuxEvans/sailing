#pragma once

class ISGClientCallback;
class ISGClient;

class ISGClientCallback
{
public:
	virtual ~ISGClientCallback() {}

	virtual void OnConnect() = 0;
	virtual void OnData(const void* pData, unsigned int nSize) = 0;
	virtual void OnDisconnect() = 0;
};

class ISGClient
{
public:
	virtual ~ISGClient() {}

	virtual void Release() = 0;

	virtual bool Available() = 0;
	virtual void Connect(unsigned int ip, unsigned short port) = 0;
	virtual void SendData(const void* pData, unsigned int nSize) = 0;
	virtual void Disconnect() = 0;
	virtual void Wait() = 0;

	virtual void SetUserData(void* pUserData) = 0;
	virtual void* GetUserData() = 0;
};

ISGClient* CreateSGClient(ISGClientCallback* pCallback, bool bAsync);
