#include <stdio.h>
#include <assert.h>
#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/sock.h>

#include "SGClient.h"

class CSGClient : public ISGClient
{
public:
	CSGClient(ISGClientCallback* pCallback, int nConn);
	virtual ~CSGClient();

	virtual void Release();

	virtual int GetIndex();
	virtual bool Available();
	virtual bool Connect(const char* pAddr);
	virtual bool SendData(const void* pData, unsigned int nSize);
	virtual void Disconnect();
	virtual bool Wait();

	virtual void SetUserData(void* pUserData);
	virtual void* GetUserData();

private:
	int m_nConn;
	SOCK_HANDLE m_hSock;
	ISGClientCallback* m_pCallback;
	void* m_pUserData;
};

ISGClient* CreateSGClient(ISGClientCallback* pCallback, int nConn)
{
	return new CSGClient(pCallback, nConn);
}

CSGClient::CSGClient(ISGClientCallback* pCallback, int nConn)
{
	m_nConn = nConn;
	m_hSock = SOCK_INVALID_HANDLE;
	m_pCallback = pCallback;
	m_pUserData = NULL;
}

CSGClient::~CSGClient()
{
	assert(m_hSock==SOCK_INVALID_HANDLE);
}	

void CSGClient::Release()
{
	delete this;
}

int CSGClient::GetIndex()
{
	return m_nConn;
}

bool CSGClient::Available()
{
	return m_hSock!=SOCK_INVALID_HANDLE;
}

bool CSGClient::Connect(const char* pAddr)
{
	assert(m_hSock==SOCK_INVALID_HANDLE);

	SOCK_ADDR sa;
	if(!sock_str2addr(pAddr, &sa)) return false;
	m_hSock = sock_connect(&sa, 0);
	if(m_hSock==SOCK_INVALID_HANDLE) return false;

	m_pCallback->OnConnect(this);

	return true;
}

bool CSGClient::SendData(const void* pData, unsigned int nSize)
{
	int ret;
	ret = sock_writebuf(m_hSock, &nSize, sizeof(unsigned short));
	if(ret!=0) return false;
	ret = sock_writebuf(m_hSock, pData, nSize);
	if(ret!=0) return false;
	return true;
}

void CSGClient::Disconnect()
{
	if(m_hSock!=SOCK_INVALID_HANDLE) {
		m_pCallback->OnDisconnect(this);
		sock_disconnect(m_hSock);
		sock_close(m_hSock);
		m_hSock = SOCK_INVALID_HANDLE;
	}
}

bool CSGClient::Wait()
{
	int ret;
	unsigned int nSize;
	char szBuf[10*1024];

	if(sock_wait_read(m_hSock, 10)==ERR_TIMEOUT) {
		return true;
	}

	nSize = 0;
	ret = sock_readbuf(m_hSock, &nSize, sizeof(unsigned short));
	if(ret!=0) {
		Disconnect();
		return false;
	}
	ret = sock_readbuf(m_hSock, szBuf, nSize);
	if(ret!=0) {
		Disconnect();
		return false;
	}
	m_pCallback->OnData(this, szBuf, nSize);
	return true;
}

void CSGClient::SetUserData(void* pUserData)
{
	m_pUserData = pUserData;
}

void* CSGClient::GetUserData()
{
	return m_pUserData;
}
