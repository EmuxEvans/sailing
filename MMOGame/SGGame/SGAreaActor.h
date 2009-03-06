#pragma once

class CSGArea;
class CSGAreaActor;
class CSGRole;
class CSGPlayer;

class CSGAreaActor : public CAreaActor<CSGArea, CSGAreaActor>
{
public:
	CSGAreaActor(int nActorType);
	virtual ~CSGAreaActor();

	int GetActorType() const { return m_nActorType; }

	virtual bool GetViewData(CSGPlayer* pPlayer, SGROLE_VIEWDATA* pData) {
		return false;
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
};
