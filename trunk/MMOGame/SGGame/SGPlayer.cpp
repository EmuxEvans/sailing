#include <string.h>
#include <assert.h>
#include <map>

#include "..\Engine\Game.h"

#include "SG.h"
#include "SGArea.h"
#include "SGAreaActor.h"
#include "SGPlayer.h"
#include "SGBuff.h"
#include "SGItem.h"
#include "SGSkill.h"
#include "SGGameLoop.h"

void UserSendData(unsigned int nSeq, const void* pData, unsigned int nSize);
void UserDisconnect(unsigned int nSeq);

CSGPlayer::CSGPlayer(IGameFES* pFES, unsigned int nUserId, FESClientData& ClientData) : CSGRole(SGACTORTYPE_PLAYER), m_pFES(pFES), m_nUserId(nUserId)
{
	memcpy(&m_ClientData, &ClientData, sizeof(ClientData));
}

CSGPlayer::~CSGPlayer()
{
}

bool CSGPlayer::DropItem(int nIndex)
{
	assert(nIndex>=0 && nIndex<SGWAREHOUSE_MAX);
	if(nIndex<0 || nIndex>=SGWAREHOUSE_MAX) return false;
	memset(&m_Warehouse[nIndex], 0, sizeof(m_Warehouse[0]));
	return true;
}

bool CSGPlayer::Equip(int nIndex, int nSolt)
{
	assert(nIndex>=0 && nIndex<SGBAG_MAX);
	if(nIndex<0 || nIndex>=SGBAG_MAX) return false;
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

void CSGPlayer::OnNotify(const CmdData* pCmdData)
{
	if(pCmdData->nCmd==CMDCODE_MOVE_JOIN) {
		if(pCmdData->nWho==GetActorId()) return;
		CSGAreaActor* pActor = GetArea()->GetActor(pCmdData->nWho);
		assert(pActor);
		if(!pActor) return;
		CDataBuffer<100> buf;
		buf.PutValue<unsigned short>(SGCMDCODE_MOVE_JOIN);
		buf.PutValue(pCmdData->nWho);
		SGROLE_VIEWDATA viewdata;
		if(!pActor->GetViewData(this, &viewdata)) return;
		buf.PutValue(viewdata);
		// send buf to client
		return;
	}
	if(pCmdData->nCmd==CMDCODE_MOVE_LEAVE) {
		if(pCmdData->nWho==GetActorId()) return;
		CSGAreaActor* pActor = GetArea()->GetActor(pCmdData->nWho);
		assert(pActor);
		if(!pActor) return;
		CDataBuffer<100> buf;
		buf.PutValue<unsigned short>(SGCMDCODE_MOVE_LEAVE);
		buf.PutValue(pCmdData->nWho);
		// send buf to client
		return;
	}
	if(pCmdData->nCmd==SGCMDCODE_MOVE_CHANGE) {
		if(pCmdData->nWho==GetActorId()) return;
		CSGAreaActor* pActor = GetArea()->GetActor(pCmdData->nWho);
		assert(pActor);
		if(!pActor) return;
		CDataBuffer<100> buf;
		buf.PutValue<unsigned short>(SGCMDCODE_MOVE_CHANGE);
		buf.PutValue(pCmdData->nWho);
		SGROLE_VIEWDATA viewdata;
		if(!pActor->GetViewData(this, &viewdata)) return;
		buf.PutValue(viewdata);
		// send buf to client
		return;
	}
}

void CSGPlayer::OnAction(const CmdData* pCmdData)
{
	if(pCmdData->nCmd==SGCMDCODE_MOVE) {
		CCmdDataReader cmd(pCmdData);
		Vector s(cmd.GetValue<float>(), cmd.GetValue<float>(), cmd.GetValue<float>());
		Vector e(cmd.GetValue<float>(), cmd.GetValue<float>(), cmd.GetValue<float>());
		Move(&s, &e, cmd.GetValue<unsigned int>());
		GetArea()->Notify(pCmdData, GetAreaCell()->GetAreaCol(), GetAreaCell()->GetAreaRow());
		return;
	}
}

void CSGPlayer::OnPassive(CSGAreaActor* pWho, const CmdData* pCmdData)
{
}

void CSGPlayer::Process(const CmdData* pCmdData)
{
	if(pCmdData->nCmd==SGCMDCODE_CONNECT) {
		SetArea(CSGGameLoopCallback::GetSingleton()->GetArea(0));
		return;
	}
	if(pCmdData->nCmd==SGCMDCODE_DISCONNECT) {
		SetArea(NULL);
		return;
	}
	if(pCmdData->nCmd==SGCMDCODE_USERDATA) {
		unsigned short nEvent;

		if(pCmdData->nSize<sizeof(nEvent)) return;
		nEvent = *((unsigned short*)pCmdData->pData);

		CmdData CmdData;
		CmdData.nCmd = SGCMDCODE_MOVE;
		CmdData.nWho = GetActorId();
		CmdData.pData = (char*)pCmdData->pData + sizeof(nEvent);
		CmdData.nSize = pCmdData->nSize - sizeof(nEvent);
		OnAction(&CmdData);
		return;
	}
}
