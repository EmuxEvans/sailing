#pragma once

class CTCPConnection
{
public:
	static void Init(void);
	static void Final(void);

	CTCPConnection(void);
	virtual ~CTCPConnection(void);

	BOOL Connect(LPCTSTR pszEndpoint);
	void Disconnect(void);

	BOOL Wait(DWORD dwTimeOut=0);
	int Receive(void* buf, int buf_len);
	void Send(const void* buf, int buf_len);

private:
	SOCKET m_hSock;

};
