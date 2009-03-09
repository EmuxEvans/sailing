#pragma once

typedef struct ItemSData {
	unsigned int	nItemId;
	unsigned char	nType;
	int				nClassId;
	char			bUniqueId;
	char			bCanEquip;
	char			bUsable;
	unsigned char	aData[100];
} ItemSData;

typedef struct ItemUData {
	unsigned int	nItemId;
	unsigned int	nUniqueId;
	unsigned char		aData[100];
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
