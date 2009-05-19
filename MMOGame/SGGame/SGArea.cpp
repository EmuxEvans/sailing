#include <assert.h>
#include <map>
#include <vector>
#include <string>

#include "..\Engine\Game.h"

#include "..\SGCommon\SGCode.h"
#include "..\SGCommon\SGData.h"

#include "SGArea.h"
#include "SGAreaActor.h"
#include "SGBuff.h"
#include "SGItem.h"
#include "SGSkill.h"

CSGArea::CSGArea() : CArea(400.0f, 400.0f, m_Cells, 99, 99)
{
}

CSGArea::~CSGArea()
{
}

CSGPlayer* CSGArea::GetPlayer(unsigned int nActorId)
{
	CSGAreaActor* pActor = GetActor(nActorId);
	if(!pActor) return NULL;
	return pActor->GetActorType()==SGACTORTYPE_PLAYER?(CSGPlayer*)pActor:NULL;
}

CSGNPC* CSGArea::GetNPC(unsigned int nActorId)
{
	CSGAreaActor* pActor = GetActor(nActorId);
	if(!pActor) return NULL;
	return pActor->GetActorType()==SGACTORTYPE_NPC?(CSGNPC*)pActor:NULL;
}

CSGPet* CSGArea::GetPet(unsigned int nActorId)
{
	CSGAreaActor* pActor = GetActor(nActorId);
	if(!pActor) return NULL;
	return pActor->GetActorType()==SGACTORTYPE_PET?(CSGPet*)pActor:NULL;
}

CSGBattleField* CSGArea::GetBattleField(unsigned int nActorId)
{
	CSGAreaActor* pActor = GetActor(nActorId);
	if(!pActor) return NULL;
	return pActor->GetActorType()==SGACTORTYPE_BATTLEFIELD?(CSGBattleField*)pActor:NULL;
}
