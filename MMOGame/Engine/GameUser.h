#pragma once

template<class TConnection>
class IConnection;
template<class TConnection>
class IChannel;

template<class TConnection>
class IConnection
{
public:
	virtual ~IConnection() {}

	virtual void OnConnect() = 0;
	virtual void OnDisconnect() = 0;
	virtual void OnData(const void* pData, unsigned int nSize) = 0;

	virtual void SendData(unsigned short nChnIdx, const void* pData, unsigned int nSize) = 0;
	virtual void Disconnect() = 0;

	virtual bool AttachStaticChannel(unsigned short nType, const char* pName, IChannel<TConnection>* pChannel) = 0;
	virtual bool DetachStaticChannel(IChannel<TConnection>* pChannel, unsigned short nChnIdx) = 0;
	virtual bool AttachDynamicChannel(unsigned short nType, const char* pName, IChannel<TConnection>* pChannel) = 0;
	virtual bool DetachDynamicChannel(IChannel<TConnection>* pChannel, unsigned short nChnIdx) = 0;
};

template<class TConnection>
class IChannel
{
public:
	virtual ~IChannel() {}

	virtual void OnAttach(unsigned short nChnIdx, TConnection* pConn) = 0;
	virtual void OnDetach(unsigned short nChnIdx, TConnection* pConn) = 0;
	virtual void OnDisconnect(unsigned short nChnIdx, TConnection* pConn) = 0;
	virtual void OnData(unsigned short nChnIdx, TConnection* pConn, const void* pData, unsigned int nSize) = 0;
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
