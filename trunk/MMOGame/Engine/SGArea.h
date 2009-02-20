#pragma once

class CSGAreaActor;
class CSGArea;

#define SGWAREHOUSE_COUNT		10
#define SGEQUIPTMENT_COUNT		10

class CSGItemLogic : public IItemLogic<CSGArea>
{
public:
};

ItemSData* SGGetItemSData(os_dword nItemId);
CSGItemLogic* SGGetItemLogic(os_dword nClassId);

class CSGAreaActor : public CAreaActor<CSGArea, CSGAreaActor>
{
public:
	CSGAreaActor() {
	}
	virtual ~CSGAreaActor() {
	}

	bool DropItem(int nIndex) {
		assert(nIndex>=0 && nIndex<SGWAREHOUSE_COUNT);
		if(nIndex<0 || nIndex>=SGWAREHOUSE_COUNT) return false;
		memset(&m_Warehouse[nIndex], 0, sizeof(m_Warehouse[0]));
		return true;
	}

	bool Equip(int nIndex, int nSolt) {
		assert(nIndex>=0 && nIndex<SGWAREHOUSE_COUNT);
		if(nIndex<0 || nIndex>=SGWAREHOUSE_COUNT) return false;
		assert(nSolt>=0 && nSolt<SGEQUIPTMENT_COUNT);
		if(nSolt<0 || nSolt>=SGEQUIPTMENT_COUNT);

		if(m_Warehouse[nIndex].nItemId==0) {
			return false;
		}

		if(m_EquipSolts[nSolt].nItemId!=0) {
			return false;
		}

		ItemSData* pSData = SGGetItemSData(m_Warehouse[nIndex].nItemId);
		if(!pSData) {
			return false;
		}

		CSGitemLogic* pLogic = SGGetItemLogic(pSData->nClassId);
		if(!pLogic) {
			return false;
		}

		return pLogic->Equip(this, nSolt, &m_Warehouse[nIndex], nIndex);
	}

	ItemUData	m_EquipSolts[SGEQUIPTMENT_COUNT];
	ItemUData	m_Warehouse[SGWAREHOUSE_COUNT];
};

class CSGArea : public CArea<CSGArea, CSGAreaActor>
{
public:
	CSGArea() {
	}
	virtual ~CSGArea() {
	}

};

class CSGItemLogic_Weapon : public CSGItemLogic
{
public:
	CSGItemLogic_Weapon();
	virtual ~CSGItemLogic_Weapon();

	virtual void Equip(CSGArea* pActor, int nSolt, ItemUData* pData, int nItemIndex);
	virtual void Use(CSGArea* pActor, ItemUData* pData, int nItemIndex);
	virtual void Drop(CSGArea* pActor, ItemUData* pData, int nItemIndex);
};

class CSGItemLogic_Armor : public CSGItemLogic
{
public:
	CSGItemLogic_Armor();
	virtual ~CSGItemLogic_Armor();

	virtual void Equip(CSGArea* pActor, int nSolt, ItemUData* pData, int nItemIndex);
	virtual void Use(CSGArea* pActor, ItemUData* pData, int nItemIndex);
	virtual void Drop(CSGArea* pActor, ItemUData* pData, int nItemIndex);
};

class CSGItemLogic_Pet : public CSGItemLogic
{
public:
	CSGItemLogic_Pet();
	virtual ~CSGItemLogic_Pet();

	virtual void Equip(CSGArea* pActor, int nSolt, ItemUData* pData, int nItemIndex);
	virtual void Use(CSGArea* pActor, ItemUData* pData, int nItemIndex);
	virtual void Drop(CSGArea* pActor, ItemUData* pData, int nItemIndex);
};
