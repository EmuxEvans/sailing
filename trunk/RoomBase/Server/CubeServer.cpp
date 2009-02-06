#include <assert.h>
#include <list>

#include <skates/skates.h>
#include <sailing/proto_net.hpp>

#include "../common/GameUser.h"
#include "../common/GameRoom.h"
#include "../common/GameUser.inl"
#include "../common/GameRoom.inl"

#include "../Cube.proto.h"
#include "../Cube.proto.h"
#include "../Cube.net.proto.h"
#include "../Cube.net.svr.hpp"
#include "CubeServer.proto.h"

#include "Cube.h"
#include "CubeServer.h"

MEMPOOL_HANDLE		user_pool = NULL;
MEMPOOL_HANDLE		room_pool = NULL;
MEMPOOL_HANDLE		memb_pool = NULL;
static SOCK_ADDR	cube_sa;

static std::list<CCubeUser*>	user_list;
static void onaccept(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname);
static CCubeUserController CubeUserController;

static void onaccept(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname);
static void onconnect(NETWORK_HANDLE handle, void* userptr);
static void ondata(NETWORK_HANDLE handle, void* userptr);
static void ondisconnect(NETWORK_HANDLE handle, void* userptr);

int cubeserver_init()
{
	user_pool = mempool_create("CUBE_USER_POOL", sizeof(CCubeUser), 0);
	room_pool = mempool_create("CUBE_ROOM_POOL", sizeof(CCubeRoom), 0);
	memb_pool = mempool_create("CUBE_MEMB_POOL", sizeof(CCubeMember), 0);

	sock_str2addr("0.0.0.0:9527", &cube_sa);
	return network_tcp_register(&cube_sa, onaccept, NULL);
}

int cubeserver_final()
{
	network_tcp_unregister(&cube_sa);

	mempool_destroy(memb_pool);
	mempool_destroy(room_pool);
	mempool_destroy(user_pool);
	return ERR_NOERROR;
}

void onaccept(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname)
{
	NETWORK_HANDLE handle;
	CCubeUser* user;
	NETWORK_EVENT event;

	user = new CCubeUser(&CubeUserController, TCP_TEXTMODE);
	if(!user) {
		sock_disconnect(sock);
		return;
	}

	event.OnConnect = onconnect;
	event.OnData = ondata;
	event.OnDisconnect = ondisconnect;
	event.recvbuf_buf = user->m_szRecvBuf;
	event.recvbuf_max = sizeof(user->m_szRecvBuf);
	event.recvbuf_pool = NULL;

	handle = network_add(sock, &event, user);
	if(!handle) {
		delete user;
		sock_disconnect(sock);
		return;
	}

	user->BindNetworkHandle(handle);
}

void onconnect(NETWORK_HANDLE handle, void* userptr)
{
	CCubeUser* user = (CCubeUser*)userptr;
	if(!user) {
		network_disconnect(handle);
		return;
	}

	user->OnConnect();
}

void ondata(NETWORK_HANDLE handle, void* userptr)
{
	CCubeUser* user = (CCubeUser*)userptr;
	if(!user) {
		return;
	}

	while(1) {
		unsigned short len;
		char pkg_buf[sizeof(user->m_szRecvBuf)];
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

		user->OnData(buf, (unsigned int)len);

		network_recvbuf_commit(handle, sizeof(len)+len);
	}
}

void ondisconnect(NETWORK_HANDLE handle, void* userptr)
{
	CCubeUser* user = (CCubeUser*)userptr;
	if(!user) {
		network_del(handle);
		return;
	}

	user->OnDisconnect();
	delete user;
	network_del(handle);
}
