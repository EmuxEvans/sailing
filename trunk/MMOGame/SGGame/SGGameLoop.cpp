#include <map>
#include <string>
#include <assert.h>

#include "..\Engine\Game.h"

#include "SG.h"
#include "SGArea.h"
#include "SGAreaActor.h"
#include "SGPlayer.h"
#include "SGBuff.h"
#include "SGItem.h"
#include "SGSkill.h"
#include "SGGameLoop.h"

static CSGGameLoopCallback* g_pLoopCallback = NULL;
static CSGAreaActor* g_mapActors[2000] = { NULL };
static CSGArea g_GlobalArea;

extern void UserSendData(unsigned int nSeq, const void* pData, unsigned int nSize);
extern void UserDisconnect(unsigned int nSeq);

CSGConnection::CSGConnection(CSGGameLoopCallback* pCallback, unsigned int nUserId, IGameFES* pFES, unsigned int nFESSeq)
{
	m_pCallback = pCallback;
	m_nUserId = nUserId;
	m_pFES = pFES;
	m_nFESSeq = nFESSeq;
	m_pPlayer = NULL;
}

CSGConnection::~CSGConnection()
{
	assert(m_pPlayer);
}

void CSGConnection::Release()
{
	delete this;
}

bool CSGConnection::SendData(const void* pData, unsigned int nSize)
{
	UserSendData(m_nFESSeq, pData, nSize);
	return true;
}

bool CSGConnection::Disconnect()
{
	UserDisconnect(m_nFESSeq);
	return true;
}

void CSGConnection::OnConnect()
{
	m_pPlayer = new CSGPlayer(this, GetUserId());
	m_pPlayer->InitPlayer();
}

void CSGConnection::OnData(const void* pData, unsigned int nSize)
{
	assert(nSize>=sizeof(unsigned short));
	if(nSize<sizeof(unsigned short)) return;
	CmdData cmd;
	cmd.nCmd = (unsigned int)(*((unsigned short*)pData));
	cmd.nWho = m_pPlayer->GetPlayerId();
	cmd.pData = (const char*)pData + sizeof(unsigned short);
	cmd.nSize = nSize - sizeof(unsigned short);
	m_pPlayer->OnAction(&cmd);
}

void CSGConnection::OnDisconnect()
{
	m_pPlayer->QuitPlayer();
	delete m_pPlayer;
	m_pPlayer = NULL;
}

CSGGameLoopCallback* CSGGameLoopCallback::GetSingleton()
{
	if(!g_pLoopCallback)
		g_pLoopCallback = new CSGGameLoopCallback();
	return g_pLoopCallback;
}

void CSGGameLoopCallback::Cleanup()
{
	if(g_pLoopCallback) {
		delete g_pLoopCallback;
	}
}

unsigned int CSGGameLoopCallback::AllocActorId(CSGAreaActor* pActor)
{
	unsigned int i;
	for(i=0; i<sizeof(g_mapActors)/sizeof(g_mapActors[0]); i++) {
		if(!g_mapActors[i]) {
			g_mapActors[i] = pActor;
			return i;
		}
	}
	return 0xffffffff;
}

void CSGGameLoopCallback::FreeActorId(CSGAreaActor* pActor, unsigned int nActorId)
{
	assert(nActorId<sizeof(g_mapActors)/sizeof(g_mapActors[0]));
	if(nActorId>=sizeof(g_mapActors)/sizeof(g_mapActors[0])) return;
	assert(g_mapActors[nActorId]==pActor);
	g_mapActors[nActorId] = NULL;
}

CSGAreaActor* CSGGameLoopCallback::GetActor(unsigned int nActorId)
{
	assert(nActorId<sizeof(g_mapActors)/sizeof(g_mapActors[0]));
	if(nActorId>=sizeof(g_mapActors)/sizeof(g_mapActors[0])) return NULL;
	return g_mapActors[nActorId];
}

CSGGameLoopCallback::CSGGameLoopCallback()
{
}

CSGGameLoopCallback::~CSGGameLoopCallback()
{
}

CSGConnection* CSGGameLoopCallback::CreateConnection(unsigned int nUserId, IGameFES* pFES, unsigned int nFESSeq)
{
	return NULL;
}

CSGConnection* CSGGameLoopCallback::GetConnection(unsigned int nUserId)
{
	std::map<unsigned int, CSGConnection*>::iterator i;
	i = m_mapConnections.find(nUserId);
	return i==m_mapConnections.end()?NULL:i->second;
}

CSGPlayer* CSGGameLoopCallback::GetPlayer(unsigned int nUserId)
{
	std::map<unsigned int, CSGPlayer*>::iterator i;
	i = m_mapPlayersById.find(nUserId);
	return i==m_mapPlayersById.end()?i->second:NULL;
}

CSGPlayer* CSGGameLoopCallback::GetPlayer(const char* pName)
{
	std::map<std::string, CSGPlayer*>::iterator i;
	i = m_mapPlayersByName.find(pName);
	return i==m_mapPlayersByName.end()?i->second:NULL;
}

CSGArea* CSGGameLoopCallback::GetArea(unsigned int nAreaId)
{
	return &g_GlobalArea;
}

void CSGGameLoopCallback::Process(const CmdData* pCmdData)
{
	if(pCmdData->nCmd==SGCMDCODE_CONNECT) {
		assert(pCmdData->nWho!=0);
		CSGConnection* pConnection;
		pConnection = GetConnection(pCmdData->nWho);
		if(pConnection) {
			pConnection->Disconnect();
			pConnection->OnDisconnect();
			pConnection->Release();
		}
		CCmdDataReader cmd(pCmdData);
		IGameFES* pFES = GameLoop_GetFES(cmd.GetValue<unsigned int>(), cmd.GetValue<unsigned short>());
		assert(pFES);
		if(!pFES) return;
		pConnection = CreateConnection(pCmdData->nCmd, pFES, cmd.GetValue<unsigned int>());
		assert(pConnection);
		if(!pConnection) return;
		pConnection->OnConnect();
		return;
	}
	if(pCmdData->nCmd==SGCMDCODE_USERDATA) {
		assert(pCmdData->nWho!=0);
		CSGConnection* pConnection;
		pConnection = GetConnection(pCmdData->nWho);
		assert(pConnection);
		if(!pConnection) return;
		pConnection->OnData(pCmdData->pData, pCmdData->nSize);
		return;
	}
	if(pCmdData->nCmd==SGCMDCODE_DISCONNECT) {
		assert(pCmdData->nWho!=0);
		CSGConnection* pConnection;
		pConnection = GetConnection(pCmdData->nWho);
		assert(pConnection);
		if(!pConnection) return;
		pConnection->OnDisconnect();
		pConnection->Release();
		return;
	}
}

void CSGGameLoopCallback::Tick(unsigned int nCurrent, unsigned int nDelta)
{
}

void CSGGameLoopCallback::OnStart()
{
}

void CSGGameLoopCallback::OnShutdown()
{
}

void CSGGameLoopCallback::OnConnectionAttach(unsigned int nUserId, CSGConnection* pConnection)
{
	assert(m_mapConnections.find(nUserId)==m_mapConnections.end());
	m_mapConnections[nUserId] = pConnection;
}

void CSGGameLoopCallback::OnConnectionDetach(unsigned int nUserId, CSGConnection* pConnection)
{
	std::map<unsigned int, CSGConnection*>::iterator i;
	i = m_mapConnections.find(nUserId);
	assert(i!=m_mapConnections.end());
	if(i==m_mapConnections.end()) return;
	assert(i->second==pConnection);
	m_mapConnections.erase(i);
}

void CSGGameLoopCallback::OnPlayerAttach(unsigned int nPlayerId, const char* pName, CSGPlayer* pPlayer)
{
	assert(m_mapPlayersById.find(nPlayerId)==m_mapPlayersById.end());
	m_mapPlayersById[nPlayerId] = pPlayer;
	assert(m_mapPlayersByName.find(pName)==m_mapPlayersByName.end());
	m_mapPlayersByName[pName] = pPlayer;
}

void CSGGameLoopCallback::OnPlayerDetach(unsigned int nPlayerId, const char* pName, CSGPlayer* pPlayer)
{
	std::map<unsigned int, CSGPlayer*>::iterator i1;
	i1 = m_mapPlayersById.find(nPlayerId);
	assert(i1!=m_mapPlayersById.end());
	if(i1==m_mapPlayersById.end()) return;
	assert(i1->second==pPlayer);
	m_mapPlayersById.erase(i1);

	std::map<std::string, CSGPlayer*>::iterator i2;
	i2 = m_mapPlayersByName.find(pName);
	assert(i2!=m_mapPlayersByName.end());
	if(i2==m_mapPlayersByName.end()) return;
	assert(i2->second==pPlayer);
	m_mapPlayersByName.erase(i2);
}
