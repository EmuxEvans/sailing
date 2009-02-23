#pragma once

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
	CAreaCell<TArea, TAreaActor>* GetCell(const Vector& vecPos);

	TAreaActor* GetActor(const Vector& vecPos, float fRange, unsigned int nActorId);
	TAreaActor* GetActor(unsigned int nActorId);

	void Notify(const Vector& vecPos, float fRange, const FMsgBlock* pData);
	void Notify(const FMsgBlock* pData);

	virtual void Tick(unsigned int nCurTime, unsigned int nDelta);

private:
	float m_fCellWidth, m_fCellHeight;
	int m_nColCount, m_nRowCount;
	CAreaCell<TArea, TAreaActor>* m_pCells;
};

template<class TArea, class TAreaActor>
class CAreaCell
{
	friend class CAreaActor<TArea, TAreaActor>;
public:
	CAreaCell();
	virtual ~CAreaCell();

	void SetArea(TArea* pArea, int nCol, int nRow);
	TArea* GetArea();
	int GetAreaCol();
	int GetAreaRow();

	TAreaActor* GetActor(const Vector& vecPos, float fRange, unsigned int nActorId);
	TAreaActor* GetActor(unsigned int nActorId);

	void Notify(const Vector& vecPos, float fRange, const FMsgBlock* pData);
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

	void SetPosition(const Vector& vecPosition, float fDirection);
	void Move(const Vector* pStart, const Vector* pEnd, unsigned int nTime);


	const Vector& GetPosition() const;
	float GetDirection() const;

	void SetTarget(TAreaActor* pTarget);
	TAreaActor* GetTarget();

	virtual void OnMove(const Vector& vecDestination) {}
	virtual void OnEnterCell(TAreaActor* pWho, CAreaCell<TArea, TAreaActor>* pCell) {}
	virtual void OnLeaveCell(TAreaActor* pWho, CAreaCell<TArea, TAreaActor>* pCell) {}

	virtual void OnNotify(TAreaActor* pWho, const FMsgBlock* pData) {}
	virtual void OnAction(const FMsgBlock* pData) {}
	virtual void OnPassive(TAreaActor* pWho, const FMsgBlock* pData) {}
	virtual void Tick(unsigned int nCurTime, unsigned int nDelta);

private:
	unsigned int m_nActorId;
	TArea*							m_pArea;
	CAreaCell<TArea, TAreaActor>*	m_pAreaCell;
	Vector						m_vecPosition;
	float							m_fDirection;
	Vector						m_vecDestination;
	float							m_fVelocity;
	unsigned int					m_nWalkTime;

};
