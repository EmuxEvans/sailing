#pragma once

class CCubeUser;
class CCubeRoom;
class CCubeMember;

class CCubeUser : public CGameUser<CCubeUser>
{
public:
	CCubeUser();
	virtual ~CCubeUser();

};

class CCubeRoom : public CGameRoom<CCubeUser, CCubeRoom, CCubeMember, 8>
{
public:
	CCubeRoom();
	virtual ~CCubeRoom();

};

class CCubeMember : public CGameMember<CCubeUser, CCubeRoom, CCubeMember>
{
public:
	CCubeMember();
	virtual ~CCubeMember();

};
