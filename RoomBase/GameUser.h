#pragma once

class CCubeUser;
class CCubeRoom;
class CCubeMember;

typedef IGameUserCallback<CCubeUser> ICubeUserCallback;
typedef IGameRoomCallback<CCubeUser, CCubeRoom, CCubeMember> ICubeRoomCallback;

class CCubeUser : public CGameUser<CCubeUser>, public ICubeUser
{
public:
	CCubeUser(ICubeUserCallback* pCallback) : CGameUser<CCubeUser>(pCallback) {
	}
	virtual ~CCubeUser() {
	}

	//
	virtual void Disconnect() {
	}
};

class CCubeRoom : public CGameRoom<CCubeUser, CCubeRoom, CCubeMember, 8>, public ICubeRoom
{
public:
	CCubeRoom(ICubeRoomCallback* pCallback) : CGameRoom<CCubeUser, CCubeRoom, CCubeMember, 8>(pCallback) {
	}
	virtual ~CCubeRoom() {
	}

	//
	virtual os_int GetMemberCount() {
		return 0;
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

	//
	virtual char* GetNick() {
		return NULL;
	}
};
