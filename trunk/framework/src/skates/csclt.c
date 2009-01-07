#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <skates/skates.h>

static void print_usage();

int main(int argc, char* argv[])
{
	SOCK_ADDR sa;
	int ret;
	SOCK_HANDLE s;

	if(argc!=3) {
		print_usage();
	}

	sock_init();

	if(!sock_str2addr(argv[1], &sa)) {
		print_usage();
	}

	s = sock_connect(&sa, 0);
	if(s==SOCK_INVALID_HANDLE) {
		printf("failed to connect %s\n", argv[1]);
		return -1;
	}

	ret = sock_writeline(s, argv[2]);
	if(ret!=0) {
		printf("failed to write socket\n");
		return -1;
	}

	ret = ERR_UNKNOWN;
	for(;;) {
		char line[10*1024];
		char num[100], *pnum;
		if(sock_readline(s, line, sizeof(line))!=0) {
			printf("failed to read socket\n");
			return -1;
		}
		if(strlen(line)==0) break;
		pnum = strchr(line, ' ');
		if(!pnum) {
			printf("invalid format\n");
			return -1;
		}
		memcpy(num, line, pnum-line);
		num[pnum-line] = '\0';
		ret = atoi(num);
		printf("%s\n", pnum+1);
	}

	sock_final();
	return ret;
}

void print_usage()
{
	exit(-1);
}

