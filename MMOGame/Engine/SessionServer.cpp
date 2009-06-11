#include "SessionServer.h"

class CSessionClient : public ISessionClient
{
public:
	CSessionClient() { }
	virtual ~CSessionClient() { }

	virtual bool BindUID(unsigned int nUID) { return true; }
	virtual bool UnbindUID(unsigned int nUID) { return false; }
	virtual unsigned int GetUID() { return m_nUID; }

	virtual bool SendData(const void* pData, unsigned int nLength) { return false; }
	virtual void Disconnect() { }

	virtual void SetUserData(void* pUserData) { m_pUserData = pUserData; }
	virtual void* GetUserData() { return m_pUserData; }

private:
	void* m_pUserData;
	unsigned m_nUID;
};

class CSessionServer: public ISessionServer
{
public:
	CSessionServer(ISessionServerCallback* pCallback) { m_pCallback = pCallback; }
	virtual ~CSessionServer() { }

	virtual bool Start(unsigned int nIp, unsigned int& nPort) { return false; }
	virtual void Stop() { }
	virtual void Wait() { }

private:
	ISessionServerCallback* m_pCallback;
};

ISessionServer* CreateSessionServer(ISessionServerCallback* pCallback)
{
	return new CSessionServer(pCallback);
}
