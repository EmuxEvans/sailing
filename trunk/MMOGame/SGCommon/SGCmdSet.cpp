#include <string.h>
#include <assert.h>
#include <string>
#include <vector>

#include "CmdData.h"
#include "SGCode.h"
#include "SGData.h"

#include "SGCmdSet.h"

#define PushCCmd m_ClientCmdSet.PushCmd
#define PushCArg m_ClientCmdSet.PushArg
#define PushSCmd m_ServerCmdSet.PushCmd
#define PushSArg m_ServerCmdSet.PushArg

CSGCmdSetManage::CSGCmdSetManage()
{
	PushSCmd("login_seed", SGCMDCODE_LOGIN_SEED, "服务端通知客户端用来扰乱密码的salt");
	PushSArg("salt", CMDARG_TYPE_BYTE|CMDARG_TYPE_ARRAY);

	PushCCmd("login", SGCMDCODE_LOGIN, "客户端发送username & password");
	PushCArg("username", CMDARG_TYPE_STRING);
	PushCArg("password", CMDARG_TYPE_STRING, "salt:password md5 之后的结果");

	PushSCmd("login_return", SGCMDCODE_LOGIN_RETURN, "服务器返回结果");
	PushSArg("result", CMDARG_TYPE_DWORD, "返回值, 0=成功");

	PushCCmd("create", SGCMDCODE_CREATE, "创建角色");
	PushCArg("username", CMDARG_TYPE_STRING);
	PushCArg("sex", CMDARG_TYPE_DWORD);

	PushCCmd("eqips", SGCMDCODE_EQUIP, "玩家装备");
	PushCArg("face", CMDARG_TYPE_DWORD, "");
	PushCArg("hair", CMDARG_TYPE_DWORD, "");
	PushCArg("headwear", CMDARG_TYPE_DWORD, "");
	PushCArg("helmet", CMDARG_TYPE_DWORD, "");
	PushCArg("chest", CMDARG_TYPE_DWORD, "");
	PushCArg("cuff", CMDARG_TYPE_DWORD, "");
	PushCArg("belt", CMDARG_TYPE_DWORD, "");
	PushCArg("shoulder", CMDARG_TYPE_DWORD, "");
	PushCArg("clock", CMDARG_TYPE_DWORD, "");
	PushCArg("pants", CMDARG_TYPE_DWORD, "");
	PushCArg("shoes", CMDARG_TYPE_DWORD, "");

	PushCCmd("move", SGCMDCODE_MOVE, "玩家移动");
	PushCArg("sx", CMDARG_TYPE_FLOAT, "当前坐标");
	PushCArg("sy", CMDARG_TYPE_FLOAT);
	PushCArg("sz", CMDARG_TYPE_FLOAT);
	PushCArg("ex", CMDARG_TYPE_FLOAT, "预测结束点坐标");
	PushCArg("ey", CMDARG_TYPE_FLOAT);
	PushCArg("ez", CMDARG_TYPE_FLOAT);
	PushCArg("time", CMDARG_TYPE_DWORD, "预测花费的时间");

	PushSCmd("move", SGCMDCODE_MOVE, "服务器通知对象移动");
	PushSArg("actor", CMDARG_TYPE_DWORD, "对象的id");
	PushSArg("sx", CMDARG_TYPE_FLOAT, "当前坐标");
	PushSArg("sy", CMDARG_TYPE_FLOAT);
	PushSArg("sz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT, "预测结束点坐标");
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD, "预测花费的时间");

	PushSCmd("telport", SGCMDCODE_TELPORT, "瞬移到目标点");
	PushSArg("sx", CMDARG_TYPE_FLOAT, "目标坐标");
	PushSArg("sy", CMDARG_TYPE_FLOAT);
	PushSArg("sz", CMDARG_TYPE_FLOAT);
	PushSArg("direction", CMDARG_TYPE_FLOAT, "面的朝向");

	PushCCmd("mapsay", SGCMDCODE_MAPCHAT_SAY, "在地图上说话");
	PushCArg("body", CMDARG_TYPE_STRING, "内容");

	PushCCmd("mapmsay", SGCMDCODE_MAPCHAT_MSAY, "在地图上私聊");
	PushCArg("who", CMDARG_TYPE_STRING, "和谁");
	PushCArg("body", CMDARG_TYPE_STRING, "内容");

	PushSCmd("mapsay", SGCMDCODE_MAPCHAT_SAY, "服务器返回有人在地图上说话");
	PushSArg("who", CMDARG_TYPE_STRING, "谁");
	PushSArg("body", CMDARG_TYPE_STRING, "说什么");

	PushSCmd("mapmsay", SGCMDCODE_MAPCHAT_MSAY, "服务器通知有人在地图上私聊");
	PushSArg("who", CMDARG_TYPE_STRING, "谁");
	PushSArg("body", CMDARG_TYPE_STRING, "说什么");

	PushSCmd("move_join_player", SGCMDCODE_MOVE_JOIN_PLAYER, "有玩家进入视野范围");
	PushSArg("actorid", CMDARG_TYPE_DWORD, "玩家的actorid");
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA), "玩家的外表数据");
	PushSArg("cx", CMDARG_TYPE_FLOAT, "开始点坐标");
	PushSArg("cy", CMDARG_TYPE_FLOAT);
	PushSArg("cz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT, "预测结束点坐标");
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD, "预测花费时间");
	PushSArg("direction", CMDARG_TYPE_FLOAT, "面朝方向");
	PushSCmd("move_join_npc", SGCMDCODE_MOVE_JOIN_NPC, "有NPC进入视野范围");
	PushSArg("actorid", CMDARG_TYPE_DWORD, "NPC的actorid");
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGNPC_VIEWDATA", sizeof(SGNPC_VIEWDATA), "NPC的外表数据");
	PushSArg("cx", CMDARG_TYPE_FLOAT, "开始点坐标");
	PushSArg("cy", CMDARG_TYPE_FLOAT);
	PushSArg("cz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT, "预测结束点坐标");
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD, "预测花费时间");
	PushSArg("direction", CMDARG_TYPE_FLOAT, "面朝方向");
	PushSCmd("move_join_pet", SGCMDCODE_MOVE_JOIN_PET, "有宠物进入视野范围");
	PushSArg("actorid", CMDARG_TYPE_DWORD, "宠物的actorid");
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPET_VIEWDATA", sizeof(SGPET_VIEWDATA), "宠物的外表数据");
	PushSArg("cx", CMDARG_TYPE_FLOAT, "开始点坐标");
	PushSArg("cy", CMDARG_TYPE_FLOAT);
	PushSArg("cz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT, "预测结束点坐标");
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD, "预测花费时间");
	PushSArg("direction", CMDARG_TYPE_FLOAT, "面朝方向");
	PushSCmd("move_join_battle", SGCMDCODE_MOVE_JOIN_BATTLE, "有战斗进入视野范围");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGBATTLEFIELD_VIEWDATA", sizeof(SGBATTLEFIELD_VIEWDATA), "战斗的外表数据");
	PushSArg("cx", CMDARG_TYPE_FLOAT, "坐标");
	PushSArg("cy", CMDARG_TYPE_FLOAT);
	PushSArg("cz", CMDARG_TYPE_FLOAT);

	PushSCmd("move_change_player", SGCMDCODE_MOVE_CHANGE_PLAYER, "玩家外表数据变化了");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushSCmd("move_change_npc", SGCMDCODE_MOVE_CHANGE_NPC, "NPC外表数据变化了");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGNPC_VIEWDATA", sizeof(SGNPC_VIEWDATA));
	PushSCmd("move_change_pet", SGCMDCODE_MOVE_CHANGE_PET, "宠物外表数据变化了");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPET_VIEWDATA", sizeof(SGPET_VIEWDATA));
	PushSCmd("move_change_battle", SGCMDCODE_MOVE_CHANGE_BATTLE, "战斗外表数据变化了");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGBATTLEFIELD_VIEWDATA", sizeof(SGBATTLEFIELD_VIEWDATA));

	PushSCmd("move_leave", SGCMDCODE_MOVE_LEAVE, "对象离开视野范围");
	PushSArg("actorid", CMDARG_TYPE_DWORD, "对象的actorid");

	PushCCmd("target_set", SGCMDCODE_TARGET_SET);
	PushCArg("targetid", CMDARG_TYPE_DWORD);

	PushSCmd("target_change", SGCMDCODE_TARGET_SET);
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("targetid", CMDARG_TYPE_DWORD);

	PushCCmd("team_create", SGCMDCODE_TEAM_CREATE, "创建 team");
	PushCCmd("team_join", SGCMDCODE_TEAM_JOIN, "加入 team");
	PushCArg("actorid", CMDARG_TYPE_DWORD);
	PushCCmd("team_invite", SGCMDCODE_TEAM_INVITE, "邀请别人加入 team");
	PushCArg("actorid", CMDARG_TYPE_DWORD);

	PushSCmd("team_join", SGCMDCODE_TEAM_JOIN, "请求加入team");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("name", CMDARG_TYPE_STRING);
	PushSArg("token", CMDARG_TYPE_STRING);
	PushSCmd("team_invite", SGCMDCODE_TEAM_INVITE, "收到邀请加入team");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("name", CMDARG_TYPE_STRING);
	PushSArg("token", CMDARG_TYPE_STRING);

	PushCCmd("team_accept", SGCMDCODE_TEAM_ACCEPT, "确认请求");
	PushCArg("actorid", CMDARG_TYPE_DWORD);
	PushCArg("token", CMDARG_TYPE_STRING);

	PushSCmd("team_info", SGCMDCODE_TEAM_INFO, "返回队伍信息");
	PushSArg("leader", CMDARG_TYPE_DWORD);
	PushSArg("infos", CMDARG_TYPE_STRUCT|CMDARG_TYPE_ARRAY, "SGTEAM_MEMBER_INFO", sizeof(SGTEAM_MEMBER_INFO));
	PushSCmd("team_change", SGCMDCODE_TEAM_CHANGE, "返回队伍信息改变");
	PushSArg("info", CMDARG_TYPE_STRUCT, "SGTEAM_MEMBER_INFO", sizeof(SGTEAM_MEMBER_INFO));

	PushCCmd("team_leader", SGCMDCODE_TEAM_LEADER, "设置队伍队长");
	PushCArg("leader", CMDARG_TYPE_DWORD);
	PushSCmd("team_leader", SGCMDCODE_TEAM_LEADER, "队伍队长变化");
	PushSArg("leader", CMDARG_TYPE_DWORD);

	PushCCmd("team_say", SGCMDCODE_TEAM_SAY, "在队伍里说话");
	PushCArg("body", CMDARG_TYPE_STRING);
	PushSCmd("team_say", SGCMDCODE_TEAM_SAY, "有人在队伍里说话");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("body", CMDARG_TYPE_STRING);

	PushCCmd("team_kick", SGCMDCODE_TEAM_KICK, "把人踢出队伍");
	PushCArg("actorid", CMDARG_TYPE_DWORD, "把谁踢出队伍");
	PushCCmd("team_leave", SGCMDCODE_TEAM_LEAVE, "主动离开队伍");
	PushSCmd("team_leave", SGCMDCODE_TEAM_LEAVE, "通知有人离开队伍");
	PushSArg("actorid", CMDARG_TYPE_DWORD, "谁离开了队伍");

	PushCCmd("fight", SGCMDCODE_FIGHT, "开始战斗");
	PushCCmd("fight_join", SGCMDCODE_FIGHT_JOIN, "加入战斗");
	PushCArg("actorid", CMDARG_TYPE_DWORD, "battlefield的actorid");
	PushCCmd("fight_runaway", SGCMDCODE_FIGHT_RUNAWAY, "逃离战斗");

	PushSCmd("fight_start", SGCMDCODE_FIGHT_START, "开始战斗");
	PushSCmd("fight_end", SGCMDCODE_FIGHT_END, "战斗结束");

}
