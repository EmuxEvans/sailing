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

CSGTeam::CSGTeam()
{
	m_nLeader = 0;
	memset(m_Members, 0, sizeof(m_Members));
}

CSGTeam::~CSGTeam()
{
}

CSGPlayer* CSGTeam::GetMember(int nIndex)
{
	if(nIndex<0 || nIndex>=sizeof(m_Members)/sizeof(m_Members[0])) return NULL;
	return m_Members[nIndex];
}

CSGPlayer* CSGTeam::GetMember(unsigned int nActorId)
{
	for(int l=0; l<sizeof(m_Members)/sizeof(m_Members[0]); l++) {
		if(m_Members[l] && m_Members[l]->GetActorId()==nActorId) return m_Members[l];
	}
	return NULL;
}

bool CSGTeam::Join(CSGPlayer* pPlayer)
{
	int nIndex = -1;
	unsigned short count = 0;
	for(int l=0; l<sizeof(m_Members)/sizeof(m_Members[0]); l++) {
		if(!m_Members[l]) {
			if(nIndex<0) nIndex = l;
		} else {
			count++;
		}
		assert(m_Members[l]!=pPlayer);
		if(m_Members[l]==pPlayer) return false;
	}
	if(nIndex<0) return false;

	CDataBuffer<20+sizeof(SGPLAYER_VIEWDATA_INTEAM)*SGPLAYERTEAM_MEMBER_MAX> buf;
	SGPLAYER_VIEWDATA_INTEAM viewdata;

	// send member join message for everyone
	pPlayer->GetViewDataInTeam(&viewdata);
	buf.PutValue<unsigned short>(SGCMDCODE_TEAM_CHANGE);
	buf.PutStruct(&viewdata);
	SendData(buf.GetBuffer(), buf.GetLength());

	// send member infos to pPlayer
	buf.Reset();
	buf.PutValue<unsigned short>(SGCMDCODE_TEAM_INFO);
	buf.PutValue(m_nLeader);
	buf.PutValue(count);
	for(int l=0; l<sizeof(m_Members)/sizeof(m_Members[0]); l++) {
		if(!m_Members[l]) continue;
		m_Members[l]->GetViewDataInTeam(&viewdata);
		buf.PutStruct(&viewdata);
	}
	pPlayer->GetConnection()->SendData(buf.GetBuffer(), buf.GetLength());

	m_Members[nIndex] = pPlayer;
	if(!GetMember(m_nLeader)) m_nLeader = pPlayer->GetActorId();

	CCmdDataWriter<10> cmd(SGCMDCODE_TEAM_JOIN, 0);
	cmd.PutValue(this);
	pPlayer->OnNotify(cmd.GetCmdData());

	return true;
}

bool CSGTeam::Leave(CSGPlayer* pPlayer)
{
	int nIndex;
	CDataBuffer<20> buf;

	for(nIndex=0; nIndex<sizeof(m_Members)/sizeof(m_Members[0]); nIndex++) {
		if(m_Members[nIndex]==pPlayer) break;
	}
	if(nIndex>=sizeof(m_Members)/sizeof(m_Members[0])) return false;
	CCmdDataWriter<10> cmd(SGCMDCODE_TEAM_LEAVE, 0);
	cmd.PutValue(this);
	pPlayer->OnNotify(cmd.GetCmdData());

	// send leave message to pPlayer
	buf.PutValue<unsigned short>(SGCMDCODE_TEAM_LEAVE);
	buf.PutValue(m_Members[nIndex]->GetActorId());
	m_Members[nIndex]->GetConnection()->SendData(buf.GetBuffer(), buf.GetLength());
	m_Members[nIndex] = NULL;

	if(!GetMember(m_nLeader)) {
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
		m_nLeader = nLeader;

		// send leader change
		CDataBuffer<20> bufa;
		bufa.PutValue<unsigned short>(SGCMDCODE_TEAM_LEAVE);
		bufa.PutValue(m_nLeader);
		SendData(bufa.GetBuffer(), bufa.GetLength());
	}

	// send leave message for everyone
	SendData(buf.GetBuffer(), buf.GetLength());

	return true;
}

void CSGTeam::OnData(CSGPlayer* pPlayer, const CmdData* pCmdData)
{
	if(pPlayer->GetTeam() && pPlayer->GetTeam()!=this) return;

	CCmdDataReader cmd(pCmdData);
	CDataBuffer<300> buf;

	switch(pCmdData->nCmd) {
	case SGCMDCODE_TEAM_JOIN:
		{
			assert(!pPlayer->GetTeam());
			buf.PutValue<unsigned short>(SGCMDCODE_TEAM_JOIN);
			buf.PutValue(pPlayer->GetActorId());
			buf.PutString(pPlayer->GetName());
			buf.PutString("token");
			GetMember(cmd.GetValue<unsigned int>())->GetConnection()->SendData(buf.GetBuffer(), buf.GetLength());
			return;
		}
	case SGCMDCODE_TEAM_INVITE:
		{
			CSGPlayer* pWho = pPlayer->GetArea()->GetPlayer(cmd.GetValue<unsigned int>());
			if(pWho) {
				buf.PutValue<unsigned short>(SGCMDCODE_TEAM_INVITE);
				buf.PutValue(pPlayer->GetActorId());
				buf.PutString(pPlayer->GetName());
				buf.PutString("token");
				pWho->GetConnection()->SendData(buf.GetBuffer(), buf.GetLength());
			}
			return;
		}
	case SGCMDCODE_TEAM_ACCEPT:
		{
			CSGPlayer* pWho = pPlayer->GetArea()->GetPlayer(cmd.GetValue<unsigned int>());
			if(pWho) {
				if(pPlayer->GetTeam()) {
					if(!pWho->GetTeam()) {
						if(strcmp(cmd.GetString(), "token")==0) {
							Join(pWho);
						}
					}
				} else {
					if(pWho->GetTeam()) {
						if(strcmp(cmd.GetString(), "token")==0) {
							Join(pPlayer);
						}
					}
				}
			}
			return;
		}
	case SGCMDCODE_TEAM_SAY:
		{
			buf.PutValue<unsigned short>(SGCMDCODE_TEAM_SAY);
			buf.PutValue(pPlayer->GetActorId());
			buf.PutString(cmd.GetString());
			SendData(buf.GetBuffer(), buf.GetLength());
		}
	case SGCMDCODE_TEAM_LEADER:
		{
			unsigned int nLeader = cmd.GetValue<unsigned int>();
			if(pPlayer->GetActorId()==m_nLeader && GetMember(nLeader)) {
				m_nLeader = nLeader;
				CDataBuffer<20> bufa;
				bufa.PutValue<unsigned short>(SGCMDCODE_TEAM_LEAVE);
				bufa.PutValue(m_nLeader);
				SendData(buf.GetBuffer(), buf.GetLength());
			}
		}
		break;
	case SGCMDCODE_TEAM_KICK:
		{
			CSGPlayer* pWho = GetMember(cmd.GetValue<unsigned int>());
			if(pWho) Leave(pWho);
			return;
		}
	case SGCMDCODE_TEAM_LEAVE:
		{
			Leave(pPlayer);
			return;
		}
	}
}

void CSGTeam::SendData(const void* pData, unsigned int nSize)
{
	for(int l=0; l<sizeof(m_Members)/sizeof(m_Members[0]); l++) {
		if(!m_Members[l]) continue;
		m_Members[l]->GetConnection()->SendData(pData, nSize);
	}
}

CSGPlayer::CSGPlayer(CSGConnection* pConnection, unsigned int nPlayerId) : CSGRole(SGACTORTYPE_PLAYER)
{
	m_pConnection = pConnection;
	m_nPlayerId = nPlayerId;
	sprintf(m_szName, "USER-%d", m_nPlayerId);
	m_pBattle = NULL;
	m_pTeam = NULL;
	m_pPet = NULL;
	memset(m_Equip, 0, sizeof(m_Equip));
	memset(m_Bag, 0, sizeof(m_Bag));
	memset(m_Warehouse, 0, sizeof(m_Warehouse));

	m_pConnection->GetCallback()->OnPlayerAttach(GetPlayerId(), m_szName, this);
}

CSGPlayer::~CSGPlayer()
{
	m_pConnection->GetCallback()->OnPlayerDetach(GetPlayerId(), m_szName, this);

	assert(!m_pBattle);
	assert(!m_pTeam);
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
	if(GetTeam()) GetTeam()->Leave(this);
	if(GetBattleField()) GetBattleField()->Leave(this);
	return true;
}

bool CSGPlayer::GetViewData(CSGPlayer* pPlayer, SGPLAYER_VIEWDATA* pData)
{
	memset(pData, 0, sizeof(*pData));
	strcpy(pData->name, this->GetName());
	sprintf(pData->guild, "%p", this);
	return true;
}

bool CSGPlayer::GetViewDataInTeam(SGPLAYER_VIEWDATA_INTEAM* pData)
{
	memset(pData, 0, sizeof(*pData));
	pData->actorid = GetActorId();
	strcpy(pData->name, GetName());
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
	CCmdDataReader cmd(pCmdData);
	switch(cmd.CmdCode()) {
	case SGCMDCODE_BATTLEFIELD_JOIN:
		assert(!m_pBattle);
		m_pBattle = cmd.GetValue<CSGBattleField*>();
		assert(m_pBattle);
		assert(m_pBattle->GetActorId()==cmd.CmdWho());
		SetPositionNULL();
		return;
	case SGCMDCODE_BATTLEFIELD_LEAVE:
		assert(m_pBattle);
		assert(m_pBattle->GetActorId()==cmd.CmdWho());
		assert(m_pBattle==cmd.GetValue<CSGBattleField*>());
		m_pBattle = NULL;
		Move(NULL, NULL, 0);
		return;
	case SGCMDCODE_TEAM_JOIN:
		assert(!m_pTeam);
		m_pTeam = cmd.GetValue<CSGTeam*>();
		assert(m_pTeam);
		return;
	case SGCMDCODE_TEAM_LEAVE:
		assert(m_pTeam);
		assert(m_pTeam==cmd.GetValue<CSGTeam*>());
		m_pTeam = NULL;
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
		if(pActor->GetActorType()!=SGACTORTYPE_BATTLEFIELD) {
			buf.PutValue(pActor->GetDestination().x);
			buf.PutValue(pActor->GetDestination().y);
			buf.PutValue(pActor->GetDestination().z);
			buf.PutValue(pActor->GetWalkTime());
			buf.PutValue(pActor->GetDirection());
		}

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

	switch(pCmdData->nCmd) {
	case SGCMDCODE_TEAM_JOIN:
		{
			if(GetTeam()) return;
			unsigned int nActorId = cmd.GetValue<unsigned int>();
			if(GetActorId()==nActorId) {
				CSGTeam* pTeam = new CSGTeam;
				pTeam->Join(this);
				return;
			}
			CSGPlayer* pPlayer = GetArea()->GetPlayer(nActorId);
			if(!pPlayer || !pPlayer->GetTeam()) return;
			pPlayer->GetTeam()->OnData(this, pCmdData);
			return;
		}
	case SGCMDCODE_TEAM_ACCEPT:
		if(!GetTeam()) {
			CSGPlayer* pPlayer = GetArea()->GetPlayer(cmd.GetValue<unsigned int>());
			if(!pPlayer || !pPlayer->GetTeam()) return;
			pPlayer->GetTeam()->OnData(this, pCmdData);
			return;
		} else {
			GetTeam()->OnData(this, pCmdData);
			return;
		}
	case SGCMDCODE_TEAM_INVITE:
	case SGCMDCODE_TEAM_SAY:
	case SGCMDCODE_TEAM_LEADER:
	case SGCMDCODE_TEAM_KICK:
	case SGCMDCODE_TEAM_LEAVE:
		GetTeam()->OnData(this, pCmdData);
		return;
	}

}

void CSGPlayer::OnPassive(const CmdData* pCmdData)
{
}
