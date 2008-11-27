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
	static unsigned int GetRoomType() { return 0; }

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
	CCubeMember(CCubeUser* pUser, unsigned int nUIdx, CCubeRoom* pRoom, unsigned int nRIdx) : CGameMember<CCubeUser, CCubeRoom, CCubeMember>(pUser, nUIdx, pRoom, nRIdx) {
	}
	virtual ~CCubeMember() {
	}

	static CCubeMember* Create(CCubeUser* pUser, unsigned int nUIdx, CCubeRoom* pRoom, unsigned int nRIdx) {
		return NULL;
	}
	static void Delete(CCubeMember* pMember) {
	}

	//
	virtual char* GetNick() {
		return NULL;
	}
};
