#include "StdAfx.h"

#include "..\Engine\CmdData.h"
#include "..\SGGame\SGCmdCode.h"
#include "..\SGGame\SGData.h"
#include "SGServerCmdSet.h"

CSGServerCmdSet::CSGServerCmdSet()
{
	PushCmd("login_seed", SGCMDCODE_LOGIN_SEED);
	PushArg("salt", CMDARG_TYPE_BYTE|CMDARG_TYPE_ARRAY);

	PushCmd("login_return", SGCMDCODE_LOGIN_RETURN);
	PushArg("result", CMDARG_TYPE_DWORD);

	PushCmd("move", SGCMDCODE_MOVE);
	PushArg("actor", CMDARG_TYPE_DWORD);
	PushArg("sx", CMDARG_TYPE_FLOAT);
	PushArg("sy", CMDARG_TYPE_FLOAT);
	PushArg("sz", CMDARG_TYPE_FLOAT);
	PushArg("ex", CMDARG_TYPE_FLOAT);
	PushArg("ey", CMDARG_TYPE_FLOAT);
	PushArg("ez", CMDARG_TYPE_FLOAT);
	PushArg("time", CMDARG_TYPE_DWORD);

	PushCmd("mapsay", SGCMDCODE_MAPCHAT_SAY);
	PushArg("who", CMDARG_TYPE_STRING);
	PushArg("body", CMDARG_TYPE_STRING);

	PushCmd("mapmsay", SGCMDCODE_MAPCHAT_MSAY);
	PushArg("who", CMDARG_TYPE_STRING);
	PushArg("body", CMDARG_TYPE_STRING);

	PushCmd("move_join_player", SGCMDCODE_MOVE_JOIN_PLAYER);
	PushArg("actorid", CMDARG_TYPE_DWORD);
	PushArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushArg("cx", CMDARG_TYPE_FLOAT);
	PushArg("cy", CMDARG_TYPE_FLOAT);
	PushArg("cz", CMDARG_TYPE_FLOAT);
	PushArg("ex", CMDARG_TYPE_FLOAT);
	PushArg("ey", CMDARG_TYPE_FLOAT);
	PushArg("ez", CMDARG_TYPE_FLOAT);
	PushArg("time", CMDARG_TYPE_DWORD);
	PushArg("direction", CMDARG_TYPE_FLOAT);
	PushCmd("move_join_npc", SGCMDCODE_MOVE_JOIN_NPC);
	PushArg("actorid", CMDARG_TYPE_DWORD);
	PushArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushArg("cx", CMDARG_TYPE_FLOAT);
	PushArg("cy", CMDARG_TYPE_FLOAT);
	PushArg("cz", CMDARG_TYPE_FLOAT);
	PushArg("ex", CMDARG_TYPE_FLOAT);
	PushArg("ey", CMDARG_TYPE_FLOAT);
	PushArg("ez", CMDARG_TYPE_FLOAT);
	PushArg("time", CMDARG_TYPE_DWORD);
	PushArg("direction", CMDARG_TYPE_FLOAT);
	PushCmd("move_join_pet", SGCMDCODE_MOVE_JOIN_PET);
	PushArg("actorid", CMDARG_TYPE_DWORD);
	PushArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushArg("cx", CMDARG_TYPE_FLOAT);
	PushArg("cy", CMDARG_TYPE_FLOAT);
	PushArg("cz", CMDARG_TYPE_FLOAT);
	PushArg("ex", CMDARG_TYPE_FLOAT);
	PushArg("ey", CMDARG_TYPE_FLOAT);
	PushArg("ez", CMDARG_TYPE_FLOAT);
	PushArg("time", CMDARG_TYPE_DWORD);
	PushArg("direction", CMDARG_TYPE_FLOAT);
	PushCmd("move_join_battle", SGCMDCODE_MOVE_JOIN_BATTLE);
	PushArg("actorid", CMDARG_TYPE_DWORD);
	PushArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushArg("cx", CMDARG_TYPE_FLOAT);
	PushArg("cy", CMDARG_TYPE_FLOAT);
	PushArg("cz", CMDARG_TYPE_FLOAT);
	PushArg("ex", CMDARG_TYPE_FLOAT);
	PushArg("ey", CMDARG_TYPE_FLOAT);
	PushArg("ez", CMDARG_TYPE_FLOAT);
	PushArg("time", CMDARG_TYPE_DWORD);
	PushArg("direction", CMDARG_TYPE_FLOAT);

	PushCmd("move_change_player", SGCMDCODE_MOVE_CHANGE_PLAYER);
	PushArg("actorid", CMDARG_TYPE_DWORD);
	PushArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushCmd("move_change_npc", SGCMDCODE_MOVE_CHANGE_NPC);
	PushArg("actorid", CMDARG_TYPE_DWORD);
	PushArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushCmd("move_change_pet", SGCMDCODE_MOVE_CHANGE_PET);
	PushArg("actorid", CMDARG_TYPE_DWORD);
	PushArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushCmd("move_change_battle", SGCMDCODE_MOVE_CHANGE_BATTLE);
	PushArg("actorid", CMDARG_TYPE_DWORD);
	PushArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));

	PushCmd("move_leave", SGCMDCODE_MOVE_LEAVE);
	PushArg("actorid", CMDARG_TYPE_DWORD);

	PushCmd("target_change", SGCMDCODE_TARGET_SET);
	PushArg("actorid", CMDARG_TYPE_DWORD);
	PushArg("targetid", CMDARG_TYPE_DWORD);
}

