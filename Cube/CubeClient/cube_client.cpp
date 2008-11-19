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

	COOKTCPConnection::GetDefault()->Connect("127.0.0.1:2008");

	login_login(OOKTCP_GET_DEFAULT_CTX(), "a a");
	login_create_player(OOKTCP_GET_DEFAULT_CTX(), "a", "");

	for(;;) {
		COOKTCPConnection::GetDefault()->Dispatch();
	}

	CTCPConnection::Final();
	WSACleanup();
	return 0;
}
