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

	virtual bool Join(CSGRole* pPlayer, int nIndex);
	virtual bool Leave(CSGRole* pPlayer);

	virtual bool Joinable(CSGRole* pPlayer, int& nIndex);

	virtual CSGRole* GetRole(unsigned int nActorId);

	virtual bool GetViewData(CSGPlayer* pPlayer, SGBATTLEFIELD_VIEWDATA* pData);

	virtual void Notify(const CmdData* pCmdData);
	virtual void Notify(int nTeam, const CmdData* pCmdData);
	virtual void Passive(const CmdData* pCmdData);
	virtual void Passive(int nTeam, const CmdData* pCmdData);

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

bool CSGBattleFieldImpl::Join(CSGRole* pPlayer, int nIndex)
{
	CmdData cmd = { SGCMDCODE_BATTLEFIELD_JOIN, GetActorId(), NULL, 0 };
	pPlayer->OnNotify(&cmd);
	return false;
}

bool CSGBattleFieldImpl::Leave(CSGRole* pPlayer)
{
	CmdData cmd = { SGCMDCODE_BATTLEFIELD_LEAVE, GetActorId(), NULL, 0 };
	pPlayer->OnNotify(&cmd);
	return false;
}

bool CSGBattleFieldImpl::Joinable(CSGRole* pPlayer, int& nIndex)
{
	return false;
}

CSGRole* CSGBattleFieldImpl::GetRole(unsigned int nActorId)
{
	for(int l=0; l<sizeof(m_Roles)/sizeof(m_Roles[0]); l++) {
		if(m_Roles[l]!=NULL && m_Roles[l]->GetActorId()==nActorId) return m_Roles[l];
	}
	return NULL;
}

bool CSGBattleFieldImpl::GetViewData(CSGPlayer* pPlayer, SGBATTLEFIELD_VIEWDATA* pData)
{
	return false;
}

void CSGBattleFieldImpl::Notify(const CmdData* pCmdData)
{
	for(int l=0; l<sizeof(m_Roles)/sizeof(m_Roles[0]); l++) {
		if(m_Roles[l]) {
			m_Roles[l]->OnNotify(pCmdData);
		}
	}
}

void CSGBattleFieldImpl::Notify(int nTeam, const CmdData* pCmdData)
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

void CSGBattleFieldImpl::Passive(const CmdData* pCmdData)
{
	for(int l=0; l<sizeof(m_Roles)/sizeof(m_Roles[0]); l++) {
		if(m_Roles[l]) {
			m_Roles[l]->OnPassive(pCmdData);
		}
	}
}

void CSGBattleFieldImpl::Passive(int nTeam, const CmdData* pCmdData)
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
