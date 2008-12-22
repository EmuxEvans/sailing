#include <stdio.h>
#include <winsock2.h>

#include "cube_pdl.CubeClient.h"
#include "cube_pdl.CubeClient.hook.hpp"
#include "TCPConnection.h"
#include "OOKTCPConnection.h"

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	CTCPConnection::Init();

	COOKTCPConnection a, b, c, d;

	a.Connect("127.0.0.1:2008");
	b.Connect("127.0.0.1:2008");
	c.Connect("127.0.0.1:2008");
	d.Connect("127.0.0.1:2008");
	login_login(a.GetUserCtx(), "a a");
	login_create_player(a.GetUserCtx(), "a", "");
	login_login(b.GetUserCtx(), "b b");
	login_create_player(b.GetUserCtx(), "b", "");
	login_login(c.GetUserCtx(), "c c");
	login_create_player(c.GetUserCtx(), "c", "");
	login_login(d.GetUserCtx(), "d d");
	login_create_player(d.GetUserCtx(), "d", "");

	lobby_room_join_owner(a.GetUserCtx(), "a");
	a.Dispatch();
	lobby_room_join_owner(b.GetUserCtx(), "a");
	b.Dispatch();
	lobby_room_join_owner(c.GetUserCtx(), "a");
	c.Dispatch();
	lobby_room_join_owner(d.GetUserCtx(), "a");
	d.Dispatch();

	room_microphone_acquire(a.GetUserCtx(), "song a");
	a.Dispatch();

	room_set_ready(a.GetUserCtx(), 1);
	a.Dispatch();
	room_set_ready(b.GetUserCtx(), 1);
	b.Dispatch();
	room_set_ready(c.GetUserCtx(), 1);
	c.Dispatch();
	room_set_ready(d.GetUserCtx(), 1);
	d.Dispatch();

	room_loaded(a.GetUserCtx());
	a.Dispatch();

	for(;;) {
		a.Dispatch();
		b.Dispatch();
		c.Dispatch();
		d.Dispatch();
//		COOKTCPConnection::GetDefault()->Dispatch();
	}

	CTCPConnection::Final();
	WSACleanup();
	return 0;
}