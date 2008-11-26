#pragma once

class CCubeUser;
class CCubeRoom;
class CCubeMember;

typedef IGameUserController<CCubeUser> ICubeUserController;
typedef IGameRoomController<CCubeUser, CCubeRoom, CCubeMember> ICubeRoomController;

class CCubeUser : public CGameUser<CCubeUser>, public ICubeUser
{
public:
	CCubeUser(ICubeUserController* pController) : CGameUser<CCubeUser>(pController) {
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
	CCubeRoom(ICubeRoomController* pController) : CGameRoom<CCubeUser, CCubeRoom, CCubeMember, 8>(pController) {
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
