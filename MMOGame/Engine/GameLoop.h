#pragma once

typedef struct CLIENT_FESEP_DATA {
} CLIENT_FESEP_DATA;

typedef struct CLIENT_FESEP {
	unsigned int i;
} CLIENT_FESEP;

class CGameFES;
class CGameLoop;

class CGameFES
{
public:
	CGameFES();
	virtual ~CGameFES();


};

class CGameLoop
{
public:
	CGameLoop();
	virtual ~CGameLoop();

	bool PushMsg(unsigned int nCmd, unsigned int nSource, unsigned int nTarget, const void* pData);
	void Run(unsigned int nMinTime);

protected:
	virtual void Process(const CmdData* pCmdData);

	virtual void OnStart();
	virtual void OnShutdown();

private:
	std::vector<CmdData>					m_MsgQ[2];
};
