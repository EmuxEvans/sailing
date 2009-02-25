#include <winsock2.h>
#include <mswsock.h>
#include <stdlib.h>
#include <tchar.h>

#include <list>

#include "asockio.h"

using namespace std;

#define	STATUS_SUCCESS		((DWORD)0x00000000)

#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#ifdef _DEBUG
#define new				DEBUG_NEW
#endif

#define	SIOP_IDLE		0
#define	SIOP_ACCEPT		1
#define	SIOP_CONNECT	2
#define	SIOP_DISCONNECT	3
#define	SIOP_RECV		4
#define	SIOP_SEND		5
#define	SIOP_RECVFROM	6
#define	SIOP_SENDTO		7

struct AIO_CONTEXT : WSAOVERLAPPED {
	INT						operation;
	HEP						hep;
	HCONNECT				hconn;
	HPOOL					hpool;
	WSABUF					wsabuf;
};
typedef AIO_CONTEXT *PAIO_CONTEXT;

#define	BufOfCtx(ctx)	((LPBYTE)ctx+sizeof(AIO_CONTEXT))
#define	CtxOfBuf(buf)	((PAIO_CONTEXT)((LPBYTE)buf-sizeof(AIO_CONTEXT)))

struct IO_BUFFER_POOL {
	SIZE_T input_size;
	DWORD input_count;
	list<PAIO_CONTEXT> input_buffers;
	CRITICAL_SECTION cs_input;
	SIZE_T output_size;
	DWORD output_count;
	list<PAIO_CONTEXT> output_buffers;
	CRITICAL_SECTION cs_output;
};
typedef IO_BUFFER_POOL *PIO_BUFFER_POOL;

HPOOL AllocIoBufferPool(SIZE_T isize, UINT icount, SIZE_T osize, UINT ocount)
{
	PIO_BUFFER_POOL ppool = new IO_BUFFER_POOL;
	if(ppool) {
		ppool->input_size = isize;
		for(ppool->input_count=0; ppool->input_count<icount; ++ppool->input_count) {
			ppool->input_buffers.push_back((PAIO_CONTEXT)malloc(sizeof(AIO_CONTEXT)+isize));
		}
		InitializeCriticalSectionAndSpinCount(&ppool->cs_input, 4000);
		ppool->output_size = osize;
		for(ppool->output_count=0; ppool->output_count<ocount; ++ppool->output_count) {
			ppool->output_buffers.push_back((PAIO_CONTEXT)malloc(sizeof(AIO_CONTEXT)+osize));
		}
		InitializeCriticalSectionAndSpinCount(&ppool->cs_output, 4000);
	}
	return((HPOOL)ppool);
}

VOID FreeIoBufferPool(HPOOL hpool)
{
	PIO_BUFFER_POOL ppool = (PIO_BUFFER_POOL)hpool;
	while(ppool->input_buffers.size()!=ppool->input_count) Sleep(0);
	while(ppool->output_buffers.size()!=ppool->output_count) Sleep(0);
	while(!ppool->input_buffers.empty()) {
		free(ppool->input_buffers.front());
		ppool->input_buffers.pop_front();
	}
	DeleteCriticalSection(&ppool->cs_input);
	while(!ppool->output_buffers.empty()) {
		free(ppool->output_buffers.front());
		ppool->output_buffers.pop_front();
	}
	DeleteCriticalSection(&ppool->cs_output);
	delete(ppool);
}

static LPBYTE LockInputBuffer(HPOOL hpool)
{
	PAIO_CONTEXT p = NULL;
	PIO_BUFFER_POOL ppool = (PIO_BUFFER_POOL)hpool;
	EnterCriticalSection(&ppool->cs_input);
	if(!ppool->input_buffers.empty()) {
		p = ppool->input_buffers.front();
		ppool->input_buffers.pop_front();
	}
	LeaveCriticalSection(&ppool->cs_input);
	if(!p) {
		p = (PAIO_CONTEXT)malloc(sizeof(AIO_CONTEXT)+ppool->input_size);
		if(!p) {
			return(NULL);
		} else {
			InterlockedIncrement((LONG *)&ppool->input_count);
		}
	}
	ZeroMemory(p, sizeof(AIO_CONTEXT));
	p->hpool = hpool;
	p->wsabuf.len = (ULONG)ppool->input_size;
	return((LPBYTE)(p->wsabuf.buf = (char *)BufOfCtx(p)));
}

static VOID UnlockInputBuffer(LPBYTE buf)
{
	PAIO_CONTEXT p = CtxOfBuf(buf);
	PIO_BUFFER_POOL ppool = (PIO_BUFFER_POOL)p->hpool;
	EnterCriticalSection(&ppool->cs_input);
	ppool->input_buffers.push_front(p);
	LeaveCriticalSection(&ppool->cs_input);
}

LPBYTE LockOutputBuffer(HPOOL hpool)
{
	PAIO_CONTEXT p = NULL;
	PIO_BUFFER_POOL ppool = (PIO_BUFFER_POOL)hpool;
	EnterCriticalSection(&ppool->cs_output);
	if(!ppool->output_buffers.empty()) {
		p = ppool->output_buffers.front();
		ppool->output_buffers.pop_front();
	}
	LeaveCriticalSection(&ppool->cs_output);
	if(!p) {
		p = (PAIO_CONTEXT)malloc(sizeof(AIO_CONTEXT)+ppool->output_size);
		if(!p) {
			return(NULL);
		} else {
			InterlockedIncrement((LONG *)&ppool->output_count);
		}
	}
	ZeroMemory(p, sizeof(AIO_CONTEXT));
	p->hpool = hpool;
	p->wsabuf.len = (ULONG)ppool->output_size;
	return((LPBYTE)(p->wsabuf.buf = (char *)BufOfCtx(p)));
}

VOID UnlockOutputBuffer(LPBYTE buf)
{
	PAIO_CONTEXT p = CtxOfBuf(buf);
	PIO_BUFFER_POOL ppool = (PIO_BUFFER_POOL)p->hpool;
	EnterCriticalSection(&ppool->cs_output);
	ppool->output_buffers.push_front(p);
	LeaveCriticalSection(&ppool->cs_output);
}

static BOOL debug_winsock = FALSE;
static BOOL need_broadcast = FALSE;

VOID GetDefTCPOpt(PTCP_OPTION popt)
{
	popt->recvbuf = -1;
	popt->sndbuf = -1;
	popt->reuse_addr = TRUE;
	popt->conditional_accept = FALSE;
	popt->keep_alive = FALSE;
	popt->linger = -1;
	popt->nodelay = FALSE;
}

VOID GetDefUDPOpt(PUDP_OPTION popt)
{
	popt->recvbuf = -1;
	popt->sndbuf = -1;
	popt->broadcast = need_broadcast;
}

static LPFN_ACCEPTEX lpfnAcceptEx = NULL;
static LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = NULL;
static LPFN_CONNECTEX lpfnConnectEx = NULL;
static LPFN_DISCONNECTEX lpfnDisconnectEx = NULL;

static VOID ApplyTcpOption(SOCKET s, PTCP_OPTION popt)
{
	if(debug_winsock) {
		setsockopt(s, SOL_SOCKET, SO_DEBUG, (const char *)&debug_winsock, sizeof(debug_winsock));
	}
	if(popt->recvbuf!=-1) {
		setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const char *)&popt->recvbuf, sizeof(popt->recvbuf));
	}
	if(popt->sndbuf!=-1) {
		setsockopt(s, SOL_SOCKET, SO_SNDBUF, (const char *)&popt->sndbuf, sizeof(popt->sndbuf));
	}
	if(popt->reuse_addr) {
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&popt->reuse_addr, sizeof(popt->reuse_addr));
	}
	if(popt->conditional_accept) {
		setsockopt(s, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (const char *)&popt->conditional_accept, sizeof(popt->conditional_accept));
	}
	if(popt->keep_alive) {
		setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const char *)&popt->keep_alive, sizeof(popt->keep_alive));
	}
	if(popt->linger!=-1) {
		LINGER linger;
		linger.l_onoff = 1;
		linger.l_linger = popt->linger;
		setsockopt(s, SOL_SOCKET, SO_LINGER, (const char *)&linger, sizeof(linger));
	} else {
		BOOL flag = TRUE;
		setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (const char *)&flag, sizeof(flag));
	}
	if(popt->nodelay) {
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char *)&popt->nodelay, sizeof(popt->nodelay));
	}
	DWORD dwBytes;
	if(!lpfnAcceptEx) {
		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx), &dwBytes, NULL, NULL);
	}
	if(!lpfnGetAcceptExSockaddrs) {
		GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
		WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockAddrs, sizeof(GuidGetAcceptExSockAddrs), &lpfnGetAcceptExSockaddrs, sizeof(lpfnGetAcceptExSockaddrs), &dwBytes, NULL, NULL);
	}
	if(!lpfnConnectEx) {
		GUID GuidConnectEx = WSAID_CONNECTEX;
		WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx), &lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, NULL, NULL);
	}
	if(!lpfnDisconnectEx) {
		GUID GuidDisconnectEx = WSAID_DISCONNECTEX;
		WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidDisconnectEx, sizeof(GuidDisconnectEx), &lpfnDisconnectEx, sizeof(lpfnDisconnectEx), &dwBytes, NULL, NULL);
	}
}

static VOID ApplyUdpOption(SOCKET s, PUDP_OPTION popt)
{
	DWORD dwBytesReturned = 0;
	BOOL bNewBehavior = FALSE;
	WSAIoctl(s, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);
	if(debug_winsock) {
		setsockopt(s, SOL_SOCKET, SO_DEBUG, (const char *)&debug_winsock, sizeof(debug_winsock));
	}
	if(popt->recvbuf!=-1) {
		setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const char *)&popt->recvbuf, sizeof(popt->recvbuf));
	}
	if(popt->sndbuf!=-1) {
		setsockopt(s, SOL_SOCKET, SO_SNDBUF, (const char *)&popt->sndbuf, sizeof(popt->sndbuf));
	}
	if(popt->broadcast) {
		setsockopt(s, SOL_SOCKET, SO_BROADCAST, (const char *)&popt->broadcast, sizeof(popt->broadcast));
	}
}

static DWORD total_oconns = 0;
static list<HCONNECT> free_oconns;
static CRITICAL_SECTION cs_free_olist;

struct CONNECTION {
	HEP						hep;
	SOCKET					s;
	HPOOL					pool;
	LPVOID					key;
	BOOL					rcving;
	TCP_EP_HANDLER			handler;
	VOID Send(DWORD, LPBYTE);
	VOID Recv(PAIO_CONTEXT = NULL);
};
typedef CONNECTION *PCONNECTION;

struct ACCEPT_CONTEXT : AIO_CONTEXT {
	BYTE	buf[(sizeof(SOCKADDR_IN)+16)*2];
};
typedef ACCEPT_CONTEXT *PACCEPT_CONTEXT;

struct TCP_END_POINT {
	BOOL					flag;
	SOCKET					s;
	HPOOL					pool;
	LPVOID					key;
	TCP_EP_HANDLER			handler;
	ACCEPT_CONTEXT			ctx;
	DWORD					total_conns;
	list<HCONNECT>			free_conns;
	CRITICAL_SECTION		cs_free_list;
	TCP_OPTION				tcpopt;
	VOID Accept();
	~TCP_END_POINT();
};
typedef TCP_END_POINT *PTCP_END_POINT;

struct UDP_END_POINT {
	BOOL					flag;
	SOCKET					s;
	HPOOL					pool;
	LPVOID					key;
	UDP_EP_HANDLER			handler;
	SOCKADDR_IN				sainSrc;
	INT						sainLen;
	VOID SendDatagram(PSOCKADDR_IN, DWORD, LPBYTE);
	VOID RecvFrom(PAIO_CONTEXT = NULL);
};
typedef UDP_END_POINT *PUDP_END_POINT;

HPOOL GetPoolHandleOfEndPoint(HEP hep)
{
	return(((PTCP_END_POINT)hep)->pool);
}

HPOOL GetPoolHandleOfConnection(HCONNECT hconn)
{
	return(((PCONNECTION)hconn)->pool);
}

HEP GetEndPointHandleOfConnection(HCONNECT hconn)
{
	return(((PCONNECTION)hconn)->hep);
}

DWORD GetConnPeerIp(HCONNECT hconn)
{
	SOCKADDR_IN sain;
	int sainlen = sizeof(sain);
	getpeername(((PCONNECTION)hconn)->s, (sockaddr *)&sain, &sainlen);
	return(sain.sin_addr.s_addr);
}

VOID GetConnPeerEp(HCONNECT hconn, DWORD& ip, WORD& port)
{
	SOCKADDR_IN sain;
	int sainlen = sizeof(sain);
	getpeername(((PCONNECTION)hconn)->s, (sockaddr *)&sain, &sainlen);
	ip = sain.sin_addr.s_addr;
	port = sain.sin_port;
}

VOID SetConnectionKey(HCONNECT hconn, LPVOID key)
{
	((PCONNECTION)hconn)->key = key;
}

LPVOID GetConnectionKey(HCONNECT hconn)
{
	return(((PCONNECTION)hconn)->key);
}

static VOID CALLBACK SocketIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
	PAIO_CONTEXT pctx = (PAIO_CONTEXT)lpOverlapped;
	switch(pctx->operation) {
		case SIOP_ACCEPT:
			{
				PACCEPT_CONTEXT pactx = (PACCEPT_CONTEXT)pctx;
				PTCP_END_POINT pep = (PTCP_END_POINT)pctx->hep;
				PCONNECTION pconn = (PCONNECTION)pctx->hconn;
				PSOCKADDR_IN psainLocal, psainRemote;
				INT llen, rlen;
				if(dwErrorCode==STATUS_SUCCESS) {
					setsockopt(pconn->s, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&pep->s, sizeof(pep->s));
					lpfnGetAcceptExSockaddrs(pactx->buf, 0, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, (LPSOCKADDR *)&psainLocal, &llen, (LPSOCKADDR *)&psainRemote, &rlen);
					pconn->rcving = TRUE;
					if(pconn->handler.OnConnect((HCONNECT)pconn, psainLocal, psainRemote, pconn->key)) {
						pconn->Recv();
					} else {
						pconn->rcving = FALSE;
						Disconnect((HCONNECT)pconn);
					}
				} else {
					EnterCriticalSection(&pep->cs_free_list);
					pep->free_conns.push_front((HCONNECT)pconn);
					LeaveCriticalSection(&pep->cs_free_list);
				}
				pep->Accept();
			}
			break;
		case SIOP_CONNECT:
			{
				PCONNECTION pconn = (PCONNECTION)pctx->hconn;
				SOCKADDR_IN sainLocal, sainRemote;
				INT llen, rlen;
				if(dwErrorCode==STATUS_SUCCESS) {
					setsockopt(pconn->s, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
					llen = rlen = sizeof(SOCKADDR_IN);
					getsockname(pconn->s, (PSOCKADDR)&sainLocal, &llen);
					getpeername(pconn->s, (PSOCKADDR)&sainRemote, &rlen);
					pconn->rcving = TRUE;
					if(pconn->handler.OnConnect((HCONNECT)pconn, &sainLocal, &sainRemote, pconn->key)) {
						pconn->Recv(pctx);
					} else {
						pconn->rcving = FALSE;
						Disconnect((HCONNECT)pconn);
						UnlockInputBuffer(BufOfCtx(pctx));
					}
				} else {
					pconn->handler.OnConnectFailed(pconn->key);
					EnterCriticalSection(&cs_free_olist);
					free_oconns.push_front((HCONNECT)pconn);
					LeaveCriticalSection(&cs_free_olist);
					UnlockInputBuffer(BufOfCtx(pctx));
				}
			}
			break;
		case SIOP_DISCONNECT:
			{
				PTCP_END_POINT pep = (PTCP_END_POINT)pctx->hep;
				PCONNECTION pconn = (PCONNECTION)pctx->hconn;
				if(dwErrorCode==STATUS_SUCCESS) {
					while(pconn->rcving) SwitchToThread();
					pconn->handler.OnDisconnect((HCONNECT)pconn, pconn->key);
					if(pep) {
						EnterCriticalSection(&pep->cs_free_list);
						pep->free_conns.push_front((HCONNECT)pconn);
						LeaveCriticalSection(&pep->cs_free_list);
					} else {
						EnterCriticalSection(&cs_free_olist);
						free_oconns.push_front((HCONNECT)pconn);
						LeaveCriticalSection(&cs_free_olist);
					}
				}
				UnlockOutputBuffer(BufOfCtx(pctx));
			}
			break;
		case SIOP_RECV:
			{
				PCONNECTION pconn = (PCONNECTION)pctx->hconn;
				if(dwErrorCode==STATUS_SUCCESS && dwNumberOfBytesTransfered) {
					pconn->handler.OnData((HCONNECT)pconn, dwNumberOfBytesTransfered, BufOfCtx(pctx), pconn->key);
					pconn->Recv(pctx);
				} else {
					pconn->rcving = FALSE;
					Disconnect((HCONNECT)pconn);
					UnlockInputBuffer(BufOfCtx(pctx));
				}
			}
			break;
		case SIOP_RECVFROM:
			{
				PUDP_END_POINT pep = (PUDP_END_POINT)pctx->hep;
				if(dwErrorCode==STATUS_SUCCESS && dwNumberOfBytesTransfered) {
					pep->handler.OnDatagram((HEP)pep, &pep->sainSrc, dwNumberOfBytesTransfered, BufOfCtx(pctx), pep->key);
					pep->RecvFrom(pctx);
				} else {
					UnlockInputBuffer(BufOfCtx(pctx));
				}
			}
			break;
		case SIOP_SEND:
		case SIOP_SENDTO:
			UnlockOutputBuffer(BufOfCtx(pctx));
			break;
	}
}

VOID CONNECTION::Send(DWORD dwLen, LPBYTE buf)
{
	DWORD dwXfer;
	PAIO_CONTEXT pctx = CtxOfBuf(buf);
	pctx->operation = SIOP_SEND;
	pctx->hep = hep;
	pctx->hconn = (HCONNECT)this;
	pctx->wsabuf.len = dwLen;
	if(SOCKET_ERROR==WSASend(s, &pctx->wsabuf, 1, &dwXfer, 0, pctx, NULL) && WSAGetLastError()!=ERROR_IO_PENDING) {
		UnlockOutputBuffer(buf);
	}
}

VOID CONNECTION::Recv(PAIO_CONTEXT pctx)
{
	DWORD dwXfer, dwFlag = 0;
	LPBYTE buf;
	if(pctx) {
		buf = BufOfCtx(pctx);
	} else {
		buf = LockInputBuffer(pool);
		pctx = CtxOfBuf(buf);
	}
	pctx->operation = SIOP_RECV;
	pctx->hep = hep;
	pctx->hconn = (HCONNECT)this;
	if(SOCKET_ERROR==WSARecv(s, &pctx->wsabuf, 1, &dwXfer, &dwFlag, pctx, NULL) && WSAGetLastError()!=ERROR_IO_PENDING) {
		rcving = FALSE;
		UnlockInputBuffer(buf);
	}
}

VOID TCP_END_POINT::Accept()
{
	PCONNECTION pconn = NULL;
	DWORD dwXfer;
	EnterCriticalSection(&cs_free_list);
	if(!free_conns.empty()) {
		pconn = (PCONNECTION)free_conns.front();
		free_conns.pop_front();
	}
	LeaveCriticalSection(&cs_free_list);
	if(!pconn) {
		pconn = new CONNECTION;
		if(!pconn) return;
		pconn->hep = (HEP)this;
		pconn->s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(pconn->s==INVALID_SOCKET) {
			delete(pconn);
			return;
		}
		ApplyTcpOption(pconn->s, &tcpopt);
		if(!BindIoCompletionCallback((HANDLE)pconn->s, SocketIOCompletionRoutine, 0)) {
			closesocket(pconn->s);
			delete(pconn);
			return;
		}
		++total_conns;
		pconn->pool = pool;
		pconn->handler = handler;
	}
	pconn->key = key;
	ZeroMemory(&ctx, sizeof(ctx));
	ctx.operation = SIOP_ACCEPT;
	ctx.hep = (HEP)this;
	ctx.hconn = (HCONNECT)pconn;
	if(!lpfnAcceptEx(s, pconn->s, ctx.buf, 0, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwXfer, &ctx) && WSAGetLastError()!=ERROR_IO_PENDING) {
		EnterCriticalSection(&cs_free_list);
		free_conns.push_front((HCONNECT)pconn);
		LeaveCriticalSection(&cs_free_list);
	}
}

TCP_END_POINT::~TCP_END_POINT()
{
	PCONNECTION pconn;
	while(!free_conns.empty()) {
		pconn = (PCONNECTION)free_conns.front();
		free_conns.pop_front();
		closesocket(pconn->s);
		delete(pconn);
	}
}

VOID UDP_END_POINT::SendDatagram(PSOCKADDR_IN psain, DWORD dwLen, LPBYTE buf)
{
	DWORD dwXfer;
	PAIO_CONTEXT pctx = CtxOfBuf(buf);
	pctx->operation = SIOP_SENDTO;
	pctx->hep = (HEP)this;
	pctx->wsabuf.len = dwLen;
	if(SOCKET_ERROR==WSASendTo(s, &pctx->wsabuf, 1, &dwXfer, 0, (PSOCKADDR)psain, sizeof(SOCKADDR_IN), pctx, NULL) && WSAGetLastError()!=ERROR_IO_PENDING) {
		UnlockOutputBuffer(buf);
	}
}

VOID UDP_END_POINT::RecvFrom(PAIO_CONTEXT pctx)
{
	DWORD dwXfer, dwFlag = 0;
	LPBYTE buf;
	if(pctx) {
		buf = BufOfCtx(pctx);
	} else {
		buf = LockInputBuffer(pool);
		pctx = CtxOfBuf(buf);
	}
	pctx->operation = SIOP_RECVFROM;
	pctx->hep = (HEP)this;
	sainLen = sizeof(sainSrc);
	if(SOCKET_ERROR==WSARecvFrom(s, &pctx->wsabuf, 1, &dwXfer, &dwFlag, (PSOCKADDR)&sainSrc, &sainLen, pctx, NULL) && WSAGetLastError()!=ERROR_IO_PENDING) {
		UnlockInputBuffer(buf);
	}
}

HEP RegisterTcpEndPoint(PSOCKADDR_IN psain, PTCP_EP_HANDLER phandler, PTCP_OPTION popt, HPOOL hpool, LPVOID key)
{
	PTCP_END_POINT pep = NULL;

	if(!psain || !phandler || !hpool) return(NULL);
	pep = new TCP_END_POINT;
	if(pep) {
		pep->s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(pep->s==INVALID_SOCKET) {
			delete(pep);
			pep = NULL;
		} else {
			if(popt) {
				pep->tcpopt = *popt;
			} else {
				GetDefTCPOpt(&pep->tcpopt);
			}
			ApplyTcpOption(pep->s, &pep->tcpopt);
			if(!BindIoCompletionCallback((HANDLE)pep->s, SocketIOCompletionRoutine, 0) ||
				bind(pep->s, (PSOCKADDR)psain, sizeof(SOCKADDR_IN))==SOCKET_ERROR ||
				listen(pep->s, SOMAXCONN)==SOCKET_ERROR ||
				!InitializeCriticalSectionAndSpinCount(&pep->cs_free_list, 4000)) {
					closesocket(pep->s);
					delete(pep);
					pep = NULL;
			} else {
				pep->flag = TRUE;
				pep->pool = hpool;
				pep->key = key;
				pep->handler = *phandler;
				pep->total_conns = 0;
				pep->Accept();
			}
		}
	}
	return((HEP)pep);
}

static VOID UnregisterTcpEndPoint(PTCP_END_POINT ptep)
{
	closesocket(ptep->s);
	DWORD timeleft = 15000;
	while(ptep->free_conns.size()!=ptep->total_conns && timeleft) {
		Sleep(100);
		timeleft -= 100;
	}
	DeleteCriticalSection(&ptep->cs_free_list);
	delete(ptep);
}

HEP RegisterUdpEndPoint(PSOCKADDR_IN psain, PUDP_EP_HANDLER phandler, PUDP_OPTION popt, HPOOL hpool, LPVOID key)
{
	PUDP_END_POINT pep = NULL;
	UDP_OPTION opt;

	if(!psain || !phandler || !hpool) return(NULL);
	pep = new UDP_END_POINT;
	if(pep) {
		pep->s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(pep->s==INVALID_SOCKET) {
			delete(pep);
			pep = NULL;
		} else {
			if(popt) {
				ApplyUdpOption(pep->s, popt);
			} else {
				GetDefUDPOpt(&opt);
				ApplyUdpOption(pep->s, &opt);
			}
			if(!BindIoCompletionCallback((HANDLE)pep->s, SocketIOCompletionRoutine, 0) ||
				bind(pep->s, (PSOCKADDR)psain, sizeof(SOCKADDR_IN))==SOCKET_ERROR) {
					closesocket(pep->s);
					delete(pep);
					pep = NULL;
			} else {
				pep->flag = FALSE;
				pep->pool = hpool;
				pep->key = key;
				pep->handler = *phandler;
				pep->RecvFrom();
			}
		}
	}
	return((HEP)pep);
}

static VOID UnregisterUdpEndPoint(PUDP_END_POINT puep)
{
	closesocket(puep->s);
	delete(puep);
}

VOID UnregisterEndPoint(HEP hep)
{
	PTCP_END_POINT ptep = (PTCP_END_POINT)hep;
	PUDP_END_POINT puep = (PUDP_END_POINT)hep;
	if(ptep->flag) {
		UnregisterTcpEndPoint(ptep);
	} else {
		UnregisterUdpEndPoint(puep);
	}
}

BOOL Connect(PSOCKADDR_IN psain, PTCP_EP_HANDLER phandler, PTCP_OPTION popt, HPOOL hpool, LPVOID key)
{
	TCP_OPTION opt;
	SOCKADDR_IN sain;
	PCONNECTION pconn = NULL;

	if(!psain || !phandler || !hpool) return(FALSE);
	EnterCriticalSection(&cs_free_olist);
	if(!free_oconns.empty()) {
		pconn = (PCONNECTION)free_oconns.front();
		free_oconns.pop_front();
	}
	LeaveCriticalSection(&cs_free_olist);
	if(!pconn) {
		pconn = new CONNECTION;
		if(!pconn) return(FALSE);
		pconn->s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(pconn->s==INVALID_SOCKET) {
			delete(pconn);
			return(FALSE);
		}
		if(popt) {
			ApplyTcpOption(pconn->s, popt);
		} else {
			GetDefTCPOpt(&opt);
			ApplyTcpOption(pconn->s, &opt);
		}
		pconn->hep = NULL;
		ZeroMemory(&sain, sizeof(sain));
		sain.sin_family = AF_INET;
		if(!BindIoCompletionCallback((HANDLE)pconn->s, SocketIOCompletionRoutine, 0) ||
			bind(pconn->s, (PSOCKADDR)&sain, sizeof(sain))==SOCKET_ERROR) {
				closesocket(pconn->s);
				delete(pconn);
				return(FALSE);
		}
		InterlockedIncrement((LONG *)&total_oconns);
	}
	pconn->pool = hpool;
	pconn->key = key;
	pconn->handler = *phandler;
	LPBYTE buf = LockInputBuffer(hpool);
	PAIO_CONTEXT pctx = CtxOfBuf(buf);
	pctx->operation = SIOP_CONNECT;
	pctx->hconn = (HCONNECT)pconn;
	if(!lpfnConnectEx(pconn->s, (PSOCKADDR)psain, sizeof(SOCKADDR_IN), NULL, 0, NULL, pctx) && WSAGetLastError()!=ERROR_IO_PENDING) {
		UnlockInputBuffer(buf);
		EnterCriticalSection(&cs_free_olist);
		free_oconns.push_front((HCONNECT)pconn);
		LeaveCriticalSection(&cs_free_olist);
		return(FALSE);
	}
	return(TRUE);
}

BOOL Disconnect(HCONNECT hconn)
{
	PCONNECTION pconn = (PCONNECTION)hconn;
	LPBYTE buf = LockOutputBuffer(pconn->pool);
	PAIO_CONTEXT pctx = CtxOfBuf(buf);
	pctx->operation = SIOP_DISCONNECT;
	pctx->hep = pconn->hep;
	pctx->hconn = hconn;
	if(!lpfnDisconnectEx(pconn->s, pctx, TF_REUSE_SOCKET, 0) && WSAGetLastError()!=ERROR_IO_PENDING) {
		UnlockOutputBuffer(buf);
		return(FALSE);
	}
	return(TRUE);
}

VOID SendData(HCONNECT hconn, DWORD dwLen, LPBYTE buf)
{
	((PCONNECTION)hconn)->Send(dwLen, buf);
}

VOID SendDatagram(HEP hep, PSOCKADDR_IN psain, DWORD dwLen, LPBYTE buf)
{
	((PUDP_END_POINT)hep)->SendDatagram(psain, dwLen, buf);
}

BOOL ASockIOInit()
{
	InitializeCriticalSectionAndSpinCount(&cs_free_olist, 4000);
	WSADATA wsaData;
	return(0==WSAStartup(0x0202, &wsaData));
}

VOID ASockIOFini()
{
	while(free_oconns.size()!=total_oconns) Sleep(0);
	PCONNECTION pconn;
	while(!free_oconns.empty()) {
		pconn = (PCONNECTION)free_oconns.front();
		free_oconns.pop_front();
		closesocket(pconn->s);
		delete(pconn);
	}
	WSACleanup();
	DeleteCriticalSection(&cs_free_olist);
}

DWORD StringToIP(LPCTSTR szHost)
{
	CHAR buf[256];

#ifdef _UNICODE
	wcstombs(buf, szHost, 256);
#else
	strcpy(buf, szHost);
#endif

	DWORD ret = inet_addr(buf);
	if(ret==INADDR_NONE) {
		hostent *phe = gethostbyname(buf);
		if(phe) {
			ret = *(DWORD *)(phe->h_addr);
		}
	}
	return(ret);
}
