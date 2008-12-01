#pragma once

class IGameUser;
class IGameChannel;

class IGameUser
{
public:
	virtual void Connect() = NULL;
	virtual void Disconnect() = NULL;
	virtual void OnData(const void* pData, int nSize) = NULL;
	virtual void SendData(const void* pData, int nSize) = NULL;
	virtual bool BindChannel(unsigned int& nIndex, IGameChannel* pRoom, int nCIdx) = NULL;
	virtual bool UnbindChannel(unsigned int nIndex, IGameChannel* pRoom, int nCIdx) = NULL;
};

class IGameChannel
{
public:
	virtual bool Join(IGameUser* pUser) = NULL;
	virtual void OnData(unsigned int nCIdx, const void* pData, int nSize) = NULL;
	virtual void Disconnect(unsigned int nCIdx) = NULL;
};

template<class TGameUser>
class IGameUserController
{
public:
	virtual void OnConnect(TGameUser* pUser) = NULL;
	virtual void OnDisconnect(TGameUser* pUser) = NULL;
	virtual void OnData(TGameUser* pUser, const void* pData, int nSize) = NULL;
};

template<class TGameUser>
class CGameUser : public IGameChannel
{
public:
	CGameUser(IGameUserController<TGameUser>* pController);
	virtual ~CGameUser();

	virtual void Connect();
	virtual void Disconnect();
	virtual void OnData(const void* pData, int nSize);
	virtual void SendData(const void* pData, int nSize);
	virtual bool BindChannel(unsigned int& nIndex, IGameChannel* pChannel, int nCIdx);
	virtual bool UnbindChannel(unsigned int nIndex, IGameChannel* pChannel, int nCIdx);

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
