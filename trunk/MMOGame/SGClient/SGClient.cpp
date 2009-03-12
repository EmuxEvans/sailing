#include <stdio.h>

#include "SGClient.h"

class CSGClient : public ISGClient
{
public:
	CSGClient(ISGClientCallback* pCallback);
	virtual ~CSGClient();

	virtual void Release();

	virtual bool Available();
	virtual void Connect(unsigned int ip, unsigned short port);
	virtual void SendData(const void* pData, unsigned int nSize);
	virtual void Disconnect();
	virtual void Wait();

	virtual void SetUserData(void* pUserData);
	virtual void* GetUserData();

private:
	ISGClientCallback* m_pCallback;
	void* m_pUserData;
};


ISGClient* CreateSGClient(ISGClientCallback* pCallback, bool bAsync)
{
	return new CSGClient(pCallback);
}

CSGClient::CSGClient(ISGClientCallback* pCallback)
{
	m_pCallback = pCallback;
	m_pUserData = NULL;
}

CSGClient::~CSGClient()
{
}

void CSGClient::Release()
{
	delete this;
}

bool CSGClient::Available()
{
	return true;
}

void CSGClient::Connect(unsigned int ip, unsigned short port)
{
}

void CSGClient::SendData(const void* pData, unsigned int nSize)
{
}

void CSGClient::Disconnect()
{
}

void CSGClient::Wait()
{
}

void CSGClient::SetUserData(void* pUserData)
{
	m_pUserData = pUserData;
}

void* CSGClient::GetUserData()
{
	return m_pUserData;
}
