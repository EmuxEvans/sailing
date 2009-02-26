#include <assert.h>
#include <map>

#include "Game.h"
#include "Math.h"
#include "CmdData.h"

#include "GameArea.h"
#include "GameArea.inl"
#include "GameBuff.h"
#include "GameItem.h"
#include "GameSkill.h"
#include "GameLoop.h"

#include "SG.h"
#include "SGArea.h"
#include "SGAreaActor.h"
#include "SGBuff.h"
#include "SGItem.h"
#include "SGSkill.h"
#include "SGGameLoop.h"

CSGAreaActor::CSGAreaActor(int nActorType) : CAreaActor<CSGArea, CSGAreaActor>(CSGGameLoopCallback::AllocActorId(this)), m_nActorType(nActorType)
{
}

CSGAreaActor::~CSGAreaActor()
{
}

CSGPlayer::CSGPlayer(unsigned int nUserId) : CSGAreaActor(SGACTORTYPE_PLAYER), m_nUserId(nUserId)
{
}

CSGPlayer::~CSGPlayer()
{
}

bool CSGPlayer::DropItem(int nIndex)
{
	assert(nIndex>=0 && nIndex<SGWAREHOUSE_COUNT);
	if(nIndex<0 || nIndex>=SGWAREHOUSE_COUNT) return false;
	memset(&m_Warehouse[nIndex], 0, sizeof(m_Warehouse[0]));
	return true;
}

bool CSGPlayer::Equip(int nIndex, int nSolt)
{
	assert(nIndex>=0 && nIndex<SGBAG_MAXCOUNT);
	if(nIndex<0 || nIndex>=SGBAG_MAXCOUNT) return false;
	assert(nSolt>=0 && nSolt<SGEQUIPMENT_COUNT);
	if(nSolt<0 || nSolt>=SGEQUIPMENT_COUNT) return false;

	if(m_Bag[nIndex].nItemId==0) {
		return false;
	}

	if(m_Equip[nSolt].nItemId!=0) {
		return false;
	}

	ItemSData* pSData = SGGetItemSData(m_Bag[nIndex].nItemId);
	if(!pSData) {
		return false;
	}

	CSGItemLogic* pLogic = SGGetItemLogic(pSData->nClassId);
	if(!pLogic) {
		return false;
	}

	return pLogic->Equip(this, nSolt, &m_Bag[nIndex], nIndex);
}

#include <stdio.h>

#define PLAYER_ACTION_NOTIFY_RANGE		1

void CSGPlayer::OnMove(const Vector& vecPosition, CAreaCell<CSGArea, CSGAreaActor>* pFrom, CAreaCell<CSGArea, CSGAreaActor>* pTo)
{
	assert(!(pFrom==NULL && pTo==NULL));

	printf("Player(%d) OnMove(%f, %f, %f)\n", GetActorId(),
		GetPosition().x, GetPosition().y, GetPosition().z
		);

	int x, y;
	CmdData cmdJoin = {SGCMD_PLAYER_JOIN, GetActorId(), NULL, 0};
	CmdData cmdLeave = {SGCMD_PLAYER_LEAVE, GetActorId(), NULL, 0};

	if(pFrom!=pTo) {
		if(pFrom) {
			for(x=pFrom->GetAreaCol()-PLAYER_ACTION_NOTIFY_RANGE; x<=pFrom->GetAreaCol()+PLAYER_ACTION_NOTIFY_RANGE; x++) {
				if(x<0 || x>=GetArea()->AreaColCount()) continue;
				for(y=pFrom->GetAreaRow()-PLAYER_ACTION_NOTIFY_RANGE; y<=pFrom->GetAreaRow()+PLAYER_ACTION_NOTIFY_RANGE; y++) {
					if(y<0 || y>=GetArea()->AreaRowCount()) continue;

					if(pTo) {
						if(x>=pTo->GetAreaCol()-PLAYER_ACTION_NOTIFY_RANGE && x<=pTo->GetAreaCol()+PLAYER_ACTION_NOTIFY_RANGE) {
						if(y>=pTo->GetAreaRow()-PLAYER_ACTION_NOTIFY_RANGE && y<=pTo->GetAreaRow()+PLAYER_ACTION_NOTIFY_RANGE) {
							continue;
						}
						}
					}

					GetArea()->GetCell(x, y)->Notify(&cmdLeave);
				}
			}
		}

		if(pTo) {
			for(x=pTo->GetAreaCol()-PLAYER_ACTION_NOTIFY_RANGE; x<=pTo->GetAreaCol()+PLAYER_ACTION_NOTIFY_RANGE; x++) {
				if(x<0 || x>=GetArea()->AreaColCount()) continue;
				for(y=pTo->GetAreaRow()-PLAYER_ACTION_NOTIFY_RANGE; y<=pTo->GetAreaRow()+PLAYER_ACTION_NOTIFY_RANGE; y++) {
					if(y<0 || y>=GetArea()->AreaRowCount()) continue;

					if(pFrom) {
						if(x>=pFrom->GetAreaCol()-PLAYER_ACTION_NOTIFY_RANGE && x<=pFrom->GetAreaCol()+PLAYER_ACTION_NOTIFY_RANGE) {
						if(y>=pFrom->GetAreaRow()-PLAYER_ACTION_NOTIFY_RANGE && y<=pFrom->GetAreaRow()+PLAYER_ACTION_NOTIFY_RANGE) {
							continue;
						}
						}
					}

					GetArea()->GetCell(x, y)->Notify(&cmdJoin);
				}
			}
		}
	}
}

void CSGPlayer::OnNotify(const CmdData* pCmdData)
{
	if(pCmdData->nCmd==SGCMD_PLAYER_MOVE) {
	}
	if(pCmdData->nCmd==SGCMD_PLAYER_JOIN) {
	}
	if(pCmdData->nCmd==SGCMD_PLAYER_LEAVE) {
	}
}

void CSGPlayer::OnAction(const CmdData* pCmdData)
{
	if(pCmdData->nCmd==SGCMD_PLAYER_MOVE) {
		Vector s, e;
		s.x = ((const SGCMD_PLAYER_MOVE_T*)(pCmdData->pData))->fFromX;
		s.y = ((const SGCMD_PLAYER_MOVE_T*)pCmdData->pData)->fFromY;
		s.z = ((const SGCMD_PLAYER_MOVE_T*)pCmdData->pData)->fFromZ;
		e.x = ((const SGCMD_PLAYER_MOVE_T*)pCmdData->pData)->fToX;
		e.y = ((const SGCMD_PLAYER_MOVE_T*)pCmdData->pData)->fToY;
		e.z = ((const SGCMD_PLAYER_MOVE_T*)pCmdData->pData)->fToZ;
		Move(&s, &e, ((const SGCMD_PLAYER_MOVE_T*)pCmdData->pData)->nTime);
		GetArea()->Notify(GetAreaCell()->GetAreaCol(), GetAreaCell()->GetAreaRow(), PLAYER_ACTION_NOTIFY_RANGE, pCmdData);
	}
}

void CSGPlayer::OnPassive(CSGAreaActor* pWho, const CmdData* pCmdData)
{
}

void CSGPlayer::Process(const CmdData* pCmdData)
{
	printf("Player(%d) Process Cmd=%d\n", pCmdData->nWho, pCmdData->nCmd);

	if(pCmdData->nCmd==SGCMD_CONNECT) {
		SetArea(CSGGameLoopCallback::GetSingleton()->GetArea(0));
		return;
	}
	if(pCmdData->nCmd==SGCMD_DISCONNECT) {
		SetArea(NULL);
		return;
	}
	if(pCmdData->nCmd==SGCMD_USERDATA) {
		unsigned short nEvent;

		if(pCmdData->nSize<sizeof(nEvent)) return;

		nEvent = *((unsigned short*)pCmdData->pData);
		if(nEvent==SGCMD_PLAYER_MOVE) {
			if(pCmdData->nSize==sizeof(nEvent)+sizeof(SGCMD_PLAYER_MOVE)) {
				CmdData CmdData;
				CmdData.nCmd = SGCMD_PLAYER_MOVE;
				CmdData.nWho = GetActorId();
				CmdData.pData = (char*)pCmdData->pData + sizeof(nEvent);
				CmdData.nSize = sizeof(SGCMD_PLAYER_MOVE);
				OnAction(&CmdData);
				return;
			}
		}

		return;
	}
}
