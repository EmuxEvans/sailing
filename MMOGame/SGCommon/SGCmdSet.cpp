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
	PushSCmd("login_seed", SGCMDCODE_LOGIN_SEED, "�����֪ͨ�ͻ����������������salt");
	PushSArg("salt", CMDARG_TYPE_BYTE|CMDARG_TYPE_ARRAY);

	PushCCmd("login", SGCMDCODE_LOGIN, "�ͻ��˷���username & password");
	PushCArg("username", CMDARG_TYPE_STRING);
	PushCArg("password", CMDARG_TYPE_STRING, "salt:password md5 ֮��Ľ��");

	PushSCmd("login_return", SGCMDCODE_LOGIN_RETURN, "���������ؽ��");
	PushSArg("result", CMDARG_TYPE_DWORD, "����ֵ, 0=�ɹ�");

	PushCCmd("create", SGCMDCODE_CREATE, "������ɫ");
	PushCArg("username", CMDARG_TYPE_STRING);
	PushCArg("sex", CMDARG_TYPE_DWORD);

	PushCCmd("eqips", SGCMDCODE_EQUIP, "���װ��");
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

	PushCCmd("move", SGCMDCODE_MOVE, "����ƶ�");
	PushCArg("sx", CMDARG_TYPE_FLOAT, "��ǰ����");
	PushCArg("sy", CMDARG_TYPE_FLOAT);
	PushCArg("sz", CMDARG_TYPE_FLOAT);
	PushCArg("ex", CMDARG_TYPE_FLOAT, "Ԥ�����������");
	PushCArg("ey", CMDARG_TYPE_FLOAT);
	PushCArg("ez", CMDARG_TYPE_FLOAT);
	PushCArg("time", CMDARG_TYPE_DWORD, "Ԥ�⻨�ѵ�ʱ��");

	PushSCmd("move", SGCMDCODE_MOVE, "������֪ͨ�����ƶ�");
	PushSArg("actor", CMDARG_TYPE_DWORD, "�����id");
	PushSArg("sx", CMDARG_TYPE_FLOAT, "��ǰ����");
	PushSArg("sy", CMDARG_TYPE_FLOAT);
	PushSArg("sz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT, "Ԥ�����������");
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD, "Ԥ�⻨�ѵ�ʱ��");

	PushSCmd("telport", SGCMDCODE_TELPORT, "˲�Ƶ�Ŀ���");
	PushSArg("sx", CMDARG_TYPE_FLOAT, "Ŀ������");
	PushSArg("sy", CMDARG_TYPE_FLOAT);
	PushSArg("sz", CMDARG_TYPE_FLOAT);
	PushSArg("direction", CMDARG_TYPE_FLOAT, "��ĳ���");

	PushCCmd("mapsay", SGCMDCODE_MAPCHAT_SAY, "�ڵ�ͼ��˵��");
	PushCArg("body", CMDARG_TYPE_STRING, "����");

	PushCCmd("mapmsay", SGCMDCODE_MAPCHAT_MSAY, "�ڵ�ͼ��˽��");
	PushCArg("who", CMDARG_TYPE_STRING, "��˭");
	PushCArg("body", CMDARG_TYPE_STRING, "����");

	PushSCmd("mapsay", SGCMDCODE_MAPCHAT_SAY, "���������������ڵ�ͼ��˵��");
	PushSArg("who", CMDARG_TYPE_STRING, "˭");
	PushSArg("body", CMDARG_TYPE_STRING, "˵ʲô");

	PushSCmd("mapmsay", SGCMDCODE_MAPCHAT_MSAY, "������֪ͨ�����ڵ�ͼ��˽��");
	PushSArg("who", CMDARG_TYPE_STRING, "˭");
	PushSArg("body", CMDARG_TYPE_STRING, "˵ʲô");

	PushSCmd("move_join_player", SGCMDCODE_MOVE_JOIN_PLAYER, "����ҽ�����Ұ��Χ");
	PushSArg("actorid", CMDARG_TYPE_DWORD, "��ҵ�actorid");
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA), "��ҵ��������");
	PushSArg("cx", CMDARG_TYPE_FLOAT, "��ʼ������");
	PushSArg("cy", CMDARG_TYPE_FLOAT);
	PushSArg("cz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT, "Ԥ�����������");
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD, "Ԥ�⻨��ʱ��");
	PushSArg("direction", CMDARG_TYPE_FLOAT, "�泯����");
	PushSCmd("move_join_npc", SGCMDCODE_MOVE_JOIN_NPC, "��NPC������Ұ��Χ");
	PushSArg("actorid", CMDARG_TYPE_DWORD, "NPC��actorid");
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGNPC_VIEWDATA", sizeof(SGNPC_VIEWDATA), "NPC���������");
	PushSArg("cx", CMDARG_TYPE_FLOAT, "��ʼ������");
	PushSArg("cy", CMDARG_TYPE_FLOAT);
	PushSArg("cz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT, "Ԥ�����������");
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD, "Ԥ�⻨��ʱ��");
	PushSArg("direction", CMDARG_TYPE_FLOAT, "�泯����");
	PushSCmd("move_join_pet", SGCMDCODE_MOVE_JOIN_PET, "�г��������Ұ��Χ");
	PushSArg("actorid", CMDARG_TYPE_DWORD, "�����actorid");
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPET_VIEWDATA", sizeof(SGPET_VIEWDATA), "������������");
	PushSArg("cx", CMDARG_TYPE_FLOAT, "��ʼ������");
	PushSArg("cy", CMDARG_TYPE_FLOAT);
	PushSArg("cz", CMDARG_TYPE_FLOAT);
	PushSArg("ex", CMDARG_TYPE_FLOAT, "Ԥ�����������");
	PushSArg("ey", CMDARG_TYPE_FLOAT);
	PushSArg("ez", CMDARG_TYPE_FLOAT);
	PushSArg("time", CMDARG_TYPE_DWORD, "Ԥ�⻨��ʱ��");
	PushSArg("direction", CMDARG_TYPE_FLOAT, "�泯����");
	PushSCmd("move_join_battle", SGCMDCODE_MOVE_JOIN_BATTLE, "��ս��������Ұ��Χ");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGBATTLEFIELD_VIEWDATA", sizeof(SGBATTLEFIELD_VIEWDATA), "ս�����������");
	PushSArg("cx", CMDARG_TYPE_FLOAT, "����");
	PushSArg("cy", CMDARG_TYPE_FLOAT);
	PushSArg("cz", CMDARG_TYPE_FLOAT);

	PushSCmd("move_change_player", SGCMDCODE_MOVE_CHANGE_PLAYER, "���������ݱ仯��");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPLAYER_VIEWDATA", sizeof(SGPLAYER_VIEWDATA));
	PushSCmd("move_change_npc", SGCMDCODE_MOVE_CHANGE_NPC, "NPC������ݱ仯��");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGNPC_VIEWDATA", sizeof(SGNPC_VIEWDATA));
	PushSCmd("move_change_pet", SGCMDCODE_MOVE_CHANGE_PET, "����������ݱ仯��");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGPET_VIEWDATA", sizeof(SGPET_VIEWDATA));
	PushSCmd("move_change_battle", SGCMDCODE_MOVE_CHANGE_BATTLE, "ս��������ݱ仯��");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("viewdata", CMDARG_TYPE_STRUCT, "SGBATTLEFIELD_VIEWDATA", sizeof(SGBATTLEFIELD_VIEWDATA));

	PushSCmd("move_leave", SGCMDCODE_MOVE_LEAVE, "�����뿪��Ұ��Χ");
	PushSArg("actorid", CMDARG_TYPE_DWORD, "�����actorid");

	PushCCmd("target_set", SGCMDCODE_TARGET_SET);
	PushCArg("targetid", CMDARG_TYPE_DWORD);

	PushSCmd("target_change", SGCMDCODE_TARGET_SET);
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("targetid", CMDARG_TYPE_DWORD);

	PushCCmd("team_create", SGCMDCODE_TEAM_CREATE, "���� team");
	PushCCmd("team_join", SGCMDCODE_TEAM_JOIN, "���� team");
	PushCArg("actorid", CMDARG_TYPE_DWORD);
	PushCCmd("team_invite", SGCMDCODE_TEAM_INVITE, "������˼��� team");
	PushCArg("actorid", CMDARG_TYPE_DWORD);

	PushSCmd("team_join", SGCMDCODE_TEAM_JOIN, "�������team");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("name", CMDARG_TYPE_STRING);
	PushSArg("token", CMDARG_TYPE_STRING);
	PushSCmd("team_invite", SGCMDCODE_TEAM_INVITE, "�յ��������team");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("name", CMDARG_TYPE_STRING);
	PushSArg("token", CMDARG_TYPE_STRING);

	PushCCmd("team_accept", SGCMDCODE_TEAM_ACCEPT, "ȷ������");
	PushCArg("actorid", CMDARG_TYPE_DWORD);
	PushCArg("token", CMDARG_TYPE_STRING);

	PushSCmd("team_info", SGCMDCODE_TEAM_INFO, "���ض�����Ϣ");
	PushSArg("leader", CMDARG_TYPE_DWORD);
	PushSArg("infos", CMDARG_TYPE_STRUCT|CMDARG_TYPE_ARRAY, "SGTEAM_MEMBER_INFO", sizeof(SGTEAM_MEMBER_INFO));
	PushSCmd("team_change", SGCMDCODE_TEAM_CHANGE, "���ض�����Ϣ�ı�");
	PushSArg("info", CMDARG_TYPE_STRUCT, "SGTEAM_MEMBER_INFO", sizeof(SGTEAM_MEMBER_INFO));

	PushCCmd("team_leader", SGCMDCODE_TEAM_LEADER, "���ö���ӳ�");
	PushCArg("leader", CMDARG_TYPE_DWORD);
	PushSCmd("team_leader", SGCMDCODE_TEAM_LEADER, "����ӳ��仯");
	PushSArg("leader", CMDARG_TYPE_DWORD);

	PushCCmd("team_say", SGCMDCODE_TEAM_SAY, "�ڶ�����˵��");
	PushCArg("body", CMDARG_TYPE_STRING);
	PushSCmd("team_say", SGCMDCODE_TEAM_SAY, "�����ڶ�����˵��");
	PushSArg("actorid", CMDARG_TYPE_DWORD);
	PushSArg("body", CMDARG_TYPE_STRING);

	PushCCmd("team_kick", SGCMDCODE_TEAM_KICK, "�����߳�����");
	PushCArg("actorid", CMDARG_TYPE_DWORD, "��˭�߳�����");
	PushCCmd("team_leave", SGCMDCODE_TEAM_LEAVE, "�����뿪����");
	PushSCmd("team_leave", SGCMDCODE_TEAM_LEAVE, "֪ͨ�����뿪����");
	PushSArg("actorid", CMDARG_TYPE_DWORD, "˭�뿪�˶���");

	PushCCmd("fight", SGCMDCODE_FIGHT, "��ʼս��");
	PushCCmd("fight_join", SGCMDCODE_FIGHT_JOIN, "����ս��");
	PushCArg("actorid", CMDARG_TYPE_DWORD, "battlefield��actorid");
	PushCCmd("fight_runaway", SGCMDCODE_FIGHT_RUNAWAY, "����ս��");

	PushSCmd("fight_start", SGCMDCODE_FIGHT_START, "��ʼս��");
	PushSCmd("fight_end", SGCMDCODE_FIGHT_END, "ս������");

}
