#include <string.h>
#include <assert.h>
#include <map>
#include <vector>
#include <string>

#include "..\Engine\Game.h"

#include "SG.h"
#include "SGArea.h"
#include "SGAreaActor.h"
#include "SGPlayer.h"
#include "SGPet.h"
#include "SGBuff.h"
#include "SGItem.h"
#include "SGSkill.h"
#include "SGGameLoop.h"
#include "SGBattleField.h"
#include "SGPet.h"

CSGPet::CSGPet(CSGPlayer* pOwner) : CSGRole(SGACTORTYPE_PET), m_pOwner(pOwner)
{
	m_pOwner->OnPetAttach(this);
}

CSGPet::~CSGPet()
{
	m_pOwner->OnPetDetach(this);
}

bool CSGPet::GetViewData(CSGPlayer* pPlayer, SGPET_VIEWDATA* pData)
{
	pData->m_nOwner = m_pOwner->GetActorId();
	return true;
}
