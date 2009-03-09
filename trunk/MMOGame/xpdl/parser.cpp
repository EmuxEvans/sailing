#include <string>
#include <vector>

#include "parser.h"

std::vector<PDL_CMD> cmds;

static const char* get_token_char(const char* buf, char c);
static const char* get_token_string(const char* buf, char* value, int size);
static const char* get_token_id(const char* buf, char* value, int size);
static const char* get_token_keyword(const char* buf, const char* keyword, char* value);
static const char* get_token_number(const char* buf, char* value, int size);
static const char* escape_blank(const char* buf);

static const char* parser_cmd(const char* buf);
static const char* parser_arg(PDL_CMD& cmd, const char* buf);

static int load_textfile(const char* filename, char* buf, int buflen);

bool pdl_parser(const char* text)
{
	const char* buf;

	buf = escape_blank(text);
	while(*buf!='\0') {
		buf = parser_cmd(buf);
		if(buf==NULL) {
			return false;
		}

		buf = escape_blank(text);
	}

	return true;
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
	if(end+2<size) return NULL;

	memcpy(value, buf, end+1);
	value[end+1] = '\0';

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
	char mode[100];
	char code[100];
	char name[100];

	buf=get_token_id(buf, mode, sizeof(mode));
	if(!buf) return NULL;
	buf=get_token_id(buf, code, sizeof(code));
	if(!buf) return NULL;
	buf=get_token_id(buf, name, sizeof(name));
	if(!buf) return NULL;
	buf=get_token_char(buf, '(');
	if(!buf) return NULL;

	PDL_CMD cmd;
	cmd.mode = mode;
	cmd.code = code;
	cmd.name = name;

	for(;;) {
		const char* tbuf;
		tbuf = parser_arg(cmd, buf);
		if(!tbuf) break;
		buf = tbuf;

		tbuf = get_token_char(buf, ',');
		if(!tbuf) break;
		buf = tbuf;
	}
	if((buf=get_token_char(buf, ')'))==NULL || (buf=get_token_char(buf, ';'))==NULL) {
		return NULL;
	}

	cmds.push_back(cmd);

	return buf;
}

const char* parser_arg(PDL_CMD& cmd, const char* buf)
{
	char type1[100];
	char type2[100];
	bool isarray;
	char name[100];
	char range[100];
	const char* tbuf;

	if(			(tbuf=get_token_id(buf, type1, sizeof(type1)))
			&&	(tbuf=get_token_id(buf, type2, sizeof(type2)))
			&&	(tbuf=get_token_char(buf, '['))
			&&	(tbuf=get_token_char(buf, ']'))
			&&	(tbuf=get_token_id(buf, name, sizeof(name))) ) {
		strcat(type1, type2);
		isarray = true;
	} else if(	(tbuf=get_token_id(buf, type1, sizeof(type1)))
			&&	(tbuf=get_token_char(buf, '['))
			&&	(tbuf=get_token_char(buf, ']'))
			&&	(tbuf=get_token_id(buf, name, sizeof(name))) ) {
		isarray = true;
		return tbuf;
	} else if(	(tbuf=get_token_id(buf, type1, sizeof(type1)))
			&&	(tbuf=get_token_id(buf, type2, sizeof(type2)))
			&&	(tbuf=get_token_id(buf, name, sizeof(name))) ) {
		strcat(type1, type2);
		return tbuf;
	} else if(	(tbuf=get_token_id(buf, type1, sizeof(type1)))
			&&	(tbuf=get_token_id(buf, name, sizeof(name))) ) {
		isarray = false;
		return tbuf;
	} else {
		isarray = false;
		return NULL;
	}

	buf = tbuf;
	tbuf=get_token_keyword(buf, "range", NULL);
	if(tbuf) {
		tbuf = get_token_string(tbuf, range, sizeof(range));
		if(!tbuf) return NULL;
		buf = tbuf;
	} else {
		strcpy(range, "");
	}

	PDL_ARG arg;
	arg.type = type1;
	arg.isarray = isarray;
	arg.name = name;
	arg.range = range;
	cmd.args.push_back(arg);

	return buf;
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
