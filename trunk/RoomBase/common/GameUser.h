#pragma once

//class IGameUser;
//class IGameChannel;

template<class TGameUser>
class IGameUser;
template<class TGameUser>
class IGameChannel;

template<class TGameUser>
class IGameUser
{
public:
	virtual void OnConnect() = NULL;
	virtual void OnDisconnect() = NULL;
	virtual void OnData(const void* pData, unsigned int nSize) = NULL;
	virtual void SendData(IGameChannel<TGameUser>* pChannel, const void* pData, unsigned int nSize) = NULL;
	virtual void Disconnect() = NULL;
	virtual bool BindChannel(IGameChannel<TGameUser>* pRoom, unsigned int nCIdx) = NULL;
	virtual bool UnbindChannel(IGameChannel<TGameUser>* pRoom, unsigned int nCIdx) = NULL;
};

template<class TGameUser>
class IGameChannel
{
public:
	virtual bool Join(TGameUser* pUser) = NULL;
	virtual void OnData(TGameUser* pUser, unsigned int nCIdx, const void* pData, unsigned int nSize) = NULL;
	virtual void Disconnect(TGameUser* pUser, unsigned int nCIdx) = NULL;

public:
	void SetChannel(const char* pName, unsigned int nType, bool bDynamic) {
		m_pName = pName;
		m_nType = nType;
		m_bDynamic = bDynamic;
	}
	const char* GetName() const { return m_pName; }
	bool IsDynamic() const { return m_bDynamic; }
	unsigned int GetType() const { return m_nType; }
private:
	const char* m_pName;
	bool m_bDynamic;
	unsigned int m_nType;
};

template<class TGameUser>
class IGameUserController
{
public:
	virtual void OnConnect(TGameUser* pUser) = NULL;
	virtual void OnDisconnect(TGameUser* pUser) = NULL;
	virtual void OnData(TGameUser* pUser, const void* pData, unsigned int nSize) = NULL;
	virtual void SendData(TGameUser* pUser, IGameChannel<TGameUser>* pChannel, const void* pData, unsigned int nSize) = NULL;
	virtual void Disconnect() = NULL;
};

template<class TGameUser>
class CGameUser : public IGameUser<TGameUser>
{
public:
	CGameUser(IGameUserController<TGameUser>* pController);
	virtual ~CGameUser();

	virtual void OnConnect();
	virtual void OnDisconnect();
//	virtual void OnData(const void* pData, unsigned int nSize);
	virtual void SendData(IGameChannel<TGameUser>* pChannel, const void* pData, unsigned int nSize);
	virtual void Disconnect();
	virtual bool BindChannel(IGameChannel<TGameUser>* pChannel, unsigned int nCIdx);
	virtual bool UnbindChannel(IGameChannel<TGameUser>* pChannel, unsigned int nCIdx);

private:
	struct {
		char						szName[20];
		IGameChannel<TGameUser>*	pChannel;
		unsigned int				nCIdx;
	}								m_DynChannel[20];
	struct {
		char						szName[20];
		IGameChannel<TGameUser>*	pChannel;
		unsigned int				nCIdx;
	}								m_StaChannel[20];
	IGameUserController<TGameUser>*	m_pController;
};

unsigned int GenChannelSeq();
