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

	virtual void Tick(unsigned int nCurTime, unsigned int nDelta);

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

	void Tick(unsigned int nCurTime, unsigned int nDelta);

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
	virtual void Tick(unsigned int nCurTime, unsigned int nDelta) = 0;

private:
	unsigned int m_nActorId;
	TArea*							m_pArea;
	CAreaCell<TArea, TAreaActor>*	m_pAreaCell;
	FAreaVector						m_vecPosition, m_vecDirection;
};
