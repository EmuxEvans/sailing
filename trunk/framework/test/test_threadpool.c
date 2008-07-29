#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/threadpool.h"

static void workfunction(void *i);

int main(int argc, char** argv)
{
	int ret = ERR_NOERROR;
	int i=0;

	ret = threadpool_init(10);
	if(ret==ERR_UNKNOWN) {
		printf("threadpool_init function error\n");
	}

	for(i=0;i<=1000; i++) {
		ret = threadpool_queueitem(workfunction, &i);
		if(ret == ERR_FULL) {
			printf("queue is full!\n");
		}
		os_sleep(10);
	}

	threadpool_final();
	return 1;
}

void workfunction(void* i)
{
	printf("%d %p begin\n", threadpool_getindex(), i);
	os_sleep(100);
	printf("%d %p end\n", threadpool_getindex(), i);
}

