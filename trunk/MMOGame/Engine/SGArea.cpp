#include <list>

#include <skates/errcode.h>
#include <skates/os.h>

#include "GameArea.h"
#include "GameArea.inl"

#include "SGArea.h"

CSGAreaActor::CSGAreaActor()
{
}

CSGAreaActor::~CSGAreaActor()
{
}

bool CSGAreaActor::DropItem(int nIndex)
{
	assert(nIndex>=0 && nIndex<SGWAREHOUSE_COUNT);
	if(nIndex<0 || nIndex>=SGWAREHOUSE_COUNT) return false;
	memset(&m_Warehouse[nIndex], 0, sizeof(m_Warehouse[0]));
	return true;
}

bool CSGAreaActor::Equip(int nIndex, int nSolt)
{
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

CSGItemLogic_Weapon::CSGItemLogic_Weapon()
{
}

CSGItemLogic_Weapon::~CSGItemLogic_Weapon()
{
}

void CSGItemLogic_Weapon::Equip(CSGArea* pActor, int nSolt, ItemUData* pData, int nItemIndex)
{
}

void CSGItemLogic_Weapon::Use(CSGArea* pActor, ItemUData* pData, int nItemIndex)
{
}

void CSGItemLogic_Weapon::Drop(CSGArea* pActor, ItemUData* pData, int nItemIndex)
{
}

CSGItemLogic_Armor::CSGItemLogic_Armor()
{
}

CSGItemLogic_Armor::~CSGItemLogic_Armor()
{
}

void CSGItemLogic_Armor::Equip(CSGArea* pActor, int nSolt, ItemUData* pData, int nItemIndex)
{
}

void CSGItemLogic_Armor::Use(CSGArea* pActor, ItemUData* pData, int nItemIndex)
{
}

void CSGItemLogic_Armor::Drop(CSGArea* pActor, ItemUData* pData, int nItemIndex)
{
}

CSGItemLogic_Pet::CSGItemLogic_Pet()
{
}

CSGItemLogic_Pet::~CSGItemLogic_Pet()
{
}

void CSGItemLogic_Pet::Equip(CSGArea* pActor, int nSolt, ItemUData* pData, int nItemIndex)
{
}

void CSGItemLogic_Pet::Use(CSGArea* pActor, ItemUData* pData, int nItemIndex)
{
}

void CSGItemLogic_Pet::Drop(CSGArea* pActor, ItemUData* pData, int nItemIndex)
{
}
