#pragma once

class CSGArea;
class CSGAreaActor;
class CSGRole;
class CSGNPC;
class CSGPlayer;
class CSGBattleField;

class CSGAreaActor : public CAreaActor<CSGArea, CSGAreaActor>
{
public:
	CSGAreaActor(int nActorType);
	virtual ~CSGAreaActor();

	int GetActorType() const {
		return m_nActorType;
	}

private:
	int m_nActorType;
};

class CSGRole : public CSGAreaActor
{
public:
	CSGRole(int nActorType);
	virtual ~CSGRole();

};

class CSGNPC : public CSGRole
{
public:
	CSGNPC() : CSGRole(SGACTORTYPE_NPC) {
	}
	virtual ~CSGNPC() {
	}

	virtual bool GetViewData(CSGPlayer* pPlayer, SGNPC_VIEWDATA* pData) {
		return false;
	}
};
