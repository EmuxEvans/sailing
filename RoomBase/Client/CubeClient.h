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
	virtual char* GetRecvBuf(unsigned int& nRecvBufSize);

	void SetHook(TClientHook* pHook);
	void Unhook(TClientHook* pHook);

	CCubeClient* GetClient() { return m_pClient; }
private:
	CCubeClient* m_pClient;
	TClientHook* m_pHooks[100];
};

class CCubeClient
{
public:
	CCubeClient();
	~CCubeClient();

	bool Connect(const char* pAddr);
	void Disconnect();

public:
	CCubeClientWarp<CLoginClientBase, CLoginClientHook> m_Login;

public:
	char* GetRecvBuf(unsigned int& nRecvBufSize) {
		nRecvBufSize = sizeof(m_RecvBuf);
		return m_RecvBuf;
	}
private:
	char m_RecvBuf[1024];
};

template<class TClientBase, class TClientHook>
char* CCubeClientWarp<TClientBase, TClientHook>::GetRecvBuf(unsigned int& nRecvBufSize)
{
	return m_pClient->GetRecvBuf(nRecvBufSize);
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
