#include <stdio.h>
#include <assert.h>
#include <winsock2.h>
#include <map>
#include <vector>
#include <string>

#include "..\Engine\asockio.h"
#include "..\Engine\Game.h"

#include "..\SGCommon\SGCode.h"
#include "..\SGCommon\SGData.h"

#include "SGGameLoop.h"

static BOOL InitTCPServer(unsigned short nPort);
static BOOL FinalTCPServer();
static IMsgLoop* pLoop = NULL;
static bool bReplayMode = false;
static const char* OptGetValue(int argc, char* argv[], const char* name, const char* defvalue);
static bool OptGetSwitch(int argc, char* argv[], const char* name, bool defvalue);

int main(int argc, char* argv[])
{
	//WSAData wsaData;
	//WSAStartup(MAKEWORD(2, 2), &wsaData);
	ASockIOInit();
	MsgLoop_Init();
	Async_Init();

	if(OptGetValue(argc-1, &argv[1], "play", NULL)) {
		bReplayMode = true;
		pLoop = MsgLoop_Create();
		pLoop->Playback(CSGGameLoopCallback::GetSingleton(), OptGetValue(argc-1, &argv[1], "play", NULL));
	} else {
		pLoop = MsgLoop_Create();

		pLoop->Start(CSGGameLoopCallback::GetSingleton(), 1000, OptGetValue(argc-1, &argv[1], "record", NULL));

		InitTCPServer(1980);
		getchar();
		FinalTCPServer();

		pLoop->Stop();
		pLoop->Wait();
		CSGGameLoopCallback::Cleanup();
	}

	Async_Final();
	MsgLoop_Final();
	ASockIOFini();
	//WSACleanup();
	return 0;
}

const char* OptGetValue(int argc, char* argv[], const char* name, const char* defvalue)
{
	for(int i=0; i<argc; i++) {
		if(*argv[i]!='-') continue;
		char* filepst = strchr(argv[i], ':');
		if(filepst==NULL) continue;
		if(strlen(name)!=filepst-argv[i]-1) continue;
		if(memcmp(argv[i]+1, name, filepst-argv[i]-1)!=0) continue;
		return filepst+1;
	}
	return defvalue;
}

bool OptGetSwitch(int argc, char* argv[], const char* name, bool defvalue)
{
	return true;
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
static CTCPClient* g_mapClients[1000];
static CRITICAL_SECTION g_csClients[1000];
static unsigned int g_nClientSeq = 1980;

class CTCPClient
{
public:
	CTCPClient(HCONNECT hConnect, unsigned int nSeq) {
		m_hConnect = hConnect;
		m_nSeq = nSeq;
		m_nUserId = 0;
		m_bActive = FALSE;
		m_dwDataBufSize = 0;
		assert(g_mapClients[nSeq&0xffff]==NULL);
		g_mapClients[nSeq&0xffff] = this;
	}
	~CTCPClient() {
		assert(g_mapClients[m_nSeq&0xffff]==this);		
		g_mapClients[m_nSeq&0xffff] = NULL;		
	}

	unsigned int GetUserId() {
		return m_nUserId;
	}
	unsigned int GetSeq() {
		return m_nSeq;
	}

	void OnConnect(unsigned int nIP, unsigned short nPort) {
		m_bActive = TRUE;
		m_nIP = nIP;
		m_nPort = nPort;
		CDataBuffer<100> buf;
		unsigned char aSalt[10];
		buf.PutValue<unsigned short>(SGCMDCODE_LOGIN_SEED);
		memset(aSalt, 0xf3, sizeof(aSalt));
		buf.PutArray(aSalt, sizeof(aSalt));
		SendData(buf.GetBuffer(), buf.GetLength());
	}
	void OnData(DWORD nSize, LPVOID pData) {
		if(!m_bActive) return;

		if(sizeof(m_DataBuf)-m_dwDataBufSize<nSize) {
			Disconnect();
			return;
		}

		memcpy(m_DataBuf+m_dwDataBufSize, pData, nSize);
		m_dwDataBufSize += nSize;

		for(;;) {
			WORD len;
			if(m_dwDataBufSize<sizeof(len)) break;
			len = *((WORD*)m_DataBuf);
			if(m_dwDataBufSize<sizeof(len)+len) break;

			WORD code;
			if(sizeof(code)>len) {
				Disconnect();
				return;
			}
			code = *((WORD*)(m_DataBuf+sizeof(len)));

			switch(code) {
			case SGCMDCODE_LOGIN:
				if(m_nUserId) {
					Disconnect();
					return;
				}

				{
					unsigned int nUserId;
					const char* pUserName;
					const char* pPassword;
					CDataReader buf(m_DataBuf+sizeof(len)+sizeof(code), len-sizeof(code));
					buf.GetString(pUserName);
					buf.GetString(pPassword);
					nUserId = (unsigned int)atoi(pUserName);
					int ret = -1;
					if(strcmp(pPassword, "password")==0) {
						ret = 0;
						SetAuthUser(nUserId);
					}

					CDataBuffer<100> sbuf;
					sbuf.PutValue<unsigned short>(SGCMDCODE_LOGIN_RETURN);
					sbuf.PutValue(ret);
					SendData(sbuf.GetBuffer(), sbuf.GetLength());
				}
				break;
			default:
				if(!m_nUserId) {
					Disconnect();
					return;
				}
				pLoop->PushMsg(SGCMDCODE_USERDATA, m_nUserId, m_DataBuf+sizeof(len), len);
				break;
			}

			memmove(m_DataBuf, &m_DataBuf[sizeof(len)+len], m_dwDataBufSize-sizeof(len)-len);
			m_dwDataBufSize -= sizeof(len) + len;
		}
	}
	void OnDisconnect() {
		if(m_bActive && m_nUserId) {
			pLoop->PushMsg(SGCMDCODE_DISCONNECT, m_nUserId, NULL, 0);
		}

		m_bActive = FALSE;
	}

	void Disconnect() {
		if(m_bActive) {
			m_bActive = FALSE;
			::Disconnect(m_hConnect);
		}
	}
	void SendData(const void* pData, unsigned int nSize) {
		if(m_bActive) {
			LPBYTE pBuf = LockOutputBuffer(mem_pool);
			*((WORD*)pBuf) = (WORD)nSize;
			memcpy(pBuf+sizeof(WORD), pData, nSize);
			::SendData(m_hConnect, sizeof(WORD)+nSize, pBuf);
		}
	}

	void SetAuthUser(unsigned int nUserId) {
		assert(m_nUserId==0);
		m_nUserId = nUserId;
		CDataBuffer<100> sbuf;
		sbuf.PutValue<unsigned int>(0);
		sbuf.PutValue<unsigned short>(0);
		sbuf.PutValue(m_nSeq);
		pLoop->PushMsg(SGCMDCODE_CONNECT, nUserId, sbuf.GetBuffer(), sbuf.GetLength());
	}

private:
	HCONNECT m_hConnect;
	unsigned int m_nSeq;
	unsigned int m_nUserId;
	BOOL m_bActive;
	unsigned int m_nIP;
	unsigned short m_nPort;
	char m_DataBuf[10*1024];
	DWORD m_dwDataBufSize;
};

BOOL InitTCPServer(unsigned short nPort)
{
	for(int i=0; i<sizeof(g_csClients)/sizeof(g_csClients[0]); i++) {
		InitializeCriticalSectionAndSpinCount(&g_csClients[i], 0x80000400);
	}

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
	for(;;) {
		unsigned int count = 0;
		for(int i=0; i<sizeof(g_csClients)/sizeof(g_csClients[0]); i++) {
			EnterCriticalSection(&g_csClients[i]);
			if(g_mapClients[i]) {
				g_mapClients[i]->Disconnect();
				count++;
			}
			LeaveCriticalSection(&g_csClients[i]);
		}
		if(count==0) break;
		Sleep(10);
	}
	if(mem_pool) {
		FreeIoBufferPool(mem_pool);
	}
	for(int i=0; i<sizeof(g_csClients)/sizeof(g_csClients[0]); i++) {
		DeleteCriticalSection(&g_csClients[i]);
	}
	return true;
}

BOOL CALLBACK OnUserConnect(HCONNECT hconn, PSOCKADDR_IN, PSOCKADDR_IN psainRemote, LPVOID)
{
	for(unsigned int i=0; i<sizeof(g_csClients)/sizeof(g_csClients[0]); i++) {
		if(g_mapClients[i]==NULL) {
			EnterCriticalSection(&g_csClients[i]);
			if(g_mapClients[i]==NULL) {
				g_mapClients[i] = new CTCPClient(hconn, i | ((g_nClientSeq++)&0xffff)<<16);
				SetConnectionKey(hconn, g_mapClients[i]);
				g_mapClients[i]->OnConnect(psainRemote->sin_addr.S_un.S_addr, psainRemote->sin_port);
				LeaveCriticalSection(&g_csClients[i]);
				return TRUE;
			}
			LeaveCriticalSection(&g_csClients[i]);
		}
	}
	return TRUE;
}

VOID CALLBACK OnUserDisconnect(HCONNECT hConn , LPVOID pKey)
{
	CTCPClient* pClient;
	pClient = (CTCPClient*)pKey;
	if(pClient) {
		unsigned int nIndex = pClient->GetSeq() & 0xffff;
		EnterCriticalSection(&g_csClients[nIndex]);
		pClient->OnDisconnect();
		delete pClient;
		LeaveCriticalSection(&g_csClients[nIndex]);
	}
}

VOID CALLBACK OnUserData(HCONNECT hConn, DWORD nLen, LPBYTE pBuf, LPVOID pKey)
{
	CTCPClient* pClient;
	pClient = (CTCPClient*)pKey;
	if(pClient) {
		unsigned int nIndex = pClient->GetSeq() & 0xffff;
		EnterCriticalSection(&g_csClients[nIndex]);
		pClient->OnData(nLen, pBuf);
		LeaveCriticalSection(&g_csClients[nIndex]);
	}
}

void UserSendData(unsigned int nSeq, const void* pData, unsigned int nSize)
{
	if(bReplayMode) return;

	unsigned int nIndex = nSeq & 0xffff;
	if(nIndex>=sizeof(g_csClients)/sizeof(g_csClients[0])) return;
	EnterCriticalSection(&g_csClients[nIndex]);
	if(g_mapClients[nIndex]!=NULL && g_mapClients[nIndex]->GetSeq()==nSeq) {
		g_mapClients[nIndex]->SendData(pData, nSize);
	}
	LeaveCriticalSection(&g_csClients[nIndex]);
}

void UserDisconnect(unsigned int nSeq)
{
	if(bReplayMode) return;

	unsigned int nIndex = nSeq & 0xffff;
	if(nIndex>=sizeof(g_csClients)/sizeof(g_csClients[0])) return;
	EnterCriticalSection(&g_csClients[nIndex]);
	if(g_mapClients[nIndex]!=NULL && g_mapClients[nIndex]->GetSeq()==nSeq) {
		g_mapClients[nIndex]->Disconnect();
	}
	LeaveCriticalSection(&g_csClients[nIndex]);
}
