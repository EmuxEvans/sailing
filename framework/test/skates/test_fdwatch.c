#include <stdio.h>
#include <stdlib.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/sock.h>
#include <skates/rlist.h>
#include <skates/fdwatch.h>

static FDWATCH_HANDLE fdwatch_fd;
static SOCK_HANDLE b_so;
static FDWATCH_ITEM b_wi;

static void i_func(FDWATCH_ITEM* item, int events)
{
	char buf[100];
	int ret;

	ret = sock_read(item->fd, buf, sizeof(buf));
	if(ret<=0) {
		fdwatch_remove(fdwatch_fd, item);
		sock_close(item->fd);
		printf("discconect\n");
		return;
	}
	ret = sock_write(item->fd, buf, ret);
}

static void b_func(FDWATCH_ITEM* item, int events)
{
	FDWATCH_ITEM* i;
	SOCK_HANDLE a;
	a = sock_accept(b_so, NULL);
	if(a==SOCK_INVALID_HANDLE) return;

	i = malloc(sizeof(FDWATCH_ITEM));
	fdwatch_set(i, a, FDWATCH_READ, i_func, i);
	fdwatch_add(fdwatch_fd, i);	
}

static unsigned int ZION_CALLBACK nthread(void* ptr)
{
	fdwatch_loop(fdwatch_fd);
	return 0;
}

int main(int argc, char* argv[])
{
	SOCK_ADDR sa;
	os_thread_t th;

	sock_init();
	fdwatch_init();

	fdwatch_create(&fdwatch_fd);
	b_so = sock_bind(sock_str2addr("0.0.0.0:1980", &sa), SOCK_REUSEADDR);

	fdwatch_set(&b_wi, b_so, FDWATCH_READ, b_func, &b_wi);
	fdwatch_add(fdwatch_fd, &b_wi);
	os_thread_begin(&th, nthread, &b_wi);

	getchar();
	fdwatch_remove(fdwatch_fd, &b_wi);
	sock_unbind(b_so);

	fdwatch_break(fdwatch_fd);
	os_thread_wait(th, NULL);

	sock_unbind(b_so);
	fdwatch_final();
	sock_final();

	return 0;
}

