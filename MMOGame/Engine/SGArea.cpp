#include <assert.h>
#include <map>

#include "Game.h"
#include "Math.h"
#include "VarCmd.h"

#include "GameArea.h"
#include "GameArea.inl"
#include "GameBuff.h"
#include "GameItem.h"
#include "GameSkill.h"

#include "SG.h"
#include "SGArea.h"
#include "SGAreaActor.h"
#include "SGBuff.h"
#include "SGItem.h"
#include "SGSkill.h"

CSGArea::CSGArea() : CArea(1.0f, 1.0f, m_Cells, 99, 99)
{
}

CSGArea::~CSGArea()
{
}
