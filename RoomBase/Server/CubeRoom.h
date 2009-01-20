#pragma once

class CCubeRoom : public CGameRoom<CCubeUser, CCubeRoom, CCubeMember, 8>, public ICubeRoom
{
public:
	void* operator new (size_t n);
	void operator delete(void* p);

public:
	static unsigned int GetChannelType() { return 0; }

	CCubeRoom(ICubeRoomController* pController) : CGameRoom<CCubeUser, CCubeRoom, CCubeMember, 8>(pController) {
		SetChannel("CubeRoom", 0, false);
	}
	virtual ~CCubeRoom() {
	}

	//
	virtual os_int GetMemberCount() {
		return 0;
	}
};

class CCubeMember : public CGameMember<CCubeUser, CCubeRoom, CCubeMember>, public ICubeMember
{
public:
	void* operator new (size_t n);
	void operator delete(void* p);

public:
	CCubeMember(CCubeUser* pUser, CCubeRoom* pRoom, unsigned int nCIdx) : CGameMember<CCubeUser, CCubeRoom, CCubeMember>(pUser, pRoom, nCIdx) {
	}
	virtual ~CCubeMember() {
	}

	//
	virtual ICubeUser* GetUser() {
		return NULL;
	}
	virtual ICubeRoom* GetRoom() {
		return NULL;
	}
	virtual os_dword GetUUID() {
		return NULL;
	}
	virtual char* GetNick() {
		return NULL;
	}
	virtual CubeMemberInfo* GetInfo() {
		return NULL;
	}
};

class CCubeRoomController : public ICubeRoomController {
public:
	virtual void OnCreate(CCubeRoom* pRoom);
	virtual void OnDestroy(CCubeRoom* pRoom);
	virtual bool MemberPrepareJoin(CCubeRoom* pRoom, CCubeUser* pUser);
	virtual void MemberJoin(CCubeRoom* pRoom, CCubeMember* pMember);
	virtual void MemberLeave(CCubeRoom* pRoom, CCubeMember* pMember);
	virtual void MemberOndata(CCubeRoom* pRoom, CCubeMember* pMember, const void* pData, unsigned int nSize);
};
