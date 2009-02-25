#include <map>
#include <assert.h>

#include "CmdData.h"
#include "GameLoop.h"
#include "SGGameLoop.h"

#include "Game.h"
#include "Math.h"
#include "GameArea.h"
#include "GameArea.inl"
#include "GameBuff.h"
#include "GameItem.h"
#include "GameSkill.h"

#include "SG.h"
#include "SGArea.h"
#include "SGAreaActor.h"
#include "SGBuff.h"
#include "SGItem.h"
#include "SGSkill.h"

static std::map<unsigned int, CSGPlayer*> g_mapPlayers;
static CSGGameLoopCallback* g_pLoopCallback = NULL;
static CSGAreaActor* g_mapActors[2000] = { NULL };

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

CSGPlayer* CSGGameLoopCallback::GetPlayer(unsigned int nUserId)
{
	std::map<unsigned int, CSGPlayer*>::iterator i;
	i = g_mapPlayers.find(nUserId);
	return i==g_mapPlayers.end()?i->second:NULL;
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

void CSGGameLoopCallback::Process(const CmdData* pCmdData)
{
	if(pCmdData->nCmd==SGCMD_CONNECT) {
		assert(pCmdData->nWho!=0);
		std::map<unsigned int, CSGPlayer*>::iterator i;
		i = g_mapPlayers.find(pCmdData->nWho);
		if(i!=g_mapPlayers.end()) {
			CmdData CmdData = { SGCMD_DISCONNECT, pCmdData->nWho, NULL, 0 };
			i->second->Process(&CmdData);
			// send disconnect
		}
		CSGPlayer* pPlayer;
		pPlayer = new CSGPlayer(pCmdData->nWho);
		g_mapPlayers[pCmdData->nWho] = pPlayer;
		pPlayer->Process(pCmdData);
		return;
	}
	if(pCmdData->nCmd==SGCMD_USERDATA) {
		assert(pCmdData->nWho!=0);
		std::map<unsigned int, CSGPlayer*>::iterator i;
		i = g_mapPlayers.find(pCmdData->nWho);
		assert(i!=g_mapPlayers.end());
		if(i!=g_mapPlayers.end()) {
			i->second->Process(pCmdData);
		} else {
			// What can I do for you?
		}
		return;
	}
	if(pCmdData->nCmd==SGCMD_DISCONNECT) {
		assert(pCmdData->nWho!=0);
		std::map<unsigned int, CSGPlayer*>::iterator i;
		i = g_mapPlayers.find(pCmdData->nWho);
		assert(i!=g_mapPlayers.end());
		if(i!=g_mapPlayers.end()) {
			i->second->Process(pCmdData);
		} else {
			// What can I do for you?
		}
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
