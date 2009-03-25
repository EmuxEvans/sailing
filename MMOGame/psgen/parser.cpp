#include <string>
#include <string.h>
#include <vector>

#include "parser.h"

static PDL_ARG args[2000];
static PDL_CMD cmds[400];
static int args_cur = 0, cmds_cur = 0;

static const char* get_token_char(const char* buf, char c);
static const char* get_token_string(const char* buf, char* value, int size);
static const char* get_token_id(const char* buf, char* value, int size);
static const char* get_token_keyword(const char* buf, const char* keyword, char* value);
static const char* get_token_number(const char* buf, char* value, int size);
static const char* escape_blank(const char* buf);

static const char* parser_cmd(const char* buf);
static const char* parser_arg(const char* buf);

static int load_textfile(const char* filename, char* buf, int buflen);

bool psgen_parser(const char* text)
{
	const char* buf;

	buf = escape_blank(text);
	while(*buf!='\0') {
		buf = parser_cmd(buf);
		if(buf==NULL) {
			return false;
		}

		buf = escape_blank(buf);
	}

	return true;
}

int psgen_getcount()
{
	return cmds_cur;
}

const PDL_CMD* psgen_get(int index)
{
	return &cmds[index];
}

const char* get_token_char(const char* buf, char c)
{
	buf = escape_blank(buf);
	if(*buf!=c) return NULL;
	return buf+1;
}

const char* get_token_string(const char* buf, char* value, int size)
{
	int end;

	buf = escape_blank(buf);
	if(*buf!='"') return NULL;
	for(end=1; ;end++) {
		if(buf[end]=='\0') return NULL;
		if(buf[end]=='"') break;
		if(buf[end]=='\\') {
			if(buf[end-1]=='\0') return NULL;
			end++; continue;
		}
	}
	if(end+2>size) return NULL;

	memcpy(value, buf+1, end+1-1);
	value[end+1-2] = '\0';

	return buf+end+1;
}

const char* get_token_id(const char* buf, char* value, int size)
{
	int end;

	buf = escape_blank(buf);

	for(end=0;; end++) {
		if(buf[end]>='0' && buf[end]<='9') {
			if(end==0) return NULL;
			continue;
		}
		if(buf[end]>='a' && buf[end]<='z') continue;
		if(buf[end]>='A' && buf[end]<='Z') continue;
		if(buf[end]=='_') continue;

		break;
	}
	if(end==0) return NULL;

	if(end+1>size) return NULL;
	memcpy(value, buf, end);
	value[end] = '\0';
	return buf+end;
}

const char* get_token_keyword(const char* buf, const char* keyword, char* value)
{
	char id[100];
	buf = escape_blank(buf);
	buf = get_token_id(buf, id, sizeof(id));
	if(buf==NULL) return NULL;
	if(strcmp(id, keyword)!=0) return NULL;
	if(value) strcpy(value, keyword);
	return buf;
}

const char* get_token_number(const char* buf, char* value, int size)
{
	int end;
	const char* tbuf;
	char kk[100];

	buf = escape_blank(buf);

	for(end=0; ;end++) {
		if(buf[end]>='0' && buf[end]<='9') continue;
		if(buf[end]=='+') continue;
		if(buf[end]=='-') continue;
		if(buf[end]=='*') continue;
		if(buf[end]=='/') continue;
		if(buf[end]=='.') continue;
		tbuf = get_token_id(buf+end, kk, sizeof(kk));
		if(tbuf) {
			end = tbuf - buf - 1;
			continue;
		}
		if(end==0) return NULL;
		break;
	}

	if(end+1>size) return NULL;
	memcpy(value, buf, end);
	value[end] = '\0';

	return buf+end;
}

const char* escape_blank(const char* buf)
{
	for(;;buf++) {

		if(*buf=='/' && buf[1]=='/') {
			buf+=2;
			while(*buf!='\n' && *buf!='\0') buf++;
		}
		if(*buf=='\0') return buf;
		if(*buf<=' ') continue;

		return buf;
	}
}

const char* parser_cmd(const char* buf)
{
	char name[100];
	char desc[10000];

	buf=get_token_id(buf, name, sizeof(name));
	if(!buf) return NULL;
	buf=get_token_string(buf, desc, sizeof(desc));
	if(!buf) return NULL;
	buf=get_token_char(buf, '{');
	if(!buf) return NULL;

	strcpy(cmds[cmds_cur].name, name);
	strcpy(cmds[cmds_cur].desc, desc);
	cmds[cmds_cur].args = &args[args_cur];
	cmds[cmds_cur].args_count = 0;

	for(;;) {
		const char* tbuf;
		tbuf = parser_arg(buf);
		if(!tbuf) break;
		buf = tbuf;

		tbuf = get_token_char(buf, ',');
		if(!tbuf) break;
		buf = tbuf;
	}
	if((buf=get_token_char(buf, '}'))==NULL || (buf=get_token_char(buf, ';'))==NULL) {
		return NULL;
	}

	cmds_cur++;
	return buf;
}

const char* parser_arg(const char* buf)
{
	char type[100];
	char size[100];
	char count[100];
	char name[100];
	char desc[10000];
	const char* tbuf;

	type[0] = '\0';
	size[0] = '\0';
	strcpy(count, "1");
	name[0] = '\0';

	if(			(tbuf=get_token_id(buf, type, sizeof(type)))
			&&	(tbuf=get_token_char(tbuf, '['))
			&&	(tbuf=get_token_number(tbuf, count, sizeof(count)))
			&&	(tbuf=get_token_char(tbuf, ']'))
			&&	(tbuf=get_token_id(tbuf, name, sizeof(name))) ) {
	} else if(	(tbuf=get_token_keyword(buf, "string", type))
			&&	(tbuf=get_token_char(tbuf, '<'))
			&&	(tbuf=get_token_number(tbuf, size, sizeof(size)))
			&&	(tbuf=get_token_char(tbuf, '>'))
			&&	(tbuf=get_token_id(tbuf, name, sizeof(name))) ) {
	} else if(	(tbuf=get_token_id(buf, type, sizeof(type)))
			&&	(tbuf=get_token_id(tbuf, name, sizeof(name))) ) {
	} else {
		return NULL;
	}

	tbuf = get_token_string(tbuf, desc, sizeof(desc));
	if(!tbuf) return NULL;

	PDL_ARG arg;
	strcpy(args[args_cur].name, name);
	strcpy(args[args_cur].type, type);
	strcpy(args[args_cur].size, size);
	strcpy(args[args_cur].count, count);
	strcpy(args[args_cur].desc, desc);
	args_cur++;
	cmds[cmds_cur].args_count++;

	return tbuf;
}

int load_textfile(const char* filename, char* buf, int buflen)
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
