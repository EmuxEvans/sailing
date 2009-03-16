#include "StdAfx.h"

#include "..\Engine\CmdData.h"
#include "..\SGGame\SGCmdCode.h"
#include "SGServerCmdSet.h"

CSGServerCmdSet::CSGServerCmdSet()
{
	PushCmd("login_seed", SGCMDCODE_LOGIN_SEED);
	PushArg("salt", CMDARG_TYPE_BYTE|CMDARG_TYPE_ARRAY);
}
