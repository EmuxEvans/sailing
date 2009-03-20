#pragma once

class CSGArea;
class CSGAreaActor;

class CSGPlayer;
class CSGNPC;
class CSGPet;
class CSGBattleField;

class CSGArea : public CArea<CSGArea, CSGAreaActor>
{
public:
	CSGArea();
	virtual ~CSGArea();

	CSGPlayer* GetPlayer(unsigned int nActorId);
	CSGNPC* GetNPC(unsigned int nActorId);
	CSGPet* GetPet(unsigned int nActorId);
	CSGBattleField* GetBattleField(unsigned int nActorId);

private:
	CAreaCell<CSGArea, CSGAreaActor> m_Cells[100*100];
};

typedef CAreaCell<CSGArea, CSGAreaActor> CSGAreaCell;
