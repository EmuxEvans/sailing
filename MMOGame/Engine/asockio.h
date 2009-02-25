#pragma once

DECLARE_HANDLE(HPOOL);
DECLARE_HANDLE(HEP);
DECLARE_HANDLE(HCONNECT);

HPOOL AllocIoBufferPool(SIZE_T, UINT, SIZE_T, UINT);
VOID FreeIoBufferPool(HPOOL);
LPBYTE LockOutputBuffer(HPOOL);
VOID UnlockOutputBuffer(LPBYTE);
HPOOL GetPoolHandleOfEndPoint(HEP);
HPOOL GetPoolHandleOfConnection(HCONNECT);
HEP GetEndPointHandleOfConnection(HCONNECT);
DWORD GetConnPeerIp(HCONNECT);
VOID GetConnPeerEp(HCONNECT, DWORD&, WORD&);
VOID SetConnectionKey(HCONNECT, LPVOID);
LPVOID GetConnectionKey(HCONNECT);

typedef BOOL (CALLBACK *PFN_ON_CONNECT)(HCONNECT, PSOCKADDR_IN, PSOCKADDR_IN, LPVOID);
typedef VOID (CALLBACK *PFN_ON_DISCONNECT)(HCONNECT, LPVOID);
typedef VOID (CALLBACK *PFN_ON_DATA)(HCONNECT, DWORD, LPBYTE, LPVOID);
typedef VOID (CALLBACK *PFN_ON_CONNECTFAILED)(LPVOID);
typedef VOID (CALLBACK *PFN_ON_DATAGRAM)(HEP, PSOCKADDR_IN, DWORD, LPBYTE, LPVOID);

typedef struct _TCP_EP_HANDLER {
	PFN_ON_CONNECT			OnConnect;
	PFN_ON_DISCONNECT		OnDisconnect;
	PFN_ON_DATA				OnData;
	PFN_ON_CONNECTFAILED	OnConnectFailed;
} TCP_EP_HANDLER, *PTCP_EP_HANDLER;

typedef struct _UDP_EP_HANDLER {
	PFN_ON_DATAGRAM			OnDatagram;
} UDP_EP_HANDLER, *PUDP_EP_HANDLER;

typedef struct _TCP_OPTION {
	INT		recvbuf;
	INT		sndbuf;
	BOOL	reuse_addr;
	BOOL	conditional_accept;
	BOOL	keep_alive;
	INT		linger;
	BOOL	nodelay;
} TCP_OPTION, *PTCP_OPTION;

typedef struct _UDP_OPTION {
	INT		recvbuf;
	INT		sndbuf;
	BOOL	broadcast;
} UDP_OPTION, *PUDP_OPTION;

VOID GetDefTCPOpt(PTCP_OPTION);
VOID GetDefUDPOpt(PUDP_OPTION);
HEP RegisterTcpEndPoint(PSOCKADDR_IN, PTCP_EP_HANDLER, PTCP_OPTION, HPOOL, LPVOID);
HEP RegisterUdpEndPoint(PSOCKADDR_IN, PUDP_EP_HANDLER, PUDP_OPTION, HPOOL, LPVOID);
VOID UnregisterEndPoint(HEP);
BOOL Connect(PSOCKADDR_IN, PTCP_EP_HANDLER, PTCP_OPTION, HPOOL, LPVOID);
BOOL Disconnect(HCONNECT);
VOID SendData(HCONNECT, DWORD, LPBYTE);
VOID SendDatagram(HEP, PSOCKADDR_IN, DWORD, LPBYTE);
DWORD StringToIP(LPCTSTR);
BOOL ASockIOInit();
VOID ASockIOFini();
