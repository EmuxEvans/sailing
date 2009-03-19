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

CSGArea::CSGArea() : CArea(1.0f, 1.0f, m_Cells, 99, 99)
{
}

CSGArea::~CSGArea()
{
}
