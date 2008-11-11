#include <tchar.h>
#include <winsock2.h>
#include <process.h>
#include <crtdbg.h>

#include "TCPConnection.h"

#define WMNW_SEND			(WM_USER+1000)

typedef struct SENDBUF {
	WSAOVERLAPPED	overlap;
	int				buf_len;
	unsigned char	buf[1];
} SENDBUF;

static void CALLBACK SendAction(SOCKET hSock, SENDBUF* buf);
static void CALLBACK SendCallback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
static void NetworkThread(void*);
static DWORD g_dwNWThreadId	= 0;

void CTCPConnection::Init(void)
{
	_beginthread(NetworkThread, 0, NULL);
	while(g_dwNWThreadId==0) Sleep(5);
}

void CTCPConnection::Final(void)
{
	HANDLE hThread;
	hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, g_dwNWThreadId);
	_ASSERT(hThread!=NULL);
	PostThreadMessage(g_dwNWThreadId, WM_QUIT, 0, 0);
	WaitForSingleObject(hThread, INFINITE);
	g_dwNWThreadId = 0;
}

CTCPConnection::CTCPConnection(void)
{
	m_hSock = INVALID_SOCKET;
}

CTCPConnection::~CTCPConnection(void)
{
}

BOOL CTCPConnection::Connect(LPCTSTR pszEndpoint)
{
	_ASSERT(m_hSock==INVALID_SOCKET);
	if(m_hSock!=INVALID_SOCKET) return FALSE;

	TCHAR szBuf[100];
	SOCKADDR_IN sa;
	INT sa_len = sizeof(sa);
	_tcscpy(szBuf, pszEndpoint);
	if(WSAStringToAddress(szBuf, AF_INET, NULL, (LPSOCKADDR)&sa, &sa_len)==SOCKET_ERROR) return FALSE;

	m_hSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_hSock==INVALID_SOCKET) return FALSE;

	BOOL bFlag = TRUE;
	setsockopt(m_hSock, IPPROTO_TCP, TCP_NODELAY, (const char*)&bFlag, sizeof(bFlag));

	if(connect(m_hSock, (LPSOCKADDR)&sa, sizeof(sa))==SOCKET_ERROR) {
		closesocket(m_hSock); m_hSock = INVALID_SOCKET;
		return FALSE;
	}

	return TRUE;
}

void CTCPConnection::Disconnect()
{
	_ASSERT(m_hSock!=INVALID_SOCKET);
	if(m_hSock==INVALID_SOCKET) return;

	shutdown(m_hSock, SD_BOTH);
	closesocket(m_hSock);
	m_hSock = INVALID_SOCKET;
}

BOOL CTCPConnection::Wait(DWORD dwTimeOut)
{
	_ASSERT(m_hSock!=INVALID_SOCKET);
	if(m_hSock==INVALID_SOCKET) return -1;

	fd_set rd;
	int ret;
	FD_ZERO(&rd);
	FD_SET(m_hSock, &rd);

	if(dwTimeOut==0) {
		ret = select(0, &rd, NULL, NULL, NULL);
	} else {
		struct timeval tv;
		tv.tv_sec = dwTimeOut/1000;
		tv.tv_usec = (dwTimeOut%1000)*1000;
		ret = select(0, &rd, NULL, NULL, &tv);
	}
	return ret==1?TRUE:FALSE;
}

int CTCPConnection::Receive(void* buf, int buf_len)
{
	_ASSERT(m_hSock!=INVALID_SOCKET);
	if(m_hSock==INVALID_SOCKET) return -1;

	fd_set rd;
	struct timeval tv;
	FD_ZERO(&rd);
	FD_SET(m_hSock, &rd);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if(select(0, &rd, NULL, NULL, &tv)==1) {
		int ret = recv(m_hSock, (char*)buf, buf_len, 0);
		if(ret==0) return -1;
		return ret;
	} else {
		return 0;
	}
}

void CTCPConnection::Send(const void* buf, int buf_len)
{
	_ASSERT(m_hSock!=INVALID_SOCKET);
	if(m_hSock==INVALID_SOCKET) return;

	SENDBUF* sendbuf;
	sendbuf = (SENDBUF*)malloc(sizeof(SENDBUF)+buf_len);
	sendbuf->buf_len = buf_len;
	memcpy(sendbuf->buf, buf, buf_len);
	PostThreadMessage(g_dwNWThreadId, WMNW_SEND, (WPARAM)m_hSock, (LPARAM)sendbuf);
}

void CALLBACK SendAction(SOCKET hSock, SENDBUF* buf)
{
	WSABUF wsaBuf;
	ZeroMemory(&buf->overlap, sizeof(buf->overlap));
	wsaBuf.buf = (char*)buf->buf;
	wsaBuf.len = buf->buf_len;
	DWORD dwSendBytes;
	if(WSASend(hSock, &wsaBuf, 1, &dwSendBytes, 0, &buf->overlap, SendCallback)==SOCKET_ERROR) {
		if(WSAGetLastError()!=WSA_IO_PENDING) {
			free(buf);
		}
	} else {
//		_ASSERT(0);
	}

}

void CALLBACK SendCallback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	free(lpOverlapped);
}

void NetworkThread(void*)
{
	MSG msg;
	HANDLE hThread;

	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	g_dwNWThreadId	= GetCurrentThreadId();
	hThread = CreateEvent(NULL, FALSE, FALSE, NULL);
  
	for(;;) {
		DWORD dwRet;
		dwRet = MsgWaitForMultipleObjectsEx(1, &hThread, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
//		if(dwRet==WAIT_IO_COMPLETION) continue;

		for(;;) {
			if(!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) break;

			switch(msg.message) {
			case WM_QUIT:
				CloseHandle(hThread);
				return;
			case WMNW_SEND:
				SendAction((SOCKET)msg.wParam, (SENDBUF*)msg.lParam);
				break;
			default:
				_ASSERT(0);
			}
		}
	}
}
