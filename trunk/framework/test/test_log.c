#include <stdio.h>
#include <string.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/log.h"

int main()
{
	LOG_HANDLE handle = NULL;

	handle = log_open("file://log");
	if(handle == NULL) {
		printf("log_open function error!\n");
		return -1;
	}

	log_puts(handle, LOG_DEBUG, "log_puts", 8);
	log_write(handle, LOG_DEBUG, "%s %d","malong", 6);

	log_close(handle);

	return 0;
}

