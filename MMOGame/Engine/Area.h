#pragma once

#define AREA_ACTION_NOTIFY_RANGE		0

#define CMDCODE_MOVE			1001
#define CMDCODE_MOVE_JOIN		1002
#define CMDCODE_MOVE_LEAVE		1003

template<class TArea, class TAreaActor>
class CArea;
template<class TArea, class TAreaActor>
class CAreaCell;
template<class TArea, class TAreaActor>
class CAreaActor;

template<class TArea, class TAreaActor>
class CArea
{
	friend class CAreaActor<TArea, TAreaActor>;
public:
	CArea(float fCellWidth, float fCellHeight, CAreaCell<TArea, TAreaActor>* pCells, int nColMax, int nRowMax);
	virtual ~CArea();

	float CellWidth();
	float CellHeight();
	int AreaColCount();
	int AreaRowCount();
	CAreaCell<TArea, TAreaActor>* GetCell(int nCol, int nRow);
	CAreaCell<TArea, TAreaActor>* GetCell(const Vector& vecPos);

	TAreaActor* GetActor(unsigned int nActorId);

	void Notify(const CmdData* pCmdData, const Vector& vecPos, float fRange);
	void Notify(const CmdData* pCmdData, int nCellX, int nCellY, int nRange=AREA_ACTION_NOTIFY_RANGE);
	void Notify(const CmdData* pCmdData, CAreaCell<TArea, TAreaActor>* pCell, int nRange=AREA_ACTION_NOTIFY_RANGE);
	void Notify(const CmdData* pCmdData);

	void Passive(const CmdData* pCmdData, const Vector& vecPos, float fRange);
	void Passive(const CmdData* pCmdData, int nCellX, int nCellY, int nRange=AREA_ACTION_NOTIFY_RANGE);
	void Passive(const CmdData* pCmdData, CAreaCell<TArea, TAreaActor>* pCell, int nRange=AREA_ACTION_NOTIFY_RANGE);
	void Passive(const CmdData* pCmdData);

	virtual void Tick(unsigned int nCurTime, unsigned int nDelta);

protected:
	bool InsertActor(TAreaActor* pActor);
	bool RemoveActor(TAreaActor* pActor);
private:
	float m_fCellWidth, m_fCellHeight;
	int m_nColCount, m_nRowCount;
	CAreaCell<TArea, TAreaActor>* m_pCells;
	std::map<unsigned int, TAreaActor*> m_mapActors;
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

	void Notify(const Vector& vecPos, float fRange, const CmdData* pCmdData);
	void Notify(const CmdData* pCmdData);

	void Passive(const Vector& vecPos, float fRange, const CmdData* pCmdData);
	void Passive(const CmdData* pCmdData);

	TAreaActor* GetActorByIndex(int nIndex);
	int GetActorIndexMax();

protected:
	bool InsertActor(TAreaActor* pActor);
	bool RemoveActor(TAreaActor* pActor);
private:
	TArea* m_pArea;
	int m_nAreaCol, m_nAreaRow;
	TAreaActor* m_Actors[100];
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
	void SetPositionNULL();
	void Move(const Vector* pStart, const Vector* pEnd, unsigned int nTime);
	void ChangeCell(CAreaCell<TArea, TAreaActor>* pFromCell, CAreaCell<TArea, TAreaActor>* pToCell);

	const Vector& GetPosition() const;
	const Vector& GetDestination() const;
	const unsigned int GetWalkTime() const;
	float GetDirection() const;

	void SetTarget(TAreaActor* pTarget);
	TAreaActor* GetTarget();

	void SendNotify(const CmdData* pCmdData, int nRange = AREA_ACTION_NOTIFY_RANGE);
	void SendNotify(const CmdData* pCmdData, float fRange);

	void SendPassive(const CmdData* pCmdData, int nRange = AREA_ACTION_NOTIFY_RANGE);
	void SendPassive(const CmdData* pCmdData, float fRange);

	virtual void OnNotify(const CmdData* pCmdData) {}
	virtual void OnAction(const CmdData* pCmdData) {}
	virtual void OnPassive(const CmdData* pCmdData) {}
	virtual void Tick(unsigned int nCurTime, unsigned int nDelta);

private:
	unsigned int m_nActorId;
	TArea*							m_pArea;
	CAreaCell<TArea, TAreaActor>*	m_pAreaCell;
	Vector							m_vecPosition;
	float							m_fDirection;
	Vector							m_vecDestination;
	float							m_fVelocity;
	unsigned int					m_nWalkTime;

};
