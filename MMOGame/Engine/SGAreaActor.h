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

class CSGPlayer : public CSGAreaActor
{
public:
	CSGPlayer(unsigned int nUserId);
	virtual ~CSGPlayer();

	bool DropItem(int nIndex);
	bool Equip(int nIndex, int nSolt);

	virtual void OnMove(const Vector& vecDestination);
	virtual void Process(const CmdData* pCmdData);

protected:
	unsigned int m_nUserId;
	ItemUData	m_Equip[SGEQUIPMENT_COUNT];
	ItemUData	m_Bag[SGBAG_MAXCOUNT];
	ItemUData	m_Warehouse[SGWAREHOUSE_COUNT];
};
