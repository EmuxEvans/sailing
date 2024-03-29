#pragma once

class CCubeUser : public CGameUser<CCubeUser>, public ICubeUser
{
public:
	void* operator new (size_t n);
	void operator delete(void* p);

public:
	CCubeUser(ICubeUserController* pController, bool bTextMode);
	virtual ~CCubeUser();

	void BindNetworkHandle(NETWORK_HANDLE handle) {
		m_pHandle = handle;
	}

	virtual void OnData(const void* pData, unsigned int nSize);
	virtual void SendData(IGameChannel<CCubeUser>* pChannel, unsigned short nUCIdx, const void* pData, unsigned int nSize);
	virtual void Disconnect();

	virtual ICubeRoom* GetCubeRoom() {
		return NULL;
	}
	virtual CubeRoleInfo* GetRoleInfo() {
		return NULL;
	}

	char m_szRecvBuf[2*1024];
	NETWORK_HANDLE m_pHandle;
	bool m_bTextMode;
};

class CCubeUserController : public ICubeUserController {
public:
	virtual void OnInit(CCubeUser* pUser);
	virtual void OnFinal(CCubeUser* pUser);
	virtual void OnConnect(CCubeUser* pUser);
	virtual void OnDisconnect(CCubeUser* pUser);
	virtual void OnData(CCubeUser* pUser, const void* pData, unsigned int nSize);
	virtual void SendData(CCubeUser* pUser, IGameChannel<CCubeUser>* pChannel, unsigned short nUCIdx, const void* pData, unsigned int nSize);
	virtual void Disconnect(CCubeUser* pUser);
private:
	CServerLoginChannel m_LoginChannel;
};
