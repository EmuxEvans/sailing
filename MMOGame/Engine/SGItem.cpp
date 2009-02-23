#include <assert.h>
#include <list>

#include "Game.h"
#include "Math.h"
#include "VarCmd.h"

#include "GameArea.h"
#include "GameArea.inl"
#include "GameBuff.h"
#include "GameItem.h"
#include "GameSkill.h"

#include "SG.h"
#include "SGArea.h"
#include "SGAreaActor.h"
#include "SGBuff.h"
#include "SGItem.h"
#include "SGSkill.h"

ItemSData* SGGetItemSData(os_dword nItemId)
{
	return NULL;
}

CSGItemLogic* SGGetItemLogic(os_dword nClassId)
{
	return NULL;
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
