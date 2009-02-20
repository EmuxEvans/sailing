#pragma once

typedef struct FAreaVector {
	float		x;
	float		y;
	float		z;
} FAreaVector;

typedef struct FMsgBlock {
	int nCmd;
	char* pData;
} FMsgBlock;

float AreaVectorDistance(const FAreaVector& vecA, const FAreaVector& vecB);

template<class TArea, class TAreaActor>
class CArea;
template<class TArea, class TAreaActor>
class CAreaCell;
template<class TArea, class TAreaActor>
class CAreaActor;

template<class TArea, class TAreaActor>
class CArea
{
public:
	CArea(float fCellWidth, float fCellHeight, CAreaCell<TArea, TAreaActor>* pCells, int nColMax, int nRowMax);
	virtual ~CArea();

	float CellWidth();
	float CellHeight();
	int AreaColMax();
	int AreaRowMax();
	CAreaCell<TArea, TAreaActor>* GetCell(int nCol, int nRow);
	CAreaCell<TArea, TAreaActor>* GetCell(const FAreaVector& vecPos);

	TAreaActor* GetActor(const FAreaVector& vecPos, float fRange, unsigned int nActorId);
	TAreaActor* GetActor(unsigned int nActorId);

	void Notify(const FAreaVector& vecPos, float fRange, const FMsgBlock* pData);
	void Notify(const FMsgBlock* pData);
	
private:
	float				m_fCellWidth, m_fCellHeight;
	TAreaActor*			m_pCells;
	int					m_nColMax, m_nRowMax;
};

template<class TArea, class TAreaActor>
class CAreaCell
{
public:
	CAreaCell();
	virtual ~CAreaCell();

	void SetArea(CArea<TArea, TAreaActor>* pAreaList, int nCol, int nRow);
	TArea* GetArea();
	int GetAreaCol();
	int GetAreaRow();

	TAreaActor* GetActor(const FAreaVector& vecPos, float fRange, unsigned int nActorId);
	TAreaActor* GetActor(unsigned int nActorId);

	void Notify(const FAreaVector& vecPos, float fRange, const FMsgBlock* pData);
	void Notify(const FMsgBlock* pData);

protected:
	void InsertActor(TAreaActor* pActor);
	void RemoveActor(TAreaActor* pActor);
private:
	TArea* m_pArea;
	int m_nAreaCol, m_nAreaRow;
	std::list<TAreaActor*> m_Actors;
};

template<class TArea, class TAreaActor>
class CAreaActor
{
public:
	CAreaActor(unsigned int nActorId);
	virtual ~CAreaActor();

	unsigned int GetActorId();
	TArea* GetArea();
	CAreaCell<TArea, TAreaActor>* GetAreaCell();
	void SetArea(TArea* pArea);
	void SetAreaCell(TArea* pArea, CAreaCell<TArea, TAreaActor>* pCell);

	void SetPosition(const FAreaVector& vecPosition, const FAreaVector& vecDirection);
	const FAreaVector& GetPosition() const;
	const FAreaVector& GetDirection() const;

	void SetTarget(TAreaActor* pTarget);
	TAreaActor* GetTarget();

	virtual void OnNotify(TAreaActor* pWho, const FMsgBlock* pData) = 0;
	virtual void OnAction(const FMsgBlock* pData) = 0;
	virtual void OnPassive(TAreaActor* pWho, const FMsgBlock* pData) = 0;

private:
	unsigned int m_nActorId;
	TArea*							m_pArea;
	CAreaCell<TArea, TAreaActor>*	m_pAreaCell;
	FAreaVector						m_vecPosition, m_vecDirection;
};

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

	virtual void Equip(TAreaActor* pActor, int nSolt, ItemUData* pUData, int nItemIndex) = 0;
	virtual void Use(TAreaActor* pActor, ItemUData* pUData, int nItemIndex) = 0;
	virtual void Drop(TAreaActor* pActor, ItemUData* pUData, int nItemIndex) = 0;
};
