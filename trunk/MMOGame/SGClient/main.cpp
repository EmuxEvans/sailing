#include <iostream>
#include <assert.h>
#include "SGClient.h"

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/sock.h>

#include "..\Engine\CmdData.h"
#include "..\SGGame\SGCmdCode.h"
#include "SGClient.h"

class CSGClientCallback : public ISGClientCallback
{
public:
	CSGClientCallback();
	virtual ~CSGClientCallback();

	virtual void OnConnect();
	virtual void OnData(const void* pData, unsigned int nSize);
	virtual void OnDisconnect();
};

int main(int argc, char* argv[])
{
	CSGClientCallback SGCallback;
	sock_init();
	ISGClient* pClient = CreateSGClient(&SGCallback, false);

	pClient->Connect("127.0.0.1:1980");
	pClient->Wait();
	{
		CDataBuffer<100> buf;
		buf.PutValue<unsigned short>(SGCMDCODE_LOGIN);
		buf.PutValue<unsigned int>(1999);
		buf.PutString("password");
		pClient->SendData(buf.GetBuffer(), buf.GetLength());
	}
	pClient->Wait();

	pClient->Release();
	sock_final();
	return 0;
}


CSGClientCallback::CSGClientCallback()
{
}

CSGClientCallback::~CSGClientCallback()
{
}

void CSGClientCallback::OnConnect()
{
}

void CSGClientCallback::OnData(const void* pData, unsigned int nSize)
{
}

void CSGClientCallback::OnDisconnect()
{
}
