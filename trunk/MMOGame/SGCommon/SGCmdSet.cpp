#include <string.h>
#include <assert.h>
#include <string>
#include <vector>

#include "..\Engine\CmdData.h"
#include "SGCmdCode.h"
#include "SGData.h"

#include "SGCmdSet.h"

#define PushCCmd m_ClientCmdSet.PushCmd
#define PushCArg m_ClientCmdSet.PushArg
#define PushSCmd m_ServerCmdSet.PushCmd
#define PushSArg m_ServerCmdSet.PushArg

CSGCmdSetManage::CSGCmdSetManage()
{
	PushSCmd("login_seed", SGCMDCODE_LOGIN_SEED);
	PushSArg("salt", CMDARG_TYPE_BYTE|CMDARG_TYPE_ARRAY);

	PushCCmd("login", SGCMDCODE_LOGIN);
	PushCArg("username", CMDARG_TYPE_STRING);
	PushCArg("password", CMDARG_TYPE_STRING);

	PushSCmd("login_return", SGCMDCODE_LOGIN_RETURN);
	PushSArg("result", CMDARG_TYPE_DWORD);

	PushCCmd("move", SGCMDCODE_MOVE);
	PushCArg("sx", CMDARG_TYPE_FLOAT);
	PushCArg("sy", CMDARG_TYPE_FLOAT);
	PushCArg("sz", CMDARG_TYPE_FLOAT);
	PushCArg("ex", CMDARG_TYPE_FLOAT);
	PushCArg("ey", CMDARG_TYPE_FLOAT);
	PushCArg("ez", CMDARG_TYPE_FLOAT);
	PushCArg("time", CMDARG_TYPE_DWORD);

	PushSCmd("move", SGCMDCODE_MOVE);
	PushSArg("actor", CMDARG_TYPE_DWORD);
	PushSArg("sx", CMDARG_TYPE_FLOAT);
	PushSArg("sy", CMDARG_TYPE_FLOAT);
	PushSArg("sz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT);
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD);

	PushCCmd("mapsay", SGCMDCODE_MAPCHAT_SAY);
	PushCArg("body", CMDARG_TYPE_STRING);

	PushCCmd("mapmsay", SGCMDCODE_MAPCHAT_MSAY);
	PushCArg("who", CMDARG_TYPE_STRING);
	PushCArg("body", CMDARG_TYPE_STRING);

	PushSCmd("mapsay", SGCMDCODE_MAPCHAT_SAY);
	PushSArg("who", CMDARG_TYPE_STRING);
	PushSArg("body", CMDARG_TYPE_STRING);

	PushSCmd("mapmsay", SGCMDCODE_MAPCHAT_MSAY);
	PushSArg("who", CMDARG_TYPE_STRING);
	PushSArg("body", CMDARG_TYPE_STRING);

	PushSCmd("move_join_player", SGCMDCODE_MOVE_JOIN_PLAYER);
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushSArg("cx", CMDARG_TYPE_FLOAT);
	PushSArg("cy", CMDARG_TYPE_FLOAT);
	PushSArg("cz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT);
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD);
	PushSArg("direction", CMDARG_TYPE_FLOAT);
	PushSCmd("move_join_npc", SGCMDCODE_MOVE_JOIN_NPC);
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushSArg("cx", CMDARG_TYPE_FLOAT);
	PushSArg("cy", CMDARG_TYPE_FLOAT);
	PushSArg("cz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT);
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD);
	PushSArg("direction", CMDARG_TYPE_FLOAT);
	PushSCmd("move_join_pet", SGCMDCODE_MOVE_JOIN_PET);
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushSArg("cx", CMDARG_TYPE_FLOAT);
	PushSArg("cy", CMDARG_TYPE_FLOAT);
	PushSArg("cz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT);
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD);
	PushSArg("direction", CMDARG_TYPE_FLOAT);
	PushSCmd("move_join_battle", SGCMDCODE_MOVE_JOIN_BATTLE);
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushSArg("cx", CMDARG_TYPE_FLOAT);
	PushSArg("cy", CMDARG_TYPE_FLOAT);
	PushSArg("cz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT);
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD);
	PushSArg("direction", CMDARG_TYPE_FLOAT);

	PushSCmd("move_change_player", SGCMDCODE_MOVE_CHANGE_PLAYER);
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushSCmd("move_change_npc", SGCMDCODE_MOVE_CHANGE_NPC);
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushSCmd("move_change_pet", SGCMDCODE_MOVE_CHANGE_PET);
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushSCmd("move_change_battle", SGCMDCODE_MOVE_CHANGE_BATTLE);
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));

	PushSCmd("move_leave", SGCMDCODE_MOVE_LEAVE);
	PushSArg("actorid", CMDARG_TYPE_DWORD);

	PushCCmd("target_set", SGCMDCODE_TARGET_SET);
	PushCArg("targetid", CMDARG_TYPE_DWORD);

	PushSCmd("target_change", SGCMDCODE_TARGET_SET);
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("targetid", CMDARG_TYPE_DWORD);

}
