#pragma once

//class IGameUser;
//class IGameChannel;

template<class TConnection>
class IConnection;
template<class TConnection>
class IChannel;

template<class TConnection>
class IConnection
{
public:
	virtual void OnConnect() = NULL;
	virtual void OnDisconnect() = NULL;
	virtual void OnData(const void* pData, unsigned int nSize) = NULL;

	virtual void SendData(unsigned short nChnIdx, const void* pData, unsigned int nSize) = NULL;
	virtual void Disconnect() = NULL;

	virtual bool AttachStaticChannel(unsigned short nType, const char* pName, IChannel<TConnection>* pChannel) = NULL;
	virtual bool DetachStaticChannel(IChannel<TConnection>* pChannel, unsigned short nChnIdx) = NULL;
	virtual bool AttachDynamicChannel(unsigned short nType, const char* pName, IChannel<TConnection>* pChannel) = NULL;
	virtual bool DetachDynamicChannel(IChannel<TConnection>* pChannel, unsigned short nChnIdx) = NULL;
};

template<class TConnection>
class IChannel
{
public:
	virtual void OnAttach(unsigned short nChnIdx, TConnection* pConn) = NULL;
	virtual void OnDetach(unsigned short nChnIdx, TConnection* pConn) = NULL;
	virtual void OnDisconnect(unsigned short nChnIdx, TConnection* pConn) = NULL;
	virtual void OnData(unsigned short nChnIdx, TConnection* pConn, const void* pData, unsigned int nSize) = NULL;
};

template<class TConnection>
class CConnection : public IConnection<TConnection>
{
public:
	CConnection();
	virtual ~CConnection();

	virtual void OnConnect();
	virtual void OnDisconnect();
	virtual void OnData(const void* pData, unsigned int nSize);

	virtual bool AttachStaticChannel(unsigned short nType, const char* pName, IChannel<TConnection>* pChannel);
	virtual bool DetachStaticChannel(IChannel<TConnection>* pChannel, unsigned short nChnIdx);
	virtual bool AttachDynamicChannel(unsigned short nType, const char* pName, IChannel<TConnection>* pChannel);
	virtual bool DetachDynamicChannel(IChannel<TConnection>* pChannel, unsigned short nChnIdx);

	IChannel<TConnection>* GetChannel(const char* pName);
	IChannel<TConnection>* GetChannel(unsigned short nChnIdx);

private:
	struct {
		char						szName[20];
		IChannel<TConnection>*		pChannel;
		unsigned int				nChnIdx;
	}								m_DynChannel[20];
	struct {
		unsigned short				nType;
		char						szName[20];
		IChannel<TConnection>*		pChannel;
		unsigned int				nChnIdx;
	}								m_StaChannel[20];
};

unsigned int GenChannelSeq();
