#pragma once

typedef IItemLogic<CSGAreaActor> CSGItemLogic;

ItemSData* SGGetItemSData(os_dword nItemId);
CSGItemLogic* SGGetItemLogic(os_dword nClassId);

// Weapon
typedef struct SGItemSData_Weapon {
} SGItemSData_Weapon;

typedef struct SGItemUData_Weapon {
} SGItemUData_Weapon;

SGItemSData_Weapon* sgitem_GetWeaponSData(ItemSData* pData);
SGItemUData_Weapon* sgitem_GetWeaponUData(ItemUData* pData);

class CSGItemLogic_Weapon : public CSGItemLogic
{
public:
	CSGItemLogic_Weapon();
	virtual ~CSGItemLogic_Weapon();

	virtual void Equip(CSGArea* pActor, int nSolt, ItemUData* pData, int nItemIndex);
	virtual void Use(CSGArea* pActor, ItemUData* pData, int nItemIndex);
	virtual void Drop(CSGArea* pActor, ItemUData* pData, int nItemIndex);
};

// Armor
typedef struct SGItemSData_Armor {
} SGItemSData_Armor;

typedef struct SGItemUData_Armor {
} SGItemUData_Armor;

SGItemSData_Armor* sgitem_GetArmorSData(ItemSData* pData);
SGItemUData_Armor* sgitem_GetArmorUData(ItemUData* pData);

class CSGItemLogic_Armor : public CSGItemLogic
{
public:
	CSGItemLogic_Armor();
	virtual ~CSGItemLogic_Armor();

	virtual void Equip(CSGArea* pActor, int nSolt, ItemUData* pData, int nItemIndex);
	virtual void Use(CSGArea* pActor, ItemUData* pData, int nItemIndex);
	virtual void Drop(CSGArea* pActor, ItemUData* pData, int nItemIndex);
};

// Pet
typedef struct SGItemSData_Pet {
} SGItemSData_Pet;

typedef struct SGItemUData_Pet {
} SGItemUData_Pet;

SGItemSData_Pet* sgitem_GetPetSData(ItemSData* pData);
SGItemUData_Pet* sgitem_GetPetUData(ItemUData* pData);

class CSGItemLogic_Pet : public CSGItemLogic
{
public:
	CSGItemLogic_Pet();
	virtual ~CSGItemLogic_Pet();

	virtual void Equip(CSGArea* pActor, int nSolt, ItemUData* pData, int nItemIndex);
	virtual void Use(CSGArea* pActor, ItemUData* pData, int nItemIndex);
	virtual void Drop(CSGArea* pActor, ItemUData* pData, int nItemIndex);
};
