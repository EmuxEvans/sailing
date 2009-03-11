#pragma once

template<class TArea, class TAreaActor>
CArea<TArea, TAreaActor>::CArea(float fCellWidth, float fCellHeight, CAreaCell<TArea, TAreaActor>* pCells, int nColCount, int nRowCount)
{
	m_fCellWidth = fCellWidth;
	m_fCellHeight = fCellHeight;
	m_pCells = pCells;
	m_nColCount = nColCount;
	m_nRowCount = nColCount;
	for(int i=nColCount*nRowCount-1; i>=0; i--) {
		pCells[i].SetArea((TArea*)this, i % nColCount, i / nColCount);
	}
}

template<class TArea, class TAreaActor>
CArea<TArea, TAreaActor>::~CArea()
{
}

template<class TArea, class TAreaActor>
float CArea<TArea, TAreaActor>::CellWidth()
{
	return m_fCellWidth;
}

template<class TArea, class TAreaActor>
float CArea<TArea, TAreaActor>::CellHeight()
{
	return m_fCellHeight;
}

template<class TArea, class TAreaActor>
int CArea<TArea, TAreaActor>::AreaColCount()
{
	return m_nColCount;
}

template<class TArea, class TAreaActor>
int CArea<TArea, TAreaActor>::AreaRowCount()
{
	return m_nRowCount;
}

template<class TArea, class TAreaActor>
CAreaCell<TArea, TAreaActor>* CArea<TArea, TAreaActor>::GetCell(int nCol, int nRow)
{
	if(nCol<0 || nCol>=m_nColCount) return NULL;
	if(nRow<0 || nRow>=m_nRowCount) return NULL;
	return &m_pCells[nRow*m_nColCount+nCol];
}

template<class TArea, class TAreaActor>
CAreaCell<TArea, TAreaActor>* CArea<TArea, TAreaActor>::GetCell(const Vector& vecPos)
{
	int nCol, nRow;
	nCol = (int)(vecPos.x/m_fCellWidth);
	nRow = (int)(vecPos.y/m_fCellHeight);
	if(nCol<0) nCol = 0;
	if(nCol>=m_nColCount) nCol = m_nColCount - 1;
	if(nRow<0) nRow = 0;
	if(nRow>=m_nRowCount) nRow = m_nRowCount - 1;
	return GetCell(nCol, nRow);
}

template<class TArea, class TAreaActor>
TAreaActor* CArea<TArea, TAreaActor>::GetActor(unsigned int nActorId)
{
	std::map<unsigned int, TAreaActor*>::iterator i;
	i = m_mapActors.find(nActorId);
	if(i!=m_mapActors.end()) return NULL;
	return i->second;
}

template<class TArea, class TAreaActor>
void CArea<TArea, TAreaActor>::Notify(const CmdData* pCmdData, const Vector& vecPos, float fRange)
{
}

template<class TArea, class TAreaActor>
void CArea<TArea, TAreaActor>::Notify(const CmdData* pCmdData, int nCellX, int nCellY, int nRange)
{
	int x, y;
	for(x=nCellX-nRange; x<=nCellX+nRange; x++) {
		if(x<0 || x>=m_nColCount) continue;
		for(y=nCellY-nRange; y<=nCellY+nRange; y++) {
			if(y<0 || y>=m_nRowCount) continue;
			CAreaCell<TArea, TAreaActor>* pCell;
			pCell = GetCell(x, y);
			pCell->Notify(pCmdData);
		}
	}
	
}

template<class TArea, class TAreaActor>
void CArea<TArea, TAreaActor>::Notify(const CmdData* pCmdData)
{
	for(int x=0; x<m_nColCount; x++) {
		for(int y=0; y<m_nRowCount; y++) {
			m_pCells[y*m_nColCount+x].Notify(pData);
		}
	}
}

template<class TArea, class TAreaActor>
bool CArea<TArea, TAreaActor>::InsertActor(TAreaActor* pActor)
{
	std::map<unsigned int, TAreaActor*>::iterator i;
	i = m_mapActors.find(pActor->GetActorId());
	assert(i==m_mapActors.end());
	if(i!=m_mapActors.end()) return false;
	m_mapActors[pActor->GetActorId()] = pActor;
	return true;
}

template<class TArea, class TAreaActor>
bool CArea<TArea, TAreaActor>::RemoveActor(TAreaActor* pActor)
{
	std::map<unsigned int, TAreaActor*>::iterator i;
	i = m_mapActors.find(pActor->GetActorId());
	assert(i!=m_mapActors.end());
	if(i==m_mapActors.end()) return false;
	m_mapActors.erase(i);
	return true;
}

template<class TArea, class TAreaActor>
void CArea<TArea, TAreaActor>::Tick(unsigned int nCurTime, unsigned int nDelta)
{
	std::map<unsigned int, TAreaActor*>::iterator i;
	for(i=m_mapActors.begin(); i!=m_mapActors.end(); i++) {
		(i->second)->Tick(nCurTime, nDelta);
	}
}

template<class TArea, class TAreaActor>
CAreaCell<TArea, TAreaActor>::CAreaCell()
{
}

template<class TArea, class TAreaActor>
CAreaCell<TArea, TAreaActor>::~CAreaCell()
{
}

template<class TArea, class TAreaActor>
void CAreaCell<TArea, TAreaActor>::SetArea(TArea* pArea, int nCol, int nRow)
{
	m_pArea = pArea;
	m_nAreaCol = nCol;
	m_nAreaRow = nRow;
}

template<class TArea, class TAreaActor>
TArea* CAreaCell<TArea, TAreaActor>::GetArea()
{
	return m_pArea;
}

template<class TArea, class TAreaActor>
int CAreaCell<TArea, TAreaActor>::GetAreaCol()
{
	return m_nAreaCol;
}

template<class TArea, class TAreaActor>
int CAreaCell<TArea, TAreaActor>::GetAreaRow()
{
	return m_nAreaRow;
}

template<class TArea, class TAreaActor>
void CAreaCell<TArea, TAreaActor>::Notify(const Vector& vecPos, float fRange, const CmdData* pCmdData)
{
	for(int i=0; i<sizeof(m_Actors)/sizeof(m_Actors[0]); i++) {
		if(m_Actors[i] && m_Actors[i]->GetActorId()!=pCmdData->nWho) {
			float fDistance = AreaVectorDistance(m_Actors[i]->GetPosition(), vecPos);
			if(fDistance<fRange) {
				m_Actors[i]->OnNotify(pCmdData);
			}
		}
	}
}

template<class TArea, class TAreaActor>
void CAreaCell<TArea, TAreaActor>::Notify(const CmdData* pCmdData)
{
	for(int i=0; i<sizeof(m_Actors)/sizeof(m_Actors[0]); i++) {
		if(m_Actors[i] && m_Actors[i]->GetActorId()!=pCmdData->nWho) {
			m_Actors[i]->OnNotify(pCmdData);
		}
	}
}

template<class TArea, class TAreaActor>
TAreaActor* CAreaCell<TArea, TAreaActor>::GetActorByIndex(int nIndex)
{
	if(nIndex<0 || nIndex>=sizeof(m_Actors)/sizeof(m_Actors[0])) return NULL;
	return m_Actors[nIndex];
}

template<class TArea, class TAreaActor>
int CAreaCell<TArea, TAreaActor>::GetActorIndexMax()
{
	return sizeof(m_Actors)/sizeof(m_Actors[0]);
}

template<class TArea, class TAreaActor>
bool CAreaCell<TArea, TAreaActor>::InsertActor(TAreaActor* pActor)
{
	unsigned int i;
	for(i=0; i<sizeof(m_Actors)/sizeof(m_Actors[0]); i++) {
		if(m_Actors[i]) continue;
		m_Actors[i] = pActor;
		return true;
	}
	assert(0);
	return false;
}

template<class TArea, class TAreaActor>
bool CAreaCell<TArea, TAreaActor>::RemoveActor(TAreaActor* pActor)
{
	unsigned int i;
	for(i=0; i<sizeof(m_Actors)/sizeof(m_Actors[0]); i++) {
		if(m_Actors[i]!=pActor) continue;
		m_Actors[i] = NULL;
		return true;
	}
	assert(0);
	return false;
}

template<class TArea, class TAreaActor>
CAreaActor<TArea, TAreaActor>::CAreaActor(unsigned int nActorId)
{
	m_nActorId = nActorId;
}

template<class TArea, class TAreaActor>
CAreaActor<TArea, TAreaActor>::~CAreaActor()
{
	assert(!m_pArea);
	assert(!m_pAreaCell);
}

template<class TArea, class TAreaActor>
unsigned int CAreaActor<TArea, TAreaActor>::GetActorId()
{
	return m_nActorId;
}

template<class TArea, class TAreaActor>
TArea* CAreaActor<TArea, TAreaActor>::GetArea()
{
	return m_pArea;
}

template<class TArea, class TAreaActor>
CAreaCell<TArea, TAreaActor>* CAreaActor<TArea, TAreaActor>::GetAreaCell()
{
	return m_pAreaCell;
}

template<class TArea, class TAreaActor>
void CAreaActor<TArea, TAreaActor>::SetArea(TArea* pArea)
{
	if(m_pArea) {
		m_pArea->RemoveActor((TAreaActor*)this);
	}
	m_pArea = pArea;
	m_pAreaCell = NULL;
	if(m_pArea) {
		m_pArea->InsertActor((TAreaActor*)this);
	}
}

template<class TArea, class TAreaActor>
void CAreaActor<TArea, TAreaActor>::SetPosition(const Vector& vecPosition, float fDirection)
{
	m_vecPosition = vecPosition;
	m_fDirection = fDirection;

	CAreaCell<TArea, TAreaActor>* pCell;
	CAreaCell<TArea, TAreaActor>* pOrignCell;

	pCell = m_pArea->GetCell(vecPosition);
	pOrignCell = m_pAreaCell;

	if(pOrignCell!=pCell) {
		m_pAreaCell = pCell;
		if(pOrignCell) {
			pOrignCell->RemoveActor((TAreaActor*)this);
		}
		if(pCell) {
			pCell->InsertActor((TAreaActor*)this);
		}

		ChangeCell(pOrignCell, pCell);
	}
}

template<class TArea, class TAreaActor>
void CAreaActor<TArea, TAreaActor>::SetPositionNULL()
{
	if(m_pAreaCell) {
		CAreaCell<TArea, TAreaActor>* pCell;
		pCell = m_pAreaCell;
		m_pAreaCell = NULL;
		pCell->RemoveActor((TAreaActor*)this);

		ChangeCell(pCell, NULL);
	}
}

template<class TArea, class TAreaActor>
void CAreaActor<TArea, TAreaActor>::Move(const Vector* pStart, const Vector* pEnd, unsigned int nTime)
{
	if(!pStart) pStart = &m_vecPosition;

	if(!pEnd) {
		SetPosition(*pStart, GetDirection());
		m_vecDestination = *pStart;
		m_nWalkTime = 0;
		return;
	}

	m_vecDestination = *pEnd;
	m_fVelocity = VectorDistance(*pStart, *pEnd) / nTime;
	m_nWalkTime = nTime;
}

template<class TArea, class TAreaActor>
void CAreaActor<TArea, TAreaActor>::ChangeCell(CAreaCell<TArea, TAreaActor>* pOrignCell, CAreaCell<TArea, TAreaActor>* pCell)
{
	int x, y, l;
	CmdData cmdJoin = { CMDCODE_MOVE_JOIN, GetActorId(), NULL, 0 };
	CmdData cmdLeave = { CMDCODE_MOVE_LEAVE, GetActorId(), NULL, 0 };

	if(pOrignCell) {
		for(x=pOrignCell->GetAreaCol()-AREA_ACTION_NOTIFY_RANGE; x<=pOrignCell->GetAreaCol()+AREA_ACTION_NOTIFY_RANGE; x++) {
			if(x<0 || x>=GetArea()->AreaColCount()) continue;
			for(y=pOrignCell->GetAreaRow()-AREA_ACTION_NOTIFY_RANGE; y<=pOrignCell->GetAreaRow()+AREA_ACTION_NOTIFY_RANGE; y++) {
				if(y<0 || y>=GetArea()->AreaRowCount()) continue;

				if(pCell) {
					if(x>=pCell->GetAreaCol()-AREA_ACTION_NOTIFY_RANGE && x<=pCell->GetAreaCol()+AREA_ACTION_NOTIFY_RANGE) {
					if(y>=pCell->GetAreaRow()-AREA_ACTION_NOTIFY_RANGE && y<=pCell->GetAreaRow()+AREA_ACTION_NOTIFY_RANGE) {
						continue;
					}
					}
				}

				GetArea()->GetCell(x, y)->Notify(&cmdLeave);

				for(l=0; l<GetAreaCell()->GetActorIndexMax(); l++) {
					if(!GetAreaCell()->GetActorByIndex(l)) continue;
					CmdData cmd = { CMDCODE_MOVE_LEAVE, GetAreaCell()->GetActorByIndex(l)->GetActorId(), NULL, 0 };
					OnNotify(&cmd);
				}
			}
		}
	}

	if(pCell) {
		for(x=pCell->GetAreaCol()-AREA_ACTION_NOTIFY_RANGE; x<=pCell->GetAreaCol()+AREA_ACTION_NOTIFY_RANGE; x++) {
			if(x<0 || x>=GetArea()->AreaColCount()) continue;
			for(y=pCell->GetAreaRow()-AREA_ACTION_NOTIFY_RANGE; y<=pCell->GetAreaRow()+AREA_ACTION_NOTIFY_RANGE; y++) {
				if(y<0 || y>=GetArea()->AreaRowCount()) continue;

				if(pOrignCell) {
					if(x>=pOrignCell->GetAreaCol()-AREA_ACTION_NOTIFY_RANGE && x<=pOrignCell->GetAreaCol()+AREA_ACTION_NOTIFY_RANGE) {
					if(y>=pOrignCell->GetAreaRow()-AREA_ACTION_NOTIFY_RANGE && y<=pOrignCell->GetAreaRow()+AREA_ACTION_NOTIFY_RANGE) {
						continue;
					}
					}
				}

				GetArea()->GetCell(x, y)->Notify(&cmdJoin);

				for(l=0; l<GetAreaCell()->GetActorIndexMax(); l++) {
					if(!GetAreaCell()->GetActorByIndex(l)) continue;
					CmdData cmd = { CMDCODE_MOVE_JOIN, GetAreaCell()->GetActorByIndex(l)->GetActorId(), NULL, 0 };
					OnNotify(&cmd);
				}
			}
		}
	}
}

template<class TArea, class TAreaActor>
const Vector& CAreaActor<TArea, TAreaActor>::GetPosition() const
{
	return m_vecPosition;
}

template<class TArea, class TAreaActor>
float CAreaActor<TArea, TAreaActor>::GetDirection() const
{
	return m_fDirection;
}

template<class TArea, class TAreaActor>
void CAreaActor<TArea, TAreaActor>::SetTarget(TAreaActor* pTarget)
{
	m_pTarget = pTarget;
}

template<class TArea, class TAreaActor>
TAreaActor* CAreaActor<TArea, TAreaActor>::GetTarget()
{
	return m_pTarget;
}

template<class TArea, class TAreaActor>
void CAreaActor<TArea, TAreaActor>::SendNotify(const CmdData* pCmdData, int nRange)
{
	assert(m_pArea);
	assert(m_pAreaCell);

	if(m_pAreaCell) {
		m_pArea->Notify(pCmdData, m_pAreaCell->GetAreaCol(), m_pAreaCell->GetAreaRow(), nRange);
	}
}

template<class TArea, class TAreaActor>
void CAreaActor<TArea, TAreaActor>::SendNotify(const CmdData* pCmdData, float fRange)
{
	assert(m_pArea);

	m_pArea->Notify(pCmdData, m_vecPosition, fRange);
}

template<class TArea, class TAreaActor>
void CAreaActor<TArea, TAreaActor>::Tick(unsigned int nCurTime, unsigned int nDelta)
{
	if(m_nWalkTime) {
		if(m_nWalkTime<=nDelta) {
			m_nWalkTime = 0;
			SetPosition(m_vecDestination, m_fDirection);
		} else {
			Vector d;
			float length, step;
			length = VectorDistance(m_vecDestination, m_vecPosition);
			step = m_fVelocity * nDelta;
			d.x = m_vecPosition.x + (m_vecDestination.x - m_vecPosition.x) / length * step;
			d.y = m_vecPosition.y + (m_vecDestination.y - m_vecPosition.y) / length * step;
			d.z = m_vecPosition.z + (m_vecDestination.z - m_vecPosition.z) / length * step;
			SetPosition(d, m_fDirection);
			m_nWalkTime -= nDelta;
		}
	}
}
