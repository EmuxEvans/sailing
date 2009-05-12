#pragma once

#define SGEQUIPMENT_COUNT			7
#define SGBAG_MAX					20
#define SGWAREHOUSE_MAX				80

// 人装备点
#define SGEQUIPMENT_HEAD			0 // 头
#define SGEQUIPMENT_NECK			1 // 脖子
#define SGEQUIPMENT_LHAND			2 // 左手
#define SGEQUIPMENT_RHAND			3 // 右手
#define SGEQUIPMENT_LFINGER			4 // 左边的手指
#define SGEQUIPMENT_RFINGER			5 // 右边的手指
#define SGEQUIPMENT_CLOTHES			6 // 上衣
#define SGEQUIPMENT_WAIST			7 // 腰部
#define SGEQUIPMENT_LEG				8 // 大腿
#define SGEQUIPMENT_FOOT			9 // 脚

 // 装备类型 范围 0~9999
#define SGITEM_TYPE_NECKLACE		1  // 项链
#define SGITEM_TYPE_CLOAK			2  // 披风
#define SGITEM_TYPE_RING			3  // 戒指
#define SGITEM_TYPE_DAGGER			4  // 匕首
#define SGITEM_TYPE_SWORD			5  // 单手剑
#define SGITEM_TYPE_BLADE			6  // 单刀
#define SGITEM_TYPE_SPEAR			7  // 短矛
#define SGITEM_TYPE_HANDPIKE		8  // 手戟
#define SGITEM_TYPE_HAMMER			9  // 单锤
#define SGITEM_TYPE_HANDAXE			10 // 手斧
#define SGITEM_TYPE_FAN				11 // 扇子
#define SGITEM_TYPE_SHIELD			12 // 盾牌
#define SGITEM_TYPE_CLAW			13 // 拳套
#define SGITEM_TYPE_CROSSBOW		14 // 轻弩
#define SGITEM_TYPE_GREATSWORD		15 // 大剑
#define SGITEM_TYPE_GREATBLADE		16 // 大刀
#define SGITEM_TYPE_GREATSPEAR		17 // 大枪
#define SGITEM_TYPE_GREATPIKE		18 // 大戢
#define SGITEM_TYPE_GREATHAMMER		19 // 大锤
#define SGITEM_TYPE_STICK			20 // 长棍
#define SGITEM_TYPE_GREATAXE		21 // 大斧
#define SGITEM_TYPE_SHORTBOW		22 // 短弓
#define SGITEM_TYPE_LONGBOW			23 // 长弓
#define SGITEM_TYPE_HEAVYCROSSBOW	24 // 重弩
#define SGITEM_TYPE_CART			25 // 小车车
#define SGITEM_TYPE_STAFF			26 // 法杖

 // 装备类型之防具位置分类
#define SGITEM_SUBTYPE_ARMET		0 // 头盔
#define SGITEM_SUBTYPE_CHEST		1 // 上衣
#define SGITEM_SUBTYPE_CUFF			2 // 护腕
#define SGITEM_SUBTYPE_BELT			3 // 腰带
#define SGITEM_SUBTYPE_PANTS		4 // 裤子
#define SGITEM_SUBTYPE_SHOES		5 // 鞋子
#define SGITEM_SUBTYPE_SHOUDER		6 // 护肩
 // 装备类型之防具材质分类
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
