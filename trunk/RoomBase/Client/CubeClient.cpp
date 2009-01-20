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

void CCubeClient::OnData()
{
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
	((CCubeClient*)userptr)->OnData();
}

void _on_disconnect(NETWORK_HANDLE handle, void* userptr)
{
	((CCubeClient*)userptr)->OnDiconnect();
}
