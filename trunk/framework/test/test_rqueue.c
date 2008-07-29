#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/sock.h"
#include "../inc/rlist.h"
#include "../inc/fdwatch.h"
#include "../inc/mempool.h"
#include "../inc/rqueue.h"

static void run_server();
static void run_client();

int main(int argc, char* argv[])
{
	if(argc!=2) return -1;

	sock_init();
	fdwatch_init();
	mempool_init();
	rqueue_init();

	if(*argv[1]=='S') {
		run_server();
	} else {
		run_client();
	}
	
	rqueue_final();
	mempool_final();
	fdwatch_final();
	sock_final();

	return 0;
}

static void run_server()
{
	RQUEUE* queue;
	RQUEUE_BUFFER* buffer;
	SOCK_ADDR sa;

	sock_str2addr("0.0.0.0:1980", &sa);
	queue = rqueue_create(&sa, NULL);

	for(;;) {
		rqueue_process(queue, 0);

		for(;;) {
			buffer = rqueue_read(queue);
			if(buffer==NULL) break;
			printf("%s\n", buffer->data);
			rqueue_free(buffer);
		}
	}

	rqueue_destroy(queue);
}

static void run_client()
{
	RQUEUE_CLIENT* client;
	SOCK_ADDR sa;
	char msgstr[100];
	int ret, i;

	sock_str2addr("127.0.0.1:1980", &sa);
	client = rqueue_get(&sa);

	for(i=0; i<10000; i++) {
		getchar();
		sprintf(msgstr, "%d %s", i, "welcome to china");
		ret = rqueue_write(client, rqueue_alloc(msgstr, strlen(msgstr)+1));
		printf("rqueue_write() = %d\n", ret);
	}

}

