#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/sock.h"
#include "../inc/rqueue.h"

#include "../inc/rlist.h"
#include "../inc/fdwatch.h"
#include "../inc/mempool.h"
#include "../inc/network.h"

static RQUEUE* queue;
static RQUEUE_CLIENT* client;

static os_thread_t s_tids[1];
static os_thread_t c_tids[1];

static int quit_flag = 0;

static unsigned int ZION_CALLBACK server_proc(void* arg);
static unsigned int ZION_CALLBACK client_proc(void* arg);

int main(int argc, char* argv[])
{
	int l;
	SOCK_ADDR s_sa, c_sa;

	if(argc!=3) {
		printf("%s [QUEUE_SERVER:EP] [QUEUE_CLIENT:EP]\n", argv[0]);
		return -1;
	}
	if(sock_str2addr(argv[1], &s_sa)==NULL || sock_str2addr(argv[2], &c_sa)==NULL) {
		printf("Invalid [QUEUE_SERVER:EP] or [QUEUE_CLIENT:EP]\n");
		return -1;
	}

	sock_init();
	fdwatch_init();
	mempool_init();
	network_init(20000);
	rqueue_init();

	queue = rqueue_create(&s_sa, 0, 0, NULL);
	if(queue==NULL) {
		printf("Failed to rqueue_create()\n");
		exit(-1);
	}
	client = rqueue_get(&c_sa);
	if(client==NULL) {
		printf("Failed to rqueue_get()\n");
		exit(-1);
	}

	for(l=0; l<sizeof(s_tids)/sizeof(s_tids[0]); l++) {
		os_thread_begin(&s_tids[l], server_proc, queue);
	}
	for(l=0; l<sizeof(c_tids)/sizeof(c_tids[0]); l++) {
		os_thread_begin(&c_tids[l], client_proc, client);
	}

	getchar();

	for(l=0; l<sizeof(s_tids)/sizeof(s_tids[0]); l++) {
		rqueue_write(rqueue_get(&s_sa), &quit_flag, sizeof(quit_flag));
	}
	quit_flag = 1;

	for(l=0; l<sizeof(s_tids)/sizeof(s_tids[0]); l++) {
		os_thread_wait(s_tids[l], NULL);
	}
	for(l=0; l<sizeof(c_tids)/sizeof(c_tids[0]); l++) {
		os_thread_wait(c_tids[l], NULL);
	}

	rqueue_final();
	network_final();
	mempool_final();
	fdwatch_final();
	sock_final();

	return 0;
}

unsigned int ZION_CALLBACK server_proc(void* arg)
{
	int ret;
	unsigned int size;
	char buf[10000];

	for(;;) {
		size = sizeof(buf);
		ret = rqueue_read((RQUEUE*)arg, buf, &size);
		if(ret!=ERR_NOERROR) {
			continue;
		}
		if(buf[0]=='\0') break;

		printf("\t\t%p %s\n", buf, buf);
	}

	printf("server_thread quit.\n");

	return 0;
}

unsigned int ZION_CALLBACK client_proc(void* arg)
{
	int ret;
	char buf[10000];
	unsigned int i=0;

	while(!quit_flag) {
		sprintf(buf, "%p ZION_CALLBACK %u", buf, i++);

		ret = rqueue_write((RQUEUE_CLIENT*)arg, buf, strlen(buf)+1);
		if(ret!=ERR_NOERROR) {
			printf("Failed to rqueue_write(), return %d\n", ret);
			continue;
		}

		os_sleep(10);
	}

	return 0;
}
