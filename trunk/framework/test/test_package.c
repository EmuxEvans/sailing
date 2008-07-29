#include <stdio.h>
#include <stdlib.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/sock.h"
#include "../inc/rlist.h"
#include "../inc/fdwatch.h"
#include "../inc/package.h"

void notify(PACKAGE_HANDLE handle, int code, const void* recvbuf, unsigned int recvbuf_len)
{
	switch (code) {
		case PACKAGE_ONCONNECT:
			printf("connect\n");
			break;
		case PACKAGE_ONDATA:
			if (!strncmp("exit", recvbuf, 4)) {
				package_disconnect(handle, 0);
				return;
			}
			package_send(handle, recvbuf, recvbuf_len);
			break;
		case PACKAGE_ONDISCONNECT:
			printf("disconnect\n");
			break;
		default:
			return;
	}
}

int main()
{
	int ret;
	SOCK_ADDR addr;

	ret = mempool_init();
	if (ret != ERR_NOERROR) {
		printf("mempool failed\n");
		return;
	}
	ret = threadpool_init(10);
	if (ret != ERR_NOERROR) {
		printf("threadpool failed\n");
		return;
	}
	sock_init();
	ret = fdwatch_init();
	if (ret != ERR_NOERROR) {
		printf("fdwatch failed\n");
		return;
	}
	ret = package_init(512);
	if (ret != ERR_NOERROR) {
		printf("package failed\n");
		return;
	}
	
	sock_str2addr("0.0.0.0:8888", &addr);
	ret = package_bind(&addr, notify);
	if (ret != ERR_NOERROR) {
		printf("bind failed\n");
		return;
	}

	getchar();
	package_unbind(&addr);

	package_final();
	fdwatch_final();
	sock_final();
	threadpool_final();
	mempool_final();
}
