#pragma once

class CSGArea;
class CSGRole;

class CSGBattleField : public CSGAreaActor
{
protected:
	CSGBattleField();
	virtual ~CSGBattleField();

public:
	virtual bool Join(CSGRole* pRole, int nIndex) = 0;
	virtual bool Joinable(CSGRole* pRole, int& nIndex) = 0;

	virtual CSGRole* GetRole(unsigned int nActorId) = 0;

	virtual bool GetViewData(CSGPlayer* pPlayer, SGBATTLEFIELD_VIEWDATA* pData) = 0;

	virtual void Notify(const CmdData* pCmdData) = 0;
	virtual void Notify(int nTeam, const CmdData* pCmdData) = 0;
	virtual void Passive(const CmdData* pCmdData) = 0;
	virtual void Passive(int nTeam, const CmdData* pCmdData) = 0;

};

CSGBattleField* SGBattleField_Create(CSGArea* pArea, const Vector& vecPosition);
