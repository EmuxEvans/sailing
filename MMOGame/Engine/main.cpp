#include <stdio.h>
#include <assert.h>
#include <winsock2.h>
#include <map>

#include "asockio.h"

#include "CmdData.h"
#include "GameLoop.h"

#include "SG.h"
#include "SGGameLoop.h"

BOOL InitTCPServer(unsigned short nPort);
BOOL FinalTCPServer();
static IGameLoop* pLoop = NULL;

int main(int argc, char* argv[])
{
	//WSAData wsaData;
	//WSAStartup(MAKEWORD(2, 2), &wsaData);
	ASockIOInit();

	CGameFES::Init();
	CGameAPC::Init();
	GameLoop_Init();

	{
		IGameLoopCallback* pCallback;

		pCallback = new CSGGameLoopCallback();
		pLoop = GameLoop_Create(pCallback);
		pLoop->Start(100);

		InitTCPServer(1980);
		getchar();
		FinalTCPServer();

		pLoop->Stop();
		pLoop->Wait();
		delete pCallback;
	}

	GameLoop_Final();
	CGameAPC::Final();
	CGameFES::Final();

	ASockIOFini();
	//WSACleanup();
	return 0;
}

//
#define	MAX_UP_LEN		8192
#define	MAX_DOWN_LEN	65536

class CTCPClient;

static BOOL CALLBACK OnUserConnect(HCONNECT hconn, PSOCKADDR_IN, PSOCKADDR_IN psainRemote, LPVOID);
static VOID CALLBACK OnUserDisconnect(HCONNECT hConn , LPVOID pKey);
static VOID CALLBACK OnUserData(HCONNECT hConn, DWORD nLen, LPBYTE pBuf, LPVOID pKey);
static HEP end_point = NULL;
static HPOOL mem_pool = NULL;
static std::map<unsigned int, CTCPClient*> g_mapClients;
static CRITICAL_SECTION m_csClients;

class CTCPClient
{
public:
	CTCPClient(HCONNECT hConnect) {
		m_hConnect = hConnect;
		m_dwDataBufSize = 0;
		m_nUserId = 0;
		InitializeCriticalSection(&m_csI);
	}
	~CTCPClient() {
		DeleteCriticalSection(&m_csI);
	}

	void OnConnect() {
		m_bActive = TRUE;
	}
	void OnData(DWORD nSize, LPVOID pData) {
		if(!m_bActive) return;
	}
	void OnDisconnect() {
		m_bActive = FALSE;
		EnterCriticalSection(&m_csClients);
		Disconnect();
		LeaveCriticalSection(&m_csClients);
	}

	void Disconnect() {
		if(m_nUserId!=0) {
			std::map<unsigned int, CTCPClient*>::iterator i;
			i = g_mapClients.find(m_nUserId);
			if(i!=g_mapClients.end() && i->second==this) {
				pLoop->PushMsg(SGCMD_DISCONNECT, m_nUserId, NULL, 0);
				g_mapClients.erase(m_nUserId);
			}
			m_nUserId = 0;
		}

		if(m_bActive) {
			m_bActive = FALSE;
			::Disconnect(m_hConnect);
		}
	}

	void SetAuthUser(unsigned int nUserId) {
		EnterCriticalSection(&m_csClients);
		assert(m_nUserId==0);
		if(m_nUserId==0) {
			std::map<unsigned int, CTCPClient*>::iterator i;
			i = g_mapClients.find(m_nUserId);
			if(i!=g_mapClients.end()) {
				i->second->Disconnect();
			}
			g_mapClients[nUserId] = this;
			m_nUserId = nUserId;
		}
		LeaveCriticalSection(&m_csClients);
	}

private:
	unsigned int m_nUserId;
	BOOL m_bActive;
	char m_DataBuf[10*1024];
	DWORD m_dwDataBufSize;
	HCONNECT m_hConnect;
	CRITICAL_SECTION m_csI;
};

BOOL InitTCPServer(unsigned short nPort)
{
	mem_pool = AllocIoBufferPool(MAX_UP_LEN, 2048, MAX_DOWN_LEN, 1024);
	if(!mem_pool) {
		return(FALSE);
	}

	SOCKADDR_IN sain;
	ZeroMemory(&sain, sizeof(sain));
	sain.sin_family = AF_INET;
	sain.sin_port = htons(nPort);
	TCP_EP_HANDLER conn_handler = {
		OnUserConnect,
		OnUserDisconnect,
		OnUserData,
		NULL
	};
	TCP_OPTION tcpopt;
	GetDefTCPOpt(&tcpopt);
	tcpopt.sndbuf = 0;
	tcpopt.reuse_addr = TRUE;
	tcpopt.keep_alive = TRUE;
	tcpopt.nodelay = TRUE;
	end_point = RegisterTcpEndPoint(&sain, &conn_handler, &tcpopt, mem_pool, NULL);
	return(end_point!=NULL);
}

BOOL FinalTCPServer()
{
	if(end_point) {
		UnregisterEndPoint(end_point);
	}
	if(mem_pool) {
		FreeIoBufferPool(mem_pool);
	}
	return true;
}

BOOL CALLBACK OnUserConnect(HCONNECT hconn, PSOCKADDR_IN, PSOCKADDR_IN psainRemote, LPVOID)
{
	CTCPClient* pClient;
	pClient = new CTCPClient(hconn);
	SetConnectionKey(hconn, pClient);
	pClient->OnConnect();
	return TRUE;
}

VOID CALLBACK OnUserDisconnect(HCONNECT hConn , LPVOID pKey)
{
	CTCPClient* pClient;
	pClient = (CTCPClient*)pKey;
	pClient->OnDisconnect();
	delete pClient;
}

VOID CALLBACK OnUserData(HCONNECT hConn, DWORD nLen, LPBYTE pBuf, LPVOID pKey)
{
	CTCPClient* pClient;
	pClient = (CTCPClient*)pKey;
	pClient->OnData(nLen, pBuf);
}
