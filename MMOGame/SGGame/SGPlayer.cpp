#include <string.h>
#include <assert.h>
#include <map>
#include <vector>
#include <string>

#include "..\Engine\Game.h"

#include "..\SGCommon\SGCode.h"
#include "..\SGCommon\SGData.h"

#include "SGArea.h"
#include "SGAreaActor.h"
#include "SGPlayer.h"
#include "SGPet.h"
#include "SGBuff.h"
#include "SGItem.h"
#include "SGSkill.h"
#include "SGGameLoop.h"
#include "SGBattleField.h"

void UserSendData(unsigned int nSeq, const void* pData, unsigned int nSize);
void UserDisconnect(unsigned int nSeq);

CSGPlayerTeam::CSGPlayerTeam()
{
	m_nLeader = 0;
	memset(m_Members, 0, sizeof(m_Members));
}

CSGPlayerTeam::~CSGPlayerTeam()
{
}

CSGPlayer* CSGPlayerTeam::GetMember(int nIndex)
{
	if(nIndex<0 || nIndex>=sizeof(m_Members)/sizeof(m_Members[0])) return NULL;
	return m_Members[nIndex];
}

bool CSGPlayerTeam::Join(CSGPlayer* pPlayer)
{
	int nIndex = -1;
	for(int l=0; l<sizeof(m_Members)/sizeof(m_Members[0]); l++) {
		if(!m_Members[l] && nIndex<0) nIndex = l;
		assert(m_Members[l]!=pPlayer);
		if(m_Members[l]==pPlayer) return false;
	}
	if(nIndex<0) return false;
	m_Members[nIndex] = pPlayer;
	if(!GetMember(m_nLeader)) m_nLeader = pPlayer->GetActorId();
	SyncData();
	return true;
}

bool CSGPlayerTeam::Leave(CSGPlayer* pPlayer)
{
	int nIndex;


	for(nIndex=0; nIndex<sizeof(m_Members)/sizeof(m_Members[0]); nIndex++) {
		if(m_Members[nIndex]==pPlayer) break;
	}
	if(nIndex>=sizeof(m_Members)/sizeof(m_Members[0])) return false;
	m_Members[nIndex] = NULL;

	unsigned int nLeader = 0;
	for(nIndex=0; nIndex<sizeof(m_Members)/sizeof(m_Members[0]); nIndex++) {
		if(m_Members[nIndex]) {
			nLeader = m_Members[nIndex]->GetActorId();
			break;
		}
	}
	if(nIndex==sizeof(m_Members)/sizeof(m_Members[0])) {
		delete this;
		return true;
	}
	if(GetMember(m_nLeader)==NULL) {
		m_nLeader = nLeader;
	}

	SyncData();
	return true;
}

void CSGPlayerTeam::OnData(CSGPlayer* pPlayer, const void* pData, unsigned int nSize)
{
}

void CSGPlayerTeam::SyncData()
{
	CDataBuffer<100> buf;
	buf.PutValue<unsigned short>(SGCMDCODE_TEAM_INFO);
	buf.PutValue(m_nLeader);
	for(int l=0; l<sizeof(m_Members)/sizeof(m_Members[0]); l++) {
		if(!m_Members[l]) continue;
		if(m_Members[l]->GetActorId()==m_nLeader) continue;
		buf.PutValue(m_Members[l]->GetActorId());
	}
	buf.PutValue<unsigned int>(0);
}

CSGPlayer::CSGPlayer(CSGConnection* pConnection, unsigned int nPlayerId) : CSGRole(SGACTORTYPE_PLAYER)
{
	m_pConnection = pConnection;
	m_nPlayerId = nPlayerId;
	sprintf(m_szName, "USER-%d", m_nPlayerId);
	m_pBattleField = NULL;
	m_pPlayerTeam = NULL;
	m_pPet = NULL;
	memset(m_Equip, 0, sizeof(m_Equip));
	memset(m_Bag, 0, sizeof(m_Bag));
	memset(m_Warehouse, 0, sizeof(m_Warehouse));

	m_pConnection->GetCallback()->OnPlayerAttach(GetPlayerId(), m_szName, this);
}

CSGPlayer::~CSGPlayer()
{
	m_pConnection->GetCallback()->OnPlayerDetach(GetPlayerId(), m_szName, this);

	assert(!m_pBattleField);
	assert(!m_pPlayerTeam);
	assert(!m_pPet);
}

bool CSGPlayer::InitPlayer()
{
	SetArea(GetConnection()->GetCallback()->GetArea(0));
	Vector p(10.0f, 10.0f, 0.0f);
	SetPosition(p, 0.0f);
	return true;
}

bool CSGPlayer::QuitPlayer()
{
	SetPositionNULL();
	SetArea(NULL);
	return true;
}

bool CSGPlayer::GetViewData(CSGPlayer* pPlayer, SGPLAYER_VIEWDATA* pData)
{
	memset(pData, 0, sizeof(*pData));
	strcpy(pData->name, this->GetName());
	sprintf(pData->guild, "%p", this);
	return true;
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
	if(pCmdData->nCmd==SGCMDCODE_BATTLEFIELD_JOIN) {
		assert(!m_pBattleField);
		m_pBattleField = (CSGBattleField*)(GetArea()->GetActor(pCmdData->nWho));
		assert(m_pBattleField);
		SetPositionNULL();
		return;
	}
	if(pCmdData->nCmd==SGCMDCODE_BATTLEFIELD_LEAVE) {
		assert(m_pBattleField);
		m_pBattleField = NULL;
		Move(NULL, NULL, 0);
		return;
	}

	if(pCmdData->nCmd==CMDCODE_MOVE_JOIN || pCmdData->nCmd==SGCMDCODE_MOVE_CHANGE) {
		CSGAreaActor* pActor = GetArea()->GetActor(pCmdData->nWho);
		assert(pActor);
		if(!pActor) return;

		CDataBuffer<1000> buf;

		switch(pActor->GetActorType()) {
		case SGACTORTYPE_PLAYER:
			{
				buf.PutValue<unsigned short>(pCmdData->nCmd==CMDCODE_MOVE_JOIN?SGCMDCODE_MOVE_JOIN_PLAYER:SGCMDCODE_MOVE_CHANGE_PLAYER);
				buf.PutValue(pActor->GetActorId());
				SGPLAYER_VIEWDATA viewdata;
				if(!((CSGPlayer*)pActor)->GetViewData(this, &viewdata)) return;
				buf.PutValue(viewdata);
				break;
			}
		case SGACTORTYPE_NPC:
			{
				buf.PutValue<unsigned short>(pCmdData->nCmd==CMDCODE_MOVE_JOIN?SGCMDCODE_MOVE_JOIN_NPC:SGCMDCODE_MOVE_CHANGE_NPC);
				buf.PutValue(pActor->GetActorId());
				SGNPC_VIEWDATA viewdata;
				if(!((CSGNPC*)pActor)->GetViewData(this, &viewdata)) return;
				buf.PutValue(viewdata);
				break;
			 }
		case SGACTORTYPE_PET:
			{
				buf.PutValue<unsigned short>(pCmdData->nCmd==CMDCODE_MOVE_JOIN?SGCMDCODE_MOVE_JOIN_PET:SGCMDCODE_MOVE_CHANGE_PET);
				buf.PutValue(pActor->GetActorId());
				SGPET_VIEWDATA viewdata;
				if(!((CSGPet*)pActor)->GetViewData(this, &viewdata)) return;
				buf.PutValue(viewdata);
				break;
			}
		case SGACTORTYPE_BATTLEFIELD:
			{
				buf.PutValue<unsigned short>(pCmdData->nCmd==CMDCODE_MOVE_JOIN?SGCMDCODE_MOVE_JOIN_BATTLE:SGCMDCODE_MOVE_CHANGE_BATTLE);
				buf.PutValue(pActor->GetActorId());
				SGBATTLEFIELD_VIEWDATA viewdata;
				if(!((CSGBattleField*)pActor)->GetViewData(this, &viewdata)) return;
				buf.PutValue(viewdata);
				break;
			}
		default:
			assert(0);
			return;
		}

		buf.PutValue(pActor->GetPosition().x);
		buf.PutValue(pActor->GetPosition().y);
		buf.PutValue(pActor->GetPosition().z);
		buf.PutValue(pActor->GetDestination().x);
		buf.PutValue(pActor->GetDestination().y);
		buf.PutValue(pActor->GetDestination().z);
		buf.PutValue(pActor->GetWalkTime());
		buf.PutValue(pActor->GetDirection());

		// send buf to client
		if(buf.GetLength()) {
			GetConnection()->SendData(buf.GetBuffer(), buf.GetLength());
		}
		return;
	}
	if(pCmdData->nCmd==CMDCODE_MOVE_LEAVE) {
		CSGAreaActor* pActor = GetArea()->GetActor(pCmdData->nWho);
		assert(pActor);
		if(!pActor) return;

		CDataBuffer<100> buf;
		buf.PutValue<unsigned short>(SGCMDCODE_MOVE_LEAVE);
		buf.PutValue(pCmdData->nWho);

		// send buf to client
		GetConnection()->SendData(buf.GetBuffer(), buf.GetLength());
		return;
	}

	if(pCmdData->nCmd==SGCMDCODE_MAPCHAT_SAY || pCmdData->nCmd==SGCMDCODE_MAPCHAT_MSAY) {
		CCmdDataReader cmd(pCmdData);
		CDataBuffer<100> buf;
		buf.PutValue<unsigned short>(pCmdData->nCmd);
		buf.PutString(cmd.GetValue<const char*>());
		buf.PutString(cmd.GetValue<const char*>());
		// send buf to client
		GetConnection()->SendData(buf.GetBuffer(), buf.GetLength());
		return;
	}
}

void CSGPlayer::OnAction(const CmdData* pCmdData)
{
	CCmdDataReader cmd(pCmdData);

	if(pCmdData->nCmd==SGCMDCODE_MOVE) {
		Vector s(cmd.GetValue<float>(), cmd.GetValue<float>(), cmd.GetValue<float>());
		Vector e(cmd.GetValue<float>(), cmd.GetValue<float>(), cmd.GetValue<float>());
		Move(&s, &e, cmd.GetValue<unsigned int>());
		GetArea()->Notify(pCmdData, GetAreaCell()->GetAreaCol(), GetAreaCell()->GetAreaRow());
		return;
	}
	if(pCmdData->nCmd==SGCMDCODE_MAPCHAT_SAY) {
		CCmdDataWriter<100> out(SGCMDCODE_MAPCHAT_SAY, GetActorId());
		out.PutValue(GetName());
		out.PutValue(cmd.GetString());
		GetArea()->Notify(out.GetCmdData(), GetAreaCell());
		return;
	}
	if(pCmdData->nCmd==SGCMDCODE_MAPCHAT_MSAY) {
		CSGPlayer* pPlayer = GetConnection()->GetCallback()->GetPlayer(cmd.GetString());
		CCmdDataWriter<100> out(SGCMDCODE_MAPCHAT_MSAY, GetActorId());
		if(pPlayer) {
			out.PutValue(GetName());
			out.PutValue(cmd.GetString());
			pPlayer->OnNotify(out.GetCmdData());
		}
		return;
	}
}

void CSGPlayer::OnPassive(const CmdData* pCmdData)
{
}
