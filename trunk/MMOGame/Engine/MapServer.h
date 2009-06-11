#pragma once

class IMapArea;
class IMapServer;

class IMapArea
{
	friend class IMapServer;
public:
	virtual ~IMapArea() { }

	virtual void PreprocessCommand(const CmdData* pData, unsigned int& nLockId) = 0;
	virtual void OnComand(const CmdData* pData) = 0;
	virtual void OnTick(unsigned int nTime, unsigned int nDelta) = 0;

public:
	IMapServer* GetMapServer() { return m_pServer; }
	unsigned int GetAreaId() { return m_nAreaId; }
protected:
	void OnBindArea(IMapServer* pServer, unsigned int nAreaId) { m_pServer = pServer; m_nAreaId = nAreaId; }
	void OnUnbindArea(IMapServer* pServer, unsigned int nAreaId) { m_pServer = NULL; m_nAreaId = (unsigned int )-1; }
private:
	IMapServer* m_pServer;
	unsigned int m_nAreaId;
};

class IMapServer
{
public:
	virtual ~IMapServer();

	virtual bool Start(unsigned int nIp, unsigned int nPort, unsigned int nNumberOfThreads) = 0;
	virtual void Stop() = 0;
	virtual void Wait() = 0;

	virtual bool AddArea(unsigned int nAreaId, IMapArea* pArea) = 0;
	virtual bool DelArea(unsigned int nAreaId, IMapArea* pArea) = 0;
	virtual IMapArea* GetArea(unsigned int nAreaId) = 0;

	virtual unsigned int AddLock(unsigned int nArea1, unsigned int nArea2, unsigned int nArea3 = 0, unsigned int nArea4 = 0) = 0;
	virtual bool Lock(unsigned int nLockId) = 0;
	virtual bool Unlock(unsigned int nLockId) = 0;

protected:
	void BindArea(IMapArea* pArea, unsigned int nAreaId) { pArea->OnBindArea(this, nAreaId); }
	void UnbindArea(IMapArea* pArea, unsigned int nAreaId) { pArea->OnUnbindArea(this, nAreaId); }
};
