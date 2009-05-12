#pragma once

#define SGEQUIPMENT_COUNT			7
#define SGBAG_MAX					20
#define SGWAREHOUSE_MAX				80

// ��װ����
#define SGEQUIPMENT_HEAD			0 // ͷ
#define SGEQUIPMENT_NECK			1 // ����
#define SGEQUIPMENT_LHAND			2 // ����
#define SGEQUIPMENT_RHAND			3 // ����
#define SGEQUIPMENT_LFINGER			4 // ��ߵ���ָ
#define SGEQUIPMENT_RFINGER			5 // �ұߵ���ָ
#define SGEQUIPMENT_CLOTHES			6 // ����
#define SGEQUIPMENT_WAIST			7 // ����
#define SGEQUIPMENT_LEG				8 // ����
#define SGEQUIPMENT_FOOT			9 // ��

 // װ������ ��Χ 0~9999
#define SGITEM_TYPE_NECKLACE		1  // ����
#define SGITEM_TYPE_CLOAK			2  // ����
#define SGITEM_TYPE_RING			3  // ��ָ
#define SGITEM_TYPE_DAGGER			4  // ذ��
#define SGITEM_TYPE_SWORD			5  // ���ֽ�
#define SGITEM_TYPE_BLADE			6  // ����
#define SGITEM_TYPE_SPEAR			7  // ��ì
#define SGITEM_TYPE_HANDPIKE		8  // ���
#define SGITEM_TYPE_HAMMER			9  // ����
#define SGITEM_TYPE_HANDAXE			10 // �ָ�
#define SGITEM_TYPE_FAN				11 // ����
#define SGITEM_TYPE_SHIELD			12 // ����
#define SGITEM_TYPE_CLAW			13 // ȭ��
#define SGITEM_TYPE_CROSSBOW		14 // ����
#define SGITEM_TYPE_GREATSWORD		15 // ��
#define SGITEM_TYPE_GREATBLADE		16 // ��
#define SGITEM_TYPE_GREATSPEAR		17 // ��ǹ
#define SGITEM_TYPE_GREATPIKE		18 // ���
#define SGITEM_TYPE_GREATHAMMER		19 // ��
#define SGITEM_TYPE_STICK			20 // ����
#define SGITEM_TYPE_GREATAXE		21 // ��
#define SGITEM_TYPE_SHORTBOW		22 // �̹�
#define SGITEM_TYPE_LONGBOW			23 // ����
#define SGITEM_TYPE_HEAVYCROSSBOW	24 // ����
#define SGITEM_TYPE_CART			25 // С����
#define SGITEM_TYPE_STAFF			26 // ����

 // װ������֮����λ�÷���
#define SGITEM_SUBTYPE_ARMET		0 // ͷ��
#define SGITEM_SUBTYPE_CHEST		1 // ����
#define SGITEM_SUBTYPE_CUFF			2 // ����
#define SGITEM_SUBTYPE_BELT			3 // ����
#define SGITEM_SUBTYPE_PANTS		4 // ����
#define SGITEM_SUBTYPE_SHOES		5 // Ь��
#define SGITEM_SUBTYPE_SHOUDER		6 // ����
 // װ������֮���߲��ʷ���
#define SGITEM_SUBTYPE_CLOTH		30 //

//
#define SGACTORTYPE_NPC				0
#define SGACTORTYPE_PLAYER			1
#define SGACTORTYPE_PET				2
#define SGACTORTYPE_BATTLEFIELD		3
#define SGACTORTYPE_PROPS			4

//
#define SGACTIONMASK_QUEST			0x0001
#define SGACTIONMASK_TALK			0x0002
#define SGACTIONMASK_TRADE			0x0004
#define SGACTIONMASK_OPERATE		0x0008

#define SGBATTLEFIELD_TEAM_RED		0
#define SGBATTLEFIELD_TEAM_BLUE		1

#define SGNICK_STRLEN				10

#include "SGDataDef.h"
