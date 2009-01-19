#pragma once

class CCubeUser;
class CCubeRoom;
class CCubeMember;

typedef IGameUserController<CCubeUser> ICubeUserController;
typedef IGameRoomController<CCubeUser, CCubeRoom, CCubeMember> ICubeRoomController;

class CCubeUser : public CGameUser<CCubeUser>, public ICubeUserX
{
public:
	void* operator new (size_t n);
	void operator delete(void* p);

public:
	CCubeUser(ICubeUserController* pController) : CGameUser<CCubeUser>(pController) {
		m_pHandle = NULL;
	}
	virtual ~CCubeUser() {
	}

	void BindNetworkHandle(NETWORK_HANDLE handle) {
		m_pHandle = handle;
	}

	//
	virtual void Disconnect() {
	}
	virtual ICubeRoomX* GetCubeRoom() {
		return NULL;
	}
	virtual CubeRoleInfo* GetRoleInfo() {
		return NULL;
	}

	char m_szRecvBuf[2*1024];
	NETWORK_HANDLE m_pHandle;
};

class CCubeRoom : public CGameRoom<CCubeUser, CCubeRoom, CCubeMember, 8>, public ICubeRoomX
{
public:
	void* operator new (size_t n);
	void operator delete(void* p);

public:
	static unsigned int GetChannelType() { return 0; }

	CCubeRoom(ICubeRoomController* pController) : CGameRoom<CCubeUser, CCubeRoom, CCubeMember, 8>(pController) {
	}
	virtual ~CCubeRoom() {
	}

	//
	virtual os_int GetMemberCount() {
		return 0;
	}
};

class CCubeMember : public CGameMember<CCubeUser, CCubeRoom, CCubeMember>, public ICubeMemberX
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
	virtual ICubeUserX* GetUser() {
		return NULL;
	}
	virtual ICubeRoomX* GetRoom() {
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

class CCubeUserController : public ICubeUserController {
public:
	virtual void OnConnect(CCubeUser* pUser);
	virtual void OnDisconnect(CCubeUser* pUser);
	virtual void OnData(CCubeUser* pUser, const void* pData, unsigned int nSize);
	virtual void SendData(CCubeUser* pUser, const void* pData, unsigned int nSize);
	virtual void Disconnect();
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
