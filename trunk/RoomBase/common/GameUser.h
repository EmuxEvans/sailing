#pragma once

class IGameUser;
class IGameChannel;

class IGameUser
{
public:
	virtual void OnConnect() = NULL;
	virtual void OnDisconnect() = NULL;
	virtual void OnData(const void* pData, unsigned int nSize) = NULL;
	virtual void SendData(const void* pData, unsigned int nSize) = NULL;
	virtual void Disconnect() = NULL;
	virtual bool BindChannel(IGameChannel* pRoom, unsigned int nCIdx) = NULL;
	virtual bool UnbindChannel(IGameChannel* pRoom, unsigned int nCIdx) = NULL;
};

class IGameChannel
{
public:
	virtual bool Join(IGameUser* pUser) = NULL;
	virtual void OnData(unsigned int nCIdx, const void* pData, unsigned int nSize) = NULL;
	virtual void Disconnect(unsigned int nCIdx) = NULL;
};

template<class TGameUser>
class IGameUserController
{
public:
	virtual void OnConnect(TGameUser* pUser) = NULL;
	virtual void OnDisconnect(TGameUser* pUser) = NULL;
	virtual void OnData(TGameUser* pUser, const void* pData, unsigned int nSize) = NULL;
	virtual void SendData(TGameUser* pUser, const void* pData, unsigned int nSize) = NULL;
	virtual void Disconnect() = NULL;
};

template<class TGameUser>
class CGameUser : public IGameUser
{
public:
	CGameUser(IGameUserController<TGameUser>* pController);
	virtual ~CGameUser();

	virtual void OnConnect();
	virtual void OnDisconnect();
	virtual void OnData(const void* pData, unsigned int nSize);
	virtual void SendData(const void* pData, unsigned int nSize);
	virtual void Disconnect();
	virtual bool BindChannel(IGameChannel* pChannel, unsigned int nCIdx);
	virtual bool UnbindChannel(IGameChannel* pChannel, unsigned int nCIdx);

private:
	struct {
		IGameChannel*	pChannel;
		unsigned int	nCIdx;
	}								m_DynChannel[20];
	struct {
		IGameChannel*	pChannel;
		unsigned int	nCIdx;
	}								m_ptaChannel[20];
	IGameUserController<TGameUser>*	m_pController;
};

unsigned int GenChannelSeq();
