#pragma once

/************* SGCMDCODE for Server *************/

// cmdcode for user connection
#define SGCMDCODE_CONNECT			0x1010
#define SGCMDCODE_USERDATA			0x1011
#define SGCMDCODE_DISCONNECT		0x1012
#define SGCMDCODE_USERLOADED		0x1013

// cmdcode for battle field
#define SGCMDCODE_BATTLEFIELD_JOIN	0x1021
#define SGCMDCODE_BATTLEFIELD_LEAVE	0x1022
//#define SGCMDCODE_TEAM_JOIN		0x1023
//#define SGCMDCODE_TEAM_LEAVE		0x1024

/************* SGCMDCODE for Client *************/

// cmdcode for login
#define SGCMDCODE_LOGIN					0x5000
#define SGCMDCODE_LOGIN_SEED			0x5001
#define SGCMDCODE_LOGIN_RETURN			0x5002
#define SGCMDCODE_LOGIN_REPORT			0x5003
#define SGCMDCODE_LOGIN_SELECT			0x5004
#define SGCMDCODE_CREATE				0x5005

// cmdcode for player equip
#define SGCMDCODE_EQUIP					0x5600

// cmdcode for player move
#define SGCMDCODE_MOVE					0x5100
#define SGCMDCODE_MOVE_JOIN_PLAYER		0x5101
#define SGCMDCODE_MOVE_JOIN_NPC			0x5102
#define SGCMDCODE_MOVE_JOIN_PET			0x5103
#define SGCMDCODE_MOVE_JOIN_BATTLE		0x5104
#define SGCMDCODE_MOVE_CHANGE			0x5105
#define SGCMDCODE_MOVE_CHANGE_PLAYER	0x5106
#define SGCMDCODE_MOVE_CHANGE_NPC		0x5107
#define SGCMDCODE_MOVE_CHANGE_PET		0x5108
#define SGCMDCODE_MOVE_CHANGE_BATTLE	0x5109
#define SGCMDCODE_MOVE_LEAVE			0x510a
#define SGCMDCODE_TELPORT				0x510b

// cmdcode for target
#define SGCMDCODE_TARGET				0x5200
#define SGCMDCODE_TARGET_SET			0x5201
#define SGCMDCODE_TARGET_CHANGE			0x5201

// cmdcode for chat
#define SGCMDCODE_MAPSAY				0x5301
#define SGCMDCODE_SAY					0x5302

// cmdcode for team
#define SGCMDCODE_TEAM					0x5400
#define SGCMDCODE_TEAM_CREATE			0x5401
#define SGCMDCODE_TEAM_JOIN				0x5402
#define SGCMDCODE_TEAM_INVITE			0x5403
#define SGCMDCODE_TEAM_ACCEPT			0x5404
#define SGCMDCODE_TEAM_INFO				0x5405
#define SGCMDCODE_TEAM_CHANGE			0x5406
#define SGCMDCODE_TEAM_SAY				0x5407
#define SGCMDCODE_TEAM_LEADER			0x5408
#define SGCMDCODE_TEAM_KICK				0x5409
#define SGCMDCODE_TEAM_LEAVE			0x540a

// cmdcode for battlefield
#define SGCMDCODE_FIGHT					0X5500
#define SGCMDCODE_FIGHT_START			0X5501
#define SGCMDCODE_FIGHT_END				0X5502
#define SGCMDCODE_FIGHT_JOIN			0X5503
#define SGCMDCODE_FIGHT_RUNAWAY			0X5504
