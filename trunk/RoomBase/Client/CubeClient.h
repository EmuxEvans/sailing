#pragma once

class CCubeClient;

template<class TClientBase, class TClientHook>
class CCubeClientWarp : public TClientBase
{
public:
	CCubeClientWarp(CCubeClient* pClient, bool bTextMode) : TClientBase(bTextMode, m_pHooks, sizeof(m_pHooks)/sizeof(m_pHooks[0])) {
		m_pClient = pClient;
		memset(m_pHooks, 0, sizeof(m_pHooks));
	}
	virtual char* GetSendBuf(unsigned int& nSendBufSize);
	virtual void SendBuf(const char* pSendBuf, unsigned int nSendBufSize);

	void SetHook(TClientHook* pHook);
	void Unhook(TClientHook* pHook);

	CCubeClient* GetClient() { return m_pClient; }
private:
	CCubeClient* m_pClient;
	TClientHook* m_pHooks[100];
};

class CCubeClient : public ICubeClient
{
public:
	CCubeClient();
	~CCubeClient();

	virtual os_int Connect(char* addr);
	virtual os_int Disconnect();
	virtual os_int IsConnected() { return 0; }
	virtual ILoginServer* GetLogin() { return &m_Login; }

	void OnConnect(NETWORK_HANDLE handle);
	void OnData();
	void OnDiconnect();

public:
	CCubeClientWarp<CLoginClientBase, CLoginClientHook> m_Login;
public:
	char* GetSendBuf(unsigned int& nSendBufSize) {
		nSendBufSize = sizeof(m_SendBuf);
		return m_SendBuf;
	}
	void SendBuf(const char* pSendBuf, unsigned int nSendBufSize) {
		NETWORK_DOWNBUF* downbufs[10];
		unsigned int count;
		int ret;

		count = network_downbufs_alloc(downbufs, sizeof(downbufs)/sizeof(downbufs[0]), 2 + nSendBufSize);
		ret = network_downbufs_fill(downbufs, count, 0, (const void*)&nSendBufSize, 2);
		assert(ret==ERR_NOERROR);
		ret = network_downbufs_fill(downbufs, count, 2, pSendBuf, nSendBufSize);
		assert(ret==ERR_NOERROR);
		ret = network_send(m_hHandle, downbufs, count);
		assert(ret==ERR_NOERROR);
		if(ret!=ERR_NOERROR) {
			network_downbufs_free(downbufs, count);
			return;
		}
	}
private:
	NETWORK_HANDLE m_hHandle;
	char m_SendBuf[1024];
	char m_RecvBuf[1024];
};

template<class TClientBase, class TClientHook>
char* CCubeClientWarp<TClientBase, TClientHook>::GetSendBuf(unsigned int& nSendBufSize)
{
	return m_pClient->GetSendBuf(nSendBufSize);
}

template<class TClientBase, class TClientHook>
void CCubeClientWarp<TClientBase, TClientHook>::SendBuf(const char* pSendBuf, unsigned int nSendBufSize)
{
}

template<class TClientBase, class TClientHook>
void CCubeClientWarp<TClientBase, TClientHook>::SetHook(TClientHook* pHook)
{
	for(int i=0; i<sizeof(m_pHooks)/sizeof(m_pHooks[0]); i++) {
		if(m_pHooks[i]==NULL) {
			m_pHooks[i] = pHook;
			return;
		}		
	}
}

template<class TClientBase, class TClientHook>
void CCubeClientWarp<TClientBase, TClientHook>::Unhook(TClientHook* pHook)
{
	for(int i=0; i<sizeof(m_pHooks)/sizeof(m_pHooks[0]); i++) {
		if(m_pHooks[i]==pHook) {
			m_pHooks[i] = NULL;
			return;
		}		
	}
}
