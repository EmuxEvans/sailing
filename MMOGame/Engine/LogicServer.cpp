#include "CmdData.h"
#include "LogicServer.h"

class CLogicServer : public ILogicServer
{
public:
	CLogicServer(ILogicServerCallback* pCallback) { m_pCallback = pCallback; }
	virtual ~CLogicServer() { }

	virtual bool Start(unsigned int nIp, unsigned int nPort) { return false; }
	virtual void Stop() { }
	virtual void Wait() { }

private:
	ILogicServerCallback* m_pCallback;
};

class CLogicClient : public ILogicClient
{
public:
	virtual ~CLogicClient() { }

	virtual bool SendCommand(const CmdData* pData) { return false; }
};

ILogicServer* CreateLogicServer(ILogicServerCallback* pCallback)
{
	return new CLogicServer(pCallback);
}

ILogicClient* AcquieLogicClient(unsigned int nIp, unsigned int nPort)
{
	return new CLogicClient();
}
