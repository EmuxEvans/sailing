#include <stdio.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/sock.h"
#include "../inc/mempool.h"
#include "../inc/network.h"

#include "../inc/rlist.h"
#include "../inc/fdwatch.h"

static void onaccept_proc(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname);

int main(int argc, char* argv[])
{
	SOCK_ADDR sa;

	sock_init();
	fdwatch_init();
	network_init(20000);

	network_tcp_register(sock_str2addr("0.0.0.0:1980", &sa), onaccept_proc, NULL);

	getchar();

	network_tcp_unregister(&sa);

	network_final();
	fdwatch_final();
	sock_final();

	return 0;
}

void onaccept_proc(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname)
{
}
