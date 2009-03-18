#include "StdAfx.h"

#include "..\Engine\CmdData.h"
#include "..\SGGame\SGCmdCode.h"
#include "SGClientCmdSet.h"

CSGClientCmdSet::CSGClientCmdSet()
{
	PushCmd("login", SGCMDCODE_LOGIN);
	PushArg("username", CMDARG_TYPE_STRING);
	PushArg("password", CMDARG_TYPE_STRING);

	PushCmd("move", SGCMDCODE_MOVE);
	PushArg("sx", CMDARG_TYPE_FLOAT);
	PushArg("sy", CMDARG_TYPE_FLOAT);
	PushArg("sz", CMDARG_TYPE_FLOAT);
	PushArg("ex", CMDARG_TYPE_FLOAT);
	PushArg("ey", CMDARG_TYPE_FLOAT);
	PushArg("ez", CMDARG_TYPE_FLOAT);
	PushArg("time", CMDARG_TYPE_DWORD);

	PushCmd("mapsay", SGCMDCODE_MAPCHAT_SAY);
	PushArg("body", CMDARG_TYPE_STRING);

	PushCmd("mapmsay", SGCMDCODE_MAPCHAT_MSAY);
	PushArg("who", CMDARG_TYPE_STRING);
	PushArg("body", CMDARG_TYPE_STRING);

	PushCmd("target_set", SGCMDCODE_TARGET_SET);
	PushArg("targetid", CMDARG_TYPE_DWORD);
}
