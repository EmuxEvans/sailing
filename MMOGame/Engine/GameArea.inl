#pragma once

template<class TArea, class TAreaActor>
CArea<TArea, TAreaActor>::CArea(float fCellWidth, float fCellHeight, CAreaCell<TArea, TAreaActor>* pCells, int nColMax, int nRowMax)
{
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
int CArea<TArea, TAreaActor>::AreaColMax()
{
	return m_nColMax;
}

template<class TArea, class TAreaActor>
int CArea<TArea, TAreaActor>::AreaRowMax()
{
	return m_nRowMax;
}

template<class TArea, class TAreaActor>
CAreaCell<TArea, TAreaActor>* CArea<TArea, TAreaActor>::GetCell(int nCol, int nRow)
{
	if(nCol<0 || nCol>=m_nColMax) return NULL;
	if(nRow<0 || nRow>=m_nRowMax) return NULL;
	return m_pCells[nRow*m_nColMax+nCol];
}

template<class TArea, class TAreaActor>
CAreaCell<TArea, TAreaActor>* CArea<TArea, TAreaActor>::GetCell(const FAreaVector& vecPos)
{
	int nCol, nRow;
	nCol = (int)(vecPos.x/m_fCellWidth);
	nRow = (int)(vecPos.y/m_fCellHeight);
	return GetCell(nCol, nRow);
}

template<class TArea, class TAreaActor>
TAreaActor* CArea<TArea, TAreaActor>::GetActor(const FAreaVector& vecPos, float fRange, unsigned int nActorId)
{
	return NULL;
}

template<class TArea, class TAreaActor>
TAreaActor* CArea<TArea, TAreaActor>::GetActor(unsigned int nActorId)
{
	TAreaActor* pActor;
	for(int x=0; x<m_nColMax; x++) {
		for(int y=0; y<m_nRowMax; y++) {
			pActor = m_pCells[y*m_nColMax+x].GetActor(nActorId);
		}
	}
	return NULL;
}

template<class TArea, class TAreaActor>
void CArea<TArea, TAreaActor>::Notify(const FAreaVector& vecPos, float fRange, const FMsgBlock* pData)
{
}

template<class TArea, class TAreaActor>
void CArea<TArea, TAreaActor>::Notify(const FMsgBlock* pData)
{
	for(int x=0; x<m_nColMax; x++) {
		for(int y=0; y<m_nRowMax; y++) {
			m_pCells[y*m_nColMax+x].Notify(pData);
		}
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
void CAreaCell<TArea, TAreaActor>::SetArea(CArea<TArea, TAreaActor>* pAreaList, int nCol, int nRow)
{
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
TAreaActor* CAreaCell<TArea, TAreaActor>::GetActor(const FAreaVector& vecPos, float fRange, unsigned int nActorId)
{
	std::list<TAreaActor*>::iterator i;
	for(i=m_Actor.begin(); i!=m_Actor.end(); i++) {
		float fDistance = AreaVectorDistance((*i)->GetPosition(), vecPos);
		if(fDistance<fRange) {
			return (*i);
		}
	}
	return NULL;
}

template<class TArea, class TAreaActor>
TAreaActor* CAreaCell<TArea, TAreaActor>::GetActor(unsigned int nActorId)
{
	std::list<TAreaActor*>::iterator i;
	i = m_Actors.find(nActorId);
	if(i==m_Actors.end()) return NULL;
	return *i;
}

template<class TArea, class TAreaActor>
void CAreaCell<TArea, TAreaActor>::Notify(const FAreaVector& vecPos, float fRange, const FMsgBlock* pData)
{
	std::list<TAreaActor*>::iterator i;
	for(i=m_Actor.begin(); i!=m_Actor.end(); i++) {
		float fDistance = AreaVectorDistance((*i)->GetPosition(), vecPos);
		if(fDistance<fRange) {
			(*i)->OnNotify(pData);
		}
	}
}

template<class TArea, class TAreaActor>
void CAreaCell<TArea, TAreaActor>::Notify(const FMsgBlock* pData)
{
	std::list<TAreaActor*>::iterator i;
	for(i=m_Actor.begin(); i!=m_Actor.end(); i++) {
		(*i)->OnNotify(pData);
	}
}

template<class TArea, class TAreaActor>
void CAreaCell<TArea, TAreaActor>::InsertActor(TAreaActor* pActor)
{
	assert(m_Actors.find(pActor)!=m_Actors.end());
	m_Actors.push_back(pActor);
}

template<class TArea, class TAreaActor>
void CAreaCell<TArea, TAreaActor>::RemoveActor(TAreaActor* pActor)
{
	std::list<TAreaActor*>::iterator i;
	i = m_Actors.find(pActor);
	assert(i!=m_Actors.end());
	if(i==m_Actors.end()) return;
	m_Actors.earse(i);
}

template<class TArea, class TAreaActor>
CAreaActor<TArea, TAreaActor>::CAreaActor(unsigned int nActorId)
{
	m_nActorId = nActorId;
}

template<class TArea, class TAreaActor>
CAreaActor<TArea, TAreaActor>::~CAreaActor()
{
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
	m_pArea = pArea;
	m_pAreaCell = NULL;
}

template<class TArea, class TAreaActor>
void CAreaActor<TArea, TAreaActor>::SetAreaCell(TArea* pArea, CAreaCell<TArea, TAreaActor>* pCell)
{
	assert(m_pArea==pArea);
	m_pAreaCell = pCell;
}

template<class TArea, class TAreaActor>
void CAreaActor<TArea, TAreaActor>::SetPosition(const FAreaVector& vecPosition, const FAreaVector& vecDirection)
{
	m_vecPosition = vecPosition;
	m_vecDirection = vecDirection;
}

template<class TArea, class TAreaActor>
const FAreaVector& CAreaActor<TArea, TAreaActor>::GetPosition() const
{
	return m_vecPosition;
}

template<class TArea, class TAreaActor>
const FAreaVector& CAreaActor<TArea, TAreaActor>::GetDirection() const
{
	return m_vecDirection;
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
