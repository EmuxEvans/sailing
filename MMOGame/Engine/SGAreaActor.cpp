#include <assert.h>
#include <list>

#include "Game.h"
#include "Math.h"
#include "VarCmd.h"

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

CSGAreaActor::CSGAreaActor(int nActorType, unsigned int nActorId) : CAreaActor<CSGArea, CSGAreaActor>(nActorId)
{
	m_nActorType = nActorType;
}

CSGAreaActor::~CSGAreaActor()
{
}

CSGPlayer::CSGPlayer(unsigned int nActorId) : CSGAreaActor(SGACTORTYPE_PLAYER, nActorId)
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

void CSGPlayer::OnMove(const Vector& vecDestination)
{
	printf("Player(%d) OnMove(%f, %f, %f) (%f, %f, %f)\n",
		GetActorId(),
		GetPosition().x, GetPosition().y, GetPosition().z, 
		vecDestination.x, vecDestination.y, vecDestination.z
		);
}
