#include <string.h>
#include <assert.h>
#include <map>

#include "..\Engine\Game.h"

#include "SG.h"
#include "SGArea.h"
#include "SGAreaActor.h"
#include "SGPlayer.h"
#include "SGBuff.h"
#include "SGItem.h"
#include "SGSkill.h"
#include "SGGameLoop.h"

CSGAreaActor::CSGAreaActor(int nActorType) : CAreaActor<CSGArea, CSGAreaActor>(CSGGameLoopCallback::AllocActorId(this)), m_nActorType(nActorType)
{
}

CSGAreaActor::~CSGAreaActor()
{
}

CSGRole::CSGRole(int nActorType) : CSGAreaActor(nActorType)
{
}

CSGRole::~CSGRole()
{
}
