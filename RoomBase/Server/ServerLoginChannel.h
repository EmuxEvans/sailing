#pragma once

#define LOGINSTEP_START		0
#define LOGINSTEP_PUBKEY	1
#define LOGINSTEP_SESSION	2
#define LOGINSTEP_PROOF		3
#define LOGINSTEP_DONE		4
#define LOGINSTEP_ERROR		-1

class CServerLoginChannel : public CLoginServerBase, public CLoginServerHook, public ICubeChannel
{
public:
	CServerLoginChannel();
	~CServerLoginChannel();

	virtual bool Join(CCubeUser* pUser);
	virtual void OnData(CCubeUser* pUser, unsigned int nCIdx, const void* pData, unsigned int nSize);
	virtual void Disconnect(CCubeUser* pUser, unsigned int nCIdx);

protected:
	virtual char* GetSendBuf(unsigned int& nSendBufSize);
	virtual void SendBuf(const char* pSendBuf, unsigned int nSendBufSize);
protected:
	CLoginServerHook* m_pHooks;
	CCubeUser* m_pUser;
	unsigned short m_nUCIdx;

public:
	int GetLoginStep() { return step; }

	virtual void begin(char* username);
	virtual void verify(LoginPubkey* pubkey, LoginProof* proof);
private:
	srp6a_server_t srps;
	int step;
};
