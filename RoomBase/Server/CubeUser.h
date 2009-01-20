#pragma once

class CCubeUser : public CGameUser<CCubeUser>, public ICubeUser
{
public:
	void* operator new (size_t n);
	void operator delete(void* p);

public:
	CCubeUser(ICubeUserController* pController) : CGameUser<CCubeUser>(pController) {
		m_pHandle = NULL;
	}
	virtual ~CCubeUser() {
	}

	void BindNetworkHandle(NETWORK_HANDLE handle) {
		m_pHandle = handle;
	}

	//
	virtual void Disconnect() {
		if(m_pHandle)
			network_disconnect(m_pHandle);
	}
	virtual ICubeRoom* GetCubeRoom() {
		return NULL;
	}
	virtual CubeRoleInfo* GetRoleInfo() {
		return NULL;
	}

	char m_szRecvBuf[2*1024];
	NETWORK_HANDLE m_pHandle;
};

class CCubeUserController : public ICubeUserController {
public:
	virtual void OnConnect(CCubeUser* pUser);
	virtual void OnDisconnect(CCubeUser* pUser);
	virtual void OnData(CCubeUser* pUser, const void* pData, unsigned int nSize);
	virtual void SendData(CCubeUser* pUser, const void* pData, unsigned int nSize);
	virtual void Disconnect();
private:
	CServerLoginChannel m_LoginChannel;
};
