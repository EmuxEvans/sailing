#include <stdio.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/misc.h>
#include <skates/protocol.h>

ZION_API int loal_textfile(const char* filename, char* buf, int buflen);
ZION_API int save_textfile(const char* filename, char* buf, int buflen);

char txt[50*1024];
char buf[150*1024];

int main(int argc, char* argv[])
{
	int ret;
	PROTOCOL_TABLE* table;

	if(argc<2) {
		printf("invalid parameter\n");
		exit(0);
	}

	ret = load_textfile(argv[1], txt, sizeof(txt));
	if(ret!=ERR_NOERROR) {
		printf("invalid parameter\n");
		exit(0);
	}

	table = protocol_table_alloc(buf, sizeof(buf), 100, 1000);

	protocol_parse_pfile(txt, table);	

	protocol_table_free(table);

	return 0;
}
