#include <assert.h>

#include <skates/skates.h>

#include "../common/GameUser.h"
#include "../common/GameRoom.h"
#include "../common/GameUser.inl"
#include "../common/GameRoom.inl"

#include "../Cube.proto.h"
#include "../Cube.proto.h"
#include "../Cube.net.proto.h"
#include "../Cube.net.clt.hpp"
#include "CubeClient.proto.h"
#include "CubeClient.h"

static void _on_connect(NETWORK_HANDLE handle, void* userptr);
static void _on_data(NETWORK_HANDLE handle, void* userptr);
static void _on_disconnect(NETWORK_HANDLE handle, void* userptr);

CCubeClient::CCubeClient() : m_Login(this, TCP_TEXTMODE)
{
}

CCubeClient::~CCubeClient()
{
}

os_int CCubeClient::Connect(char* addr)
{
	SOCK_ADDR sa;
	SOCK_HANDLE sock;

	if(sock_str2addr(addr, &sa)==NULL) {
		return 0;
	}

	sock = sock_connect(&sa, SOCK_NONBLOCK);
	if(sock==SOCK_INVALID_HANDLE) {
		return 0;
	}

	NETWORK_EVENT event = {_on_connect, _on_data, _on_disconnect, NULL, m_RecvBuf, sizeof(m_RecvBuf)};

	if(network_add(sock, &event, this)==NULL) {
		sock_disconnect(sock);
		sock_close(sock);
		return 0;
	}

	return 1;
}

os_int CCubeClient::Disconnect()
{
	return 0;
}

void CCubeClient::OnConnect(NETWORK_HANDLE handle)
{
	m_hHandle = handle;
}

void CCubeClient::OnData(const void* pData, unsigned int nSize)
{
	const void* pUserData;
	unsigned int nUserDataSize;

	if(TCP_TEXTMODE) {
		const char* at;
		char name[100];
		at = strchr((const char*)pData, '.');
		if(!at) {
			assert(0);
			return;
		}
		memcpy(name, pData, at-(const char*)pData);
		name[at-(const char*)pData] = '\0';
		strtrim(strltrim(name));

		pUserData = at + 1;
		nUserDataSize = nSize - (at - (const char*)pData) - 1;
	} else {
		if(nSize<2) {
			assert(0);
			return;
		}

		pUserData = (const char*)pData + sizeof(unsigned short);
		nUserDataSize = nSize - sizeof(unsigned short);
	}

	m_Login.Dispatch(pUserData, nUserDataSize);
}

void CCubeClient::OnDiconnect()
{
	m_hHandle = NULL;
}

void _on_connect(NETWORK_HANDLE handle, void* userptr)
{
	((CCubeClient*)userptr)->OnConnect(handle);
}

void _on_data(NETWORK_HANDLE handle, void* userptr)
{
	while(1) {
		unsigned short len;
		char pkg_buf[1000];
		const void* buf;

		if(network_recvbuf_len(handle)<sizeof(len)) break;
		network_recvbuf_get(handle, &len, 0, sizeof(len));

		buf = network_recvbuf_ptr(handle, sizeof(len), len);
		if(!buf) {
			if(network_recvbuf_get(handle, pkg_buf, sizeof(len), len)!=ERR_NOERROR) {
				break;
			}
			buf = pkg_buf;
		}

		((CCubeClient*)userptr)->OnData(buf, (unsigned int)len);

		network_recvbuf_commit(handle, sizeof(len)+len);
	}

}

void _on_disconnect(NETWORK_HANDLE handle, void* userptr)
{
	((CCubeClient*)userptr)->OnDiconnect();
}
