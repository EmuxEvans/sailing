#include <stdio.h>
#include <string.h>

#include "../inc/errcode.h"
#include "../inc/os.h"

int main(int argc, char** argv)
{
	int ret;
	os_shm_t shm;
	void* ptr;
	unsigned int size;

	if(argc<2) return -1;
	if(strcmp(argv[1], "create")==0) {
		ret = os_shm_create(&shm, "system.conf", 100000);
		if(ret!=0) {
			printf("Failed to os_shm_create(), ret=%d\n", ret);
			return 0;
		}
		ptr = os_shm_map(shm, &size);
		if(ptr==NULL) {
			printf("Failed to os_shm_create(), ret=%d\n", ret);
			return 0;
		}
		os_shm_unmap(shm, ptr, size);
		os_shm_close(shm);
		printf("create done\n");
	} else if(strcmp(argv[1], "open")==0) {
		ret = os_shm_open(&shm, "system.conf");
		if(ret!=0) {
			printf("Failed to os_shm_open(), ret=%d\n", ret);
			return 0;
		}
		ptr = os_shm_map(shm, &size);
		if(ptr==NULL) {
			printf("Failed to os_shm_create(), ret=%d\n", ret);
			return 0;
		}
		os_shm_unmap(shm, ptr, size);
		os_shm_close(shm);
		printf("open done\n");
	} else {
	}

	return 0;
}

