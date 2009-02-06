#pragma once

class CServerLoginChannel : public ILoginServer, public ICubeChannel
{
public:
	CServerLoginChannel();
	~CServerLoginChannel();

	virtual bool Join(CCubeUser* pUser);
	virtual void OnData(CCubeUser* pUser, unsigned int nCIdx, const void* pData, unsigned int nSize);
	virtual void Disconnect(CCubeUser* pUser, unsigned int nCIdx);

	virtual void Login(char* username, char* password);
	virtual void Create(char* nickname);

private:
	CCubeUser* m_pUser;
	unsigned short m_nUCIdx;
	ILoginServer* m_pHooks;
	char m_szUserName[USERNAME_MAXLEN+1];
};
