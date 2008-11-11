#pragma once

class COOKTCPConnection;

struct CLT_USER_CTX
{
	COOKTCPConnection*	pConn;
	MEM_STREAM			stream;
	unsigned char		data[1024];
};

#define OOKTCP_GET_DEFAULT_CTX() COOKTCPConnection::GetDefault()->GetUserCtx()

class COOKTCPConnection : protected CTCPConnection, public CubeHookDispatcher
{
public:
	COOKTCPConnection(BOOL bDefault=FALSE);
	virtual ~COOKTCPConnection(void);

	BOOL Connect(LPCTSTR pszAddr, BOOL bKeepAlive=FALSE);
	void Disconnect();

public:
	BOOL Dispatch(BOOL bBlock=FALSE);
private:
	char				m_szInBuf[100*1024];
	int					m_nInBufLen;
	char				m_szOutBuf[1*1024];

	//for PDL
public:
	CLT_USER_CTX* GetUserCtx() {
		return &m_ctxUser;
	}
	STREAM* NewStream() {
		memstream_init(&m_ctxUser.stream, m_ctxUser.data+sizeof(WORD), sizeof(m_ctxUser.data)-sizeof(WORD), 0);
		return (STREAM*)&m_ctxUser.stream;
	}
	void SendStream(STREAM* stream);
private:
	CLT_USER_CTX		m_ctxUser;

	// hooker
public:
	void Attach(CubeHook* hooker);
	void Detach(CubeHook* hokker);

protected:
	virtual CubeHook* FindReset();
	virtual CubeHook* FindNext();
	virtual CubeHook* FindGet();
private:
	CubeHook*		m_arHooker[50];
	int				m_nHookerCur;

public:
	static COOKTCPConnection* GetDefault() { return m_pDefault; }
private:
	static COOKTCPConnection* m_pDefault;

};
