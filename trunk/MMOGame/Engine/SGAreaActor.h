#pragma once

class CSGArea;
class CSGAreaActor;
class CSGPlayer;

class CSGAreaActor : public CAreaActor<CSGArea, CSGAreaActor>
{
public:
	CSGAreaActor(int nActorType);
	virtual ~CSGAreaActor();

	int GetActorType() const { return m_nActorType; }

private:
	int m_nActorType;
};

class CSGRole : public CSGAreaActor
{
public:
	CSGRole(int nActorType);
	virtual ~CSGRole();


};

class CSGPlayer : public CSGAreaActor
{
public:
	CSGPlayer(unsigned int nUserId);
	virtual ~CSGPlayer();

	bool DropItem(int nIndex);
	bool Equip(int nIndex, int nSolt);

	virtual void OnMove(const Vector& vecPosition, CAreaCell<CSGArea, CSGAreaActor>* pFrom, CAreaCell<CSGArea, CSGAreaActor>* pTo);

	virtual void OnNotify(const CmdData* pCmdData);
	virtual void OnAction(const CmdData* pCmdData);
	virtual void OnPassive(CSGAreaActor* pWho, const CmdData* pCmdData);

	void Process(const CmdData* pCmdData);

protected:
	unsigned int m_nUserId;
	ItemUData	m_Equip[SGEQUIPMENT_COUNT];
	ItemUData	m_Bag[SGBAG_MAXCOUNT];
	ItemUData	m_Warehouse[SGWAREHOUSE_COUNT];
};
