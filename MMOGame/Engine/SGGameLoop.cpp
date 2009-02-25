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
		pPlayer = new CSGPlayer(pCmdData->nCmd);
		g_mapPlayers[pCmdData->nCmd] = pPlayer;
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
