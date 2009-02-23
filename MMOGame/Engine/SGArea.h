#pragma once

class CSGArea;
class CSGAreaActor;

class CSGArea : public CArea<CSGArea, CSGAreaActor>
{
public:
	CSGArea() {
	}
	virtual ~CSGArea() {
	}

};

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

