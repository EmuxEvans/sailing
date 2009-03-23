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
#include "SGBuff.h"
#include "SGItem.h"
#include "SGSkill.h"
#include "SGGameLoop.h"
#include "SGBattleField.h"

CSGBattleField::CSGBattleField() : CSGAreaActor(SGACTORTYPE_BATTLEFIELD)
{
}

CSGBattleField::~CSGBattleField()
{
}

class CSGBattleFieldImpl : public CSGBattleField
{
public:
	CSGBattleFieldImpl(CSGArea* pArea, const Vector& vecPosition);
	virtual ~CSGBattleFieldImpl();

	void Release();

	virtual bool Join(CSGRole* pPlayer, int nIndex);
	virtual bool Leave(CSGRole* pPlayer);

	virtual bool Joinable(CSGRole* pPlayer, int& nIndex);

	virtual CSGRole* GetRole(unsigned int nActorId);
	int GetRoleIndex(unsigned int nActorId);

	virtual bool GetViewData(CSGPlayer* pPlayer, SGBATTLEFIELD_VIEWDATA* pData);

	virtual void OnData(CSGRole* pRole, const CmdData* pCmdData);

	virtual void SendNotify(const CmdData* pCmdData);
	virtual void SendNotify(int nTeam, const CmdData* pCmdData);
	virtual void SendPassive(const CmdData* pCmdData);
	virtual void SendPassive(int nTeam, const CmdData* pCmdData);

private:
	CSGRole* m_Roles[10];
};

CSGBattleField* SGBattleField_Create(CSGArea* pArea, const Vector& vecPosition)
{
	return new CSGBattleFieldImpl(pArea, vecPosition);
}

CSGBattleFieldImpl::CSGBattleFieldImpl(CSGArea* pArea, const Vector& vecPosition)
{
	memset(m_Roles, 0, sizeof(m_Roles));
	SetArea(pArea);
	SetPosition(vecPosition, 0.0f);
}

CSGBattleFieldImpl::~CSGBattleFieldImpl()
{
}

void CSGBattleFieldImpl::Release()
{
	SetPositionNULL();
	SetArea(NULL);
}

bool CSGBattleFieldImpl::Join(CSGRole* pPlayer, int nIndex)
{
	if(nIndex<0 || nIndex>=sizeof(m_Roles)) {
		if(!Joinable(pPlayer, nIndex)) {
			return false;
		}
	}

	CCmdDataWriter<10> cmd(SGCMDCODE_BATTLEFIELD_JOIN, GetActorId());
	cmd.PutValue(this);
	pPlayer->OnNotify(cmd.GetCmdData());
	m_Roles[nIndex] = pPlayer;
	return true;
}

bool CSGBattleFieldImpl::Leave(CSGRole* pPlayer)
{
	int nIndex = GetRoleIndex(pPlayer->GetActorId());

	m_Roles[nIndex] = NULL;
	CCmdDataWriter<10> cmd(SGCMDCODE_BATTLEFIELD_LEAVE, GetActorId());
	cmd.PutValue(this);
	pPlayer->OnNotify(cmd.GetCmdData());
	return true;
}

bool CSGBattleFieldImpl::Joinable(CSGRole* pPlayer, int& nIndex)
{
	for(nIndex=0; nIndex<sizeof(m_Roles)/sizeof(m_Roles[0]); nIndex++) {
		if(!m_Roles[nIndex]) break;
	}
	if(nIndex==sizeof(m_Roles)/sizeof(m_Roles[0])) return false;

	return true;
}

CSGRole* CSGBattleFieldImpl::GetRole(unsigned int nActorId)
{
	for(int l=0; l<sizeof(m_Roles)/sizeof(m_Roles[0]); l++) {
		if(m_Roles[l]!=NULL && m_Roles[l]->GetActorId()==nActorId) return m_Roles[l];
	}
	return NULL;
}

int CSGBattleFieldImpl::GetRoleIndex(unsigned int nActorId)
{
	for(int l=0; l<sizeof(m_Roles)/sizeof(m_Roles[0]); l++) {
		if(m_Roles[l] && m_Roles[l]->GetActorId()==nActorId) return l;
	}
	return -1;
}

bool CSGBattleFieldImpl::GetViewData(CSGPlayer* pPlayer, SGBATTLEFIELD_VIEWDATA* pData)
{
	memset(pData, 0, sizeof(*pData));
	strcpy(pData->name, "cailie");
	return true;
}

void CSGBattleFieldImpl::OnData(CSGRole* pRole, const CmdData* pCmdData)
{
	switch(pCmdData->nCmd) {
	case SGCMDCODE_FIGHT_RUNAWAY:
		CCmdDataWriter<10> cmd(SGCMDCODE_BATTLEFIELD_LEAVE, GetActorId());
		cmd.PutValue(this);
		SendNotify(cmd.GetCmdData());
		Release();
		return;
	}
}

void CSGBattleFieldImpl::SendNotify(const CmdData* pCmdData)
{
	for(int l=0; l<sizeof(m_Roles)/sizeof(m_Roles[0]); l++) {
		if(m_Roles[l]) {
			m_Roles[l]->OnNotify(pCmdData);
		}
	}
}

void CSGBattleFieldImpl::SendNotify(int nTeam, const CmdData* pCmdData)
{
	if(nTeam==SGBATTLEFIELD_TEAM_RED) {
		for(int l=0; l<sizeof(m_Roles)/sizeof(m_Roles[0])/2; l++) {
			if(m_Roles[l]) {
				m_Roles[l]->OnNotify(pCmdData);
			}
		}
		return;
	}
	if(nTeam==SGBATTLEFIELD_TEAM_BLUE) {
		for(int l=sizeof(m_Roles)/sizeof(m_Roles[0])/2; l<sizeof(m_Roles)/sizeof(m_Roles[0]); l++) {
			if(m_Roles[l]) {
				m_Roles[l]->OnNotify(pCmdData);
			}
		}
		return;
	}
}

void CSGBattleFieldImpl::SendPassive(const CmdData* pCmdData)
{
	for(int l=0; l<sizeof(m_Roles)/sizeof(m_Roles[0]); l++) {
		if(m_Roles[l]) {
			m_Roles[l]->OnPassive(pCmdData);
		}
	}
}

void CSGBattleFieldImpl::SendPassive(int nTeam, const CmdData* pCmdData)
{
	if(nTeam==SGBATTLEFIELD_TEAM_RED) {
		for(int l=0; l<sizeof(m_Roles)/sizeof(m_Roles[0])/2; l++) {
			if(m_Roles[l]) {
				m_Roles[l]->OnPassive(pCmdData);
			}
		}
		return;
	}
	if(nTeam==SGBATTLEFIELD_TEAM_BLUE) {
		for(int l=sizeof(m_Roles)/sizeof(m_Roles[0])/2; l<sizeof(m_Roles)/sizeof(m_Roles[0]); l++) {
			if(m_Roles[l]) {
				m_Roles[l]->OnPassive(pCmdData);
			}
		}
		return;
	}
}
