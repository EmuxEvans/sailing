#include <stdio.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/misc.h"
#include "../inc/protocol.h"

static char buf[100*1024];

int main(int argc, char* argv[])
{
	int len;
	if(argc!=2) abort();
	len = loal_textfile(argv[1], buf, sizeof(buf));
	if(len<=0) {
		printf("error in load file %s\n", argv[1]);
		return(0);
	}

	protocol_parse(buf, NULL, NULL);

	return(0);
}
