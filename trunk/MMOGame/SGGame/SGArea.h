#pragma once

class CSGArea;
class CSGAreaActor;

class CSGArea : public CArea<CSGArea, CSGAreaActor>
{
public:
	CSGArea();
	virtual ~CSGArea();

private:
	CAreaCell<CSGArea, CSGAreaActor> m_Cells[100*100];
};
