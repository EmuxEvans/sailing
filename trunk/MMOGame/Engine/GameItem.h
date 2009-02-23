#pragma once

typedef struct ItemSData {
	os_dword	nItemId;
	os_byte		nType;
	os_int		nClassId;
	os_char		bUniqueId;
	os_char		bCanEquip;
	os_char		bUsable;
	os_byte		aData[100];
} ItemSData;

typedef struct ItemUData {
	os_dword	nItemId;
	os_qword	nUniqueId;
	os_byte		aData[100];
} ItemUData;

template<class TAreaActor>
class IItemLogic
{
public:
	virtual ~IItemLogic() { }

	virtual void Tick(unsigned int nCurTime, unsigned int nDelta) = 0;
	virtual bool Equip(TAreaActor* pActor, int nSolt, ItemUData* pUData, int nItemIndex) = 0;
	virtual bool Use(TAreaActor* pActor, ItemUData* pUData, int nItemIndex) = 0;
	virtual bool Drop(TAreaActor* pActor, ItemUData* pUData, int nItemIndex) = 0;
};
