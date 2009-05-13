#pragma once

#define SGEQUIPMENT_COUNT			7
#define SGBAG_MAX					20
#define SGWAREHOUSE_MAX				80

// 人装备点
#define SGEQUIPMENT_HEAD			0  // 头
#define SGEQUIPMENT_NECK			1  // 脖子
#define SGEQUIPMENT_BACK			2  // 背
#define SGEQUIPMENT_ARM				3  // 手臂
#define SGEQUIPMENT_SHOULDER		4  // 肩膀
#define SGEQUIPMENT_LHAND			5  // 左手
#define SGEQUIPMENT_RHAND			6  // 右手
#define SGEQUIPMENT_LFINGER			7  // 左边的手指
#define SGEQUIPMENT_RFINGER			8  // 右边的手指
#define SGEQUIPMENT_CLOTHES			9  // 上衣
#define SGEQUIPMENT_WAIST			10 // 腰部
#define SGEQUIPMENT_LEG				11 // 大腿
#define SGEQUIPMENT_FOOT			12 // 脚

 // 装备类型 范围 0~9999
#define SGITEM_TYPE_NECKLACE		1  // 项链
#define SGITEM_TYPE_CLOAK			2  // 披风
#define SGITEM_TYPE_RING			3  // 戒指
#define SGITEM_TYPE_SHIELD			4  // 盾牌

#define SGITEM_TYPE_DAGGER			10 // 匕首
#define SGITEM_TYPE_SWORD			11 // 单手剑
#define SGITEM_TYPE_BLADE			12 // 单刀
#define SGITEM_TYPE_SPEAR			13 // 短矛
#define SGITEM_TYPE_HANDPIKE		14 // 手戟
#define SGITEM_TYPE_HAMMER			15 // 单锤
#define SGITEM_TYPE_HANDAXE			16 // 手斧
#define SGITEM_TYPE_FAN				17 // 扇子
#define SGITEM_TYPE_CLAW			18 // 拳套
#define SGITEM_TYPE_DOUBLEDAGGER	20 // 双持匕首
#define SGITEM_TYPE_DOUBLESWORD		21 // 双持剑
#define SGITEM_TYPE_DOUBLEBLADE		22 // 双持刀
#define SGITEM_TYPE_DOUBLESPEAR		23 // 双持矛
#define SGITEM_TYPE_DOUBLEHANDPIKE	24 // 双持手戟
#define SGITEM_TYPE_DOUBLEHAMMER	25 // 双持锤
#define SGITEM_TYPE_DOUBLEHANDAXE	26 // 双持斧
#define SGITEM_TYPE_DOUBLEFAN		27 // 双持扇子
#define SGITEM_TYPE_DOUBLEBOW		28 // 双持弩
#define SGITEM_TYPE_CROSSBOW		30 // 轻弩
#define SGITEM_TYPE_GREATSWORD		31 // 大剑
#define SGITEM_TYPE_GREATBLADE		32 // 大刀
#define SGITEM_TYPE_GREATSPEAR		33 // 大枪
#define SGITEM_TYPE_GREATPIKE		34 // 大戢
#define SGITEM_TYPE_GREATHAMMER		35 // 大锤
#define SGITEM_TYPE_STICK			36 // 长棍
#define SGITEM_TYPE_GREATAXE		37 // 大斧
#define SGITEM_TYPE_SHORTBOW		38 // 短弓
#define SGITEM_TYPE_LONGBOW			39 // 长弓
#define SGITEM_TYPE_HEAVYCROSSBOW	40 // 重弩
#define SGITEM_TYPE_CART			41 // 小车车
#define SGITEM_TYPE_STAFF			42 // 法杖

 // 装备类型之防具位置分类
#define SGITEM_SUBTYPE_ARMET		0  // 头盔
#define SGITEM_SUBTYPE_CHEST		1  // 上衣
#define SGITEM_SUBTYPE_CUFF			2  // 护腕
#define SGITEM_SUBTYPE_BELT			3  // 腰带
#define SGITEM_SUBTYPE_PANTS		4  // 裤子
#define SGITEM_SUBTYPE_SHOES		5  // 鞋子
#define SGITEM_SUBTYPE_SHOUDER		6  // 护肩
 // 装备类型之防具材质分类
#define SGITEM_SUBTYPE_CLOTH		50 // 布甲
#define SGITEM_SUBTYPE_LEATHER		60 // 皮甲
#define SGITEM_SUBTYPE_LIGHTARMOR	70 // 轻甲
#define SGITEM_SUBTYPE_HEAVYARMOR	80 // 重甲

// 角色类新
#define SGACTORTYPE_NPC				0
#define SGACTORTYPE_PLAYER			1
#define SGACTORTYPE_PET				2
#define SGACTORTYPE_BATTLEFIELD		3
#define SGACTORTYPE_PROPS			4

// 动作类型
#define SGACTIONMASK_QUEST			0x0001
#define SGACTIONMASK_TALK			0x0002
#define SGACTIONMASK_TRADE			0x0004
#define SGACTIONMASK_OPERATE		0x0008

#define SGBATTLEFIELD_TEAM_RED		0
#define SGBATTLEFIELD_TEAM_BLUE		1

#define SGNICK_STRLEN				10

#include "SGDataDef.h"
