#include <iostream>
#include <string>
#include <vector>

#include <stdio.h>
#include <assert.h>

#include "parser.h"
#include "codegen.h"

static int load_textfile(const char* filename, char* buf, int buflen)
{
	FILE* fp;
	int len = 0;

	fp = fopen(filename, "rt");
	if(fp==NULL) return -1;

	for(;;) {
		if(fgets(buf+len, buflen-len, fp)==NULL) {
			break;
		}
		len += (int)strlen(buf+len);
	}

	fclose(fp);
	return len+1;
}

static char strbuf[100*1024];

int main(int argc, char* argv[])
{
	assert(argc==2);
	if(load_textfile(argv[1], strbuf, sizeof(strbuf))<0) {
		assert(0);
		return -1;
	}

	if(!pdl_parser(strbuf)) {
		printf("failed to parse %s\n", argv[1]);
		return -1;
	}

	if(!code_check()) {
		printf("failed to parse %s\n", argv[1]);
		return -1;
	}

	//std::ofstream s;
	//if(!code_gen(s)) {
	//	return -1;
	//}

	return 0;
}
