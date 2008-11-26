#pragma once

class CCubeUser;
class CCubeRoom;
class CCubeMember;

typedef IGameUserCallback<CCubeUser> ICubeUserCallback;
typedef IGameRoomCallback<CCubeUser, CCubeRoom, CCubeMember> ICubeRoomCallback;

class CCubeUser : public CGameUser<CCubeUser>
{
public:
	CCubeUser(ICubeUserCallback* pCallback) : CGameUser<CCubeUser>(pCallback) {
	}
	virtual ~CCubeUser() {
	}

};

class CCubeRoom : public CGameRoom<CCubeUser, CCubeRoom, CCubeMember, 8>
{
public:
	CCubeRoom(ICubeRoomCallback* pCallback) : CGameRoom<CCubeUser, CCubeRoom, CCubeMember, 8>(pCallback) {
	}
	virtual ~CCubeRoom() {
	}

};

class CCubeMember : public CGameMember<CCubeUser, CCubeRoom, CCubeMember>
{
public:
	CCubeMember(CCubeUser* pUser, CCubeRoom* pRoom, unsigned int nUIdx) : CGameMember<CCubeUser, CCubeRoom, CCubeMember>(pUser, pRoom, nUIdx) {
	}
	virtual ~CCubeMember() {
	}

	static CCubeMember* Create(CCubeUser* pUser, CCubeRoom* pRoom, unsigned int nUIdx) {
		return NULL;
	}
	static void Delete(CCubeMember* pMember) {
	}
};
