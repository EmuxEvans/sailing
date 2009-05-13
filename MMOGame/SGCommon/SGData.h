#pragma once

#define SGEQUIPMENT_COUNT			7
#define SGBAG_MAX					20
#define SGWAREHOUSE_MAX				80

// ��װ����
#define SGEQUIPMENT_HEAD			0  // ͷ
#define SGEQUIPMENT_NECK			1  // ����
#define SGEQUIPMENT_BACK			2  // ��
#define SGEQUIPMENT_ARM				3  // �ֱ�
#define SGEQUIPMENT_SHOULDER		4  // ���
#define SGEQUIPMENT_LHAND			5  // ����
#define SGEQUIPMENT_RHAND			6  // ����
#define SGEQUIPMENT_LFINGER			7  // ��ߵ���ָ
#define SGEQUIPMENT_RFINGER			8  // �ұߵ���ָ
#define SGEQUIPMENT_CLOTHES			9  // ����
#define SGEQUIPMENT_WAIST			10 // ����
#define SGEQUIPMENT_LEG				11 // ����
#define SGEQUIPMENT_FOOT			12 // ��

 // װ������ ��Χ 0~9999
#define SGITEM_TYPE_NECKLACE		1  // ����
#define SGITEM_TYPE_CLOAK			2  // ����
#define SGITEM_TYPE_RING			3  // ��ָ
#define SGITEM_TYPE_SHIELD			4  // ����

#define SGITEM_TYPE_DAGGER			10 // ذ��
#define SGITEM_TYPE_SWORD			11 // ���ֽ�
#define SGITEM_TYPE_BLADE			12 // ����
#define SGITEM_TYPE_SPEAR			13 // ��ì
#define SGITEM_TYPE_HANDPIKE		14 // ���
#define SGITEM_TYPE_HAMMER			15 // ����
#define SGITEM_TYPE_HANDAXE			16 // �ָ�
#define SGITEM_TYPE_FAN				17 // ����
#define SGITEM_TYPE_CLAW			18 // ȭ��
#define SGITEM_TYPE_DOUBLEDAGGER	20 // ˫��ذ��
#define SGITEM_TYPE_DOUBLESWORD		21 // ˫�ֽ�
#define SGITEM_TYPE_DOUBLEBLADE		22 // ˫�ֵ�
#define SGITEM_TYPE_DOUBLESPEAR		23 // ˫��ì
#define SGITEM_TYPE_DOUBLEHANDPIKE	24 // ˫�����
#define SGITEM_TYPE_DOUBLEHAMMER	25 // ˫�ִ�
#define SGITEM_TYPE_DOUBLEHANDAXE	26 // ˫�ָ�
#define SGITEM_TYPE_DOUBLEFAN		27 // ˫������
#define SGITEM_TYPE_DOUBLEBOW		28 // ˫����
#define SGITEM_TYPE_CROSSBOW		30 // ����
#define SGITEM_TYPE_GREATSWORD		31 // ��
#define SGITEM_TYPE_GREATBLADE		32 // ��
#define SGITEM_TYPE_GREATSPEAR		33 // ��ǹ
#define SGITEM_TYPE_GREATPIKE		34 // ���
#define SGITEM_TYPE_GREATHAMMER		35 // ��
#define SGITEM_TYPE_STICK			36 // ����
#define SGITEM_TYPE_GREATAXE		37 // ��
#define SGITEM_TYPE_SHORTBOW		38 // �̹�
#define SGITEM_TYPE_LONGBOW			39 // ����
#define SGITEM_TYPE_HEAVYCROSSBOW	40 // ����
#define SGITEM_TYPE_CART			41 // С����
#define SGITEM_TYPE_STAFF			42 // ����

 // װ������֮����λ�÷���
#define SGITEM_SUBTYPE_ARMET		0  // ͷ��
#define SGITEM_SUBTYPE_CHEST		1  // ����
#define SGITEM_SUBTYPE_CUFF			2  // ����
#define SGITEM_SUBTYPE_BELT			3  // ����
#define SGITEM_SUBTYPE_PANTS		4  // ����
#define SGITEM_SUBTYPE_SHOES		5  // Ь��
#define SGITEM_SUBTYPE_SHOUDER		6  // ����
 // װ������֮���߲��ʷ���
#define SGITEM_SUBTYPE_CLOTH		50 // ����
#define SGITEM_SUBTYPE_LEATHER		60 // Ƥ��
#define SGITEM_SUBTYPE_LIGHTARMOR	70 // ���
#define SGITEM_SUBTYPE_HEAVYARMOR	80 // �ؼ�

// ��ɫ����
#define SGACTORTYPE_NPC				0
#define SGACTORTYPE_PLAYER			1
#define SGACTORTYPE_PET				2
#define SGACTORTYPE_BATTLEFIELD		3
#define SGACTORTYPE_PROPS			4

// ��������
#define SGACTIONMASK_QUEST			0x0001
#define SGACTIONMASK_TALK			0x0002
#define SGACTIONMASK_TRADE			0x0004
#define SGACTIONMASK_OPERATE		0x0008

#define SGBATTLEFIELD_TEAM_RED		0
#define SGBATTLEFIELD_TEAM_BLUE		1

#define SGNICK_STRLEN				10

#include "SGDataDef.h"
