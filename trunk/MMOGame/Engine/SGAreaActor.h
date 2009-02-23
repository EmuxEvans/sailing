#pragma once

class CSGArea;
class CSGAreaActor;
class CSGPlayer;

class CSGAreaActor : public CAreaActor<CSGArea, CSGAreaActor>
{
public:
	CSGAreaActor(int nActorType, unsigned int nActorId);
	virtual ~CSGAreaActor();

	int GetActorType() const { return m_nActorType; }

private:
	int m_nActorType;
};

class CSGPlayer : public CSGAreaActor
{
public:
	CSGPlayer(unsigned int nActorId);
	virtual ~CSGPlayer();

	bool DropItem(int nIndex);
	bool Equip(int nIndex, int nSolt);

	virtual void OnMove(const Vector& vecDestination);

protected:
	ItemUData	m_Equip[SGEQUIPMENT_COUNT];
	ItemUData	m_Bag[SGBAG_MAXCOUNT];
	ItemUData	m_Warehouse[SGWAREHOUSE_COUNT];
};
