#include <stdio.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/misc.h>
#include <skates/protocol.h>

static char txt[50*1024];
static char buf[150*1024];
static char c_file[50*1024];
static char h_file[50*1024];
static char mode[100], type[100], name[100], value[2000];

static int parse_pfile(const char* text);
static int generate_cfile(const char* name, char* inc, unsigned int inc_len, char* src, unsigned int src_len);

static const char* get_token_char(const char* buf, char c);
static const char* get_token_string(const char* buf, char* value, int size);
static const char* get_token_id(const char* buf, char* value, int size);
static const char* get_token_keyword(const char* buf, const char* keyword, char* value);
static const char* get_token_number(const char* buf, char* value, int size);
static const char* escape_blank(const char* buf);

static const char* parse_include(const char* buf);
static const char* parse_const(const char* buf);
static const char* parse_node_def(const char* buf);
static const char* parse_field_def(const char* buf);
static const char* parse_array_def(const char* buf);

static int check_include_filename(const char* file);

static int def_include(const char* name);
static int def_node(const char* mode, const char* name);
static int def_const(const char* type, const char* name, const char* value);
static int def_field(const char* mode, const char* type, const char* name, const char* value);
static int def_array(const char* mode, const char* type, const char* name, const char* count);
static void def_errmsg(const char* msg);

static struct {
	char file[100];
} data_include[100];

static struct {
	int  is_const;
	char mode[100];
	char type[100];
	char name[100];
	char maxlen[100];
	char value[1000];
} data_variable[500];

static struct {
	char mode[100];
	char name[100];
	int  var_start;
	int  var_count;
} data_type[100];

static int num_inc, num_var, num_type;

int main(int argc, char* argv[])
{
	int ret;

	if(argc<2) {
		printf("invalid parameter\n");
		exit(0);
	}

	ret = load_textfile(argv[1], txt, sizeof(txt));
	if(ret<0) {
		printf("can't load file(%s)\n", argv[1]);
		exit(0);
	}

	num_inc = num_var = num_type = 0;

	ret = parse_pfile(txt);
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		exit(0);
	}
	ret = generate_cfile("aaaa", h_file, sizeof(h_file), c_file, sizeof(c_file));
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		exit(0);
	}

	save_textfile("aaaa.h", h_file, 0);
	save_textfile("aaaa.c", c_file, 0);

	return 0;
}

/*
static char* protocol_strdup(PROTOCOL_TABLE* table, const char* value);
static PROTOCOL_TYPE* protocol_get_type(PROTOCOL_TABLE* table, const char* name);
static PROTOCOL_VARIABLE* protocol_get_variable(PROTOCOL_TYPE* table, const char* name);
static PROTOCOL_TYPE* protocol_push_type(PROTOCOL_TABLE* table, const char* name);
static PROTOCOL_VARIABLE* protocol_push_variable(PROTOCOL_TABLE* table, const char* name, const char* type, const char* maxlen, const char* def_value);

static int is_base_type(const char* type);
*/

int parse_pfile(const char* buf)
{
	const char* tbuf;

	tbuf = buf;
	while(*buf!='\0') {
		buf = escape_blank(tbuf);

		tbuf = parse_include(buf);
		if(tbuf) continue;

		tbuf = parse_node_def(buf);
		if(tbuf) continue;

		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

int generate_cfile(const char* name, char* inc, unsigned int inc_len, char* src, unsigned int src_len)
{
	int type, var;

	inc[0] = src[0] = '\0';
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#ifndef __%s_include__\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#define __%s_include__\n");

	for(type=0; type<num_type; type++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "typedef struct %s {\n", data_type[type].name);

		for(var=data_type[type].var_start; var<data_type[type].var_start+data_type[type].var_count; var++) {
			if(data_variable[var].is_const) continue;
			if(data_variable[var].maxlen[0]=='\0') {
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s %s;\n", data_variable[var].type, data_variable[var].name);
			} else {
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s[%s] %s;\n", data_variable[var].type, data_variable[var].maxlen, data_variable[var].name);
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "int PROTOCOL_ARRAY_SIZE(%s);\n", data_variable[var].name);
			}
		}

		snprintf(inc+strlen(inc), inc_len-strlen(inc), "} %s;\n", data_type[type].name);
	}

	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "// YIYI & ERIC 2004-2008.\n");

	for(type=0; type<num_type; type++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "extern PROTOCOL_TYPE PROTOCOL_NAME(%s);\n", data_type[type].name);
	}

	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#endif\n");

	return ERR_NOERROR;
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
	buf = get_token_id(buf, id, sizeof(id));
	if(buf==NULL) return NULL;
	if(strcmp(id, keyword)!=0) return NULL;
	if(keyword) strcpy(value, keyword);
	return buf;
}

const char* get_token_number(const char* buf, char* value, int size)
{
	int end;

	buf = escape_blank(buf);

	for(end=0; ;end++) {
		if(buf[end]>='0' && buf[end]<='9') continue;
		if(buf[end]=='+') continue;
		if(buf[end]=='-') continue;
		if(buf[end]=='.') continue;
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
		if(*buf=='\0') return buf;
		if(*buf<=' ') continue;
		return buf;
	}
}

const char* parse_include(const char* buf)
{
	const char* tbuf;

	buf = get_token_keyword(buf, "include", name);
	if(buf==NULL) return NULL;
	buf = escape_blank(buf);

	tbuf = buf;
	for(;;) {
		if(*tbuf=='\0') return NULL;
		if(*tbuf<=' ') return NULL;
		if(*tbuf==';') break;
	}
	if(tbuf==buf) return NULL;
	if(tbuf-buf>=sizeof(name)) return NULL;
	memcpy(name, buf, tbuf-buf);
	name[tbuf-buf] = '\0';

	buf = get_token_char(buf, ';');
	if(buf==NULL) return NULL;

	if(!def_include(name)) return NULL;

	return buf;
}

const char* parse_node_def(const char* buf)
{
	const char* tbuf;

	tbuf = buf;
	tbuf = get_token_keyword(buf, "extern", mode);
	if(tbuf==NULL) {
		tbuf = get_token_keyword(buf, "internal", mode);
		if(tbuf==NULL) return NULL;
	}
	buf = tbuf;

	buf = get_token_id(buf, name, sizeof(name));
	if(buf==NULL) return NULL;
	buf = get_token_char(buf, '{');
	if(buf==NULL) return NULL;

	// add node
	if(!def_node(mode, name)) return NULL;

	// node body
	tbuf = buf;
	for(;;) {
		buf = tbuf;

		tbuf = get_token_char(buf, '}');
		if(tbuf!=NULL) break;

		tbuf = parse_const(buf);
		if(tbuf!=NULL) continue;

		tbuf = parse_field_def(buf);
		if(tbuf!=NULL) continue;

		tbuf = parse_array_def(buf);
		if(tbuf!=NULL) continue;

		//
		return NULL;
	}

	return tbuf;
}

const char* parse_const(const char* buf)
{
	const char* tbuf;

	buf = get_token_keyword(buf, "const", NULL);
	if(buf==NULL) return NULL;

	buf = get_token_id(buf, type, sizeof(type));
	if(buf==NULL) return NULL;
	buf = get_token_id(buf, name, sizeof(name));
	if(buf==NULL) return NULL;
	tbuf = get_token_string(buf, value, sizeof(value));
	if(tbuf==NULL) {
		tbuf = get_token_id(buf, value, sizeof(value));
		if(tbuf==NULL) {
			tbuf = get_token_number(buf, value, sizeof(value));
			if(tbuf==NULL) {
				return NULL;
			}
		}
	}

	if(!def_const(type, name, value)) return NULL;

	return tbuf;
}

const char* parse_field_def(const char* buf)
{
	const char* tbuf;

	buf = get_token_id(buf, type, sizeof(type));
	if(buf==NULL) return NULL;
	buf = get_token_id(buf, name, sizeof(name));
	if(buf==NULL) return NULL;

	tbuf = get_token_char(buf, '=');
	if(tbuf!=NULL) {
		buf = tbuf;
		tbuf = get_token_string(buf, value, sizeof(value));
		if(tbuf==NULL) {
			tbuf = get_token_number(buf, value, sizeof(value));
			if(tbuf==NULL) {
				tbuf = get_token_id(buf, value, sizeof(value));
				if(tbuf==NULL) {
					return NULL;
				}
			}
		}
		buf = tbuf;
	} else {
		value[0] = '\0';
	}

	buf = get_token_char(buf, ';');
	if(buf==NULL) return NULL;

	if(!def_field(mode, type, name, value)) return NULL;

	return buf;
}

const char* parse_array_def(const char* buf)
{
	const char* tbuf;

	tbuf = buf;
	tbuf = get_token_keyword(buf, "option", mode);
	if(tbuf==NULL) {
		tbuf = get_token_keyword(buf, "acquire", mode);
		if(tbuf==NULL) return NULL;
	}
	buf = tbuf;

	buf = get_token_id(buf, type, sizeof(type));
	if(buf==NULL) return NULL;
	buf = get_token_id(buf, name, sizeof(name));
	if(buf==NULL) return NULL;

	buf = get_token_char(buf, '[');
	if(buf==NULL) return NULL;
	tbuf = get_token_number(buf, value, sizeof(value));
	if(tbuf==NULL) {
		tbuf = get_token_id(buf, value, sizeof(value));
		if(tbuf==NULL) return NULL;
	}
	buf = tbuf;
	buf = get_token_char(buf, ']');
	if(buf==NULL) return NULL;

	buf = get_token_char(buf, ';');
	if(buf==NULL) return NULL;

	if(!def_array(mode, type, name, value)) return NULL;

	return buf;
}

/*
int protocol_generate_cfile(const PROTOCOL_TABLE* table, const char* name, char* inc, unsigned int inc_len, char* src, unsigned int src_len)
{
	int t, v;

	inc[0] = 0;
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#ifndef __%s_include__\n", name);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#define __%s_include__\n", name);
	for(t=0; t<table->type_count; t++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

		snprintf(inc+strlen(inc), inc_len-strlen(inc), "typedef struct %s {\n", table->type_list[t].name);
		for(v=0; v<table->type_list[t].var_count; v++) {
			if(table->type_list[t].var_list[v].maxlen) {
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "	%s[%s] %s; //", table->type_list[t].var_list[v].type, table->type_list[t].var_list[v].maxlen, table->type_list[t].var_list[v].name);
			} else {
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "	%s %s; // default=%s\n", table->type_list[t].var_list[v].type, table->type_list[t].var_list[v].name, table->type_list[t].var_list[v].def_value?table->type_list[t].var_list[v].def_value:"");
			}
		}
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "} %s;\n", table->type_list[t].name);
	}
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "extern PROTOCOL_TABLE __protocol_table_%s;\n", name);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#endif\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

	src[0] = 0;
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/protocol_def.h>\n", name);
	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include \"%s.h\"\n", name);
	for(t=0; t<table->type_count; t++) {
		for(v=0; v<table->type_list[t].var_count; v++) {
		}
	}
	snprintf(src+strlen(src), src_len-strlen(src), "PROTOCOL_TABLE __protocol_table_%s = {\n", name);
	snprintf(src+strlen(src), src_len-strlen(src), "0, NULL, 0, 0,\n");
	snprintf(src+strlen(src), src_len-strlen(src), "NULL, 0, 0,\n");
	snprintf(src+strlen(src), src_len-strlen(src), "NULL, 0, 0,\n");
	snprintf(src+strlen(src), src_len-strlen(src), "};\n");
	snprintf(src+strlen(src), src_len-strlen(src), "\n");

	return ERR_NOERROR;
}

char* protocol_strdup(PROTOCOL_TABLE* table, const char* value)
{
	char* ret;
	if(table->need_free) {
		ret = malloc(strlen(value)+1);
		strcpy(ret, value);
	} else {
		if(table->buf_count+(int)strlen(value)+1 > table->buf_max) {
			return NULL;
		}
		ret = table->buf + table->buf_count;
		table->buf_count += strlen(value) + 1;
		strcpy(ret, value);
	}
	return ret;
}

PROTOCOL_TYPE* protocol_get_type(PROTOCOL_TABLE* table, const char* name)
{
	int i;
	for(i=0; i<table->type_count; i++) {
		if(strcmp(table->type_list[i].name, name)==0) {
			return &table->type_list[i];
		}
	}
	return NULL;
}

PROTOCOL_VARIABLE* protocol_get_variable(PROTOCOL_TYPE* table, const char* name)
{
	int i;
	for(i=0; i<table->var_count; i++) {
		if(strcmp(table->var_list[i].name, name)==0) {
			return &table->var_list[i];
		}
	}
	return NULL;
}

PROTOCOL_TYPE* protocol_push_type(PROTOCOL_TABLE* table, const char* name)
{
	if(table->type_count==table->type_max)
		return NULL;

	if(name==NULL)
		return NULL;

	table->type_list[table->type_count].name = protocol_strdup(table, name);
	if(!table->type_list[table->type_count].name)
		return NULL;

	table->type_list[table->type_count].var_list = &table->var_list[table->var_count];
	table->type_list[table->type_count].var_count = 0;

	return &table->type_list[table->type_count++];
}

PROTOCOL_VARIABLE* protocol_push_variable(PROTOCOL_TABLE* table, const char* name, const char* type, const char* maxlen, const char* def_value)
{
	if(table->var_count==table->var_max)
		return NULL;

	if(name==NULL)
		return NULL;

	table->var_list[table->var_count].name = protocol_strdup(table, name);
	if(!table->var_list[table->var_count].name)
		return NULL;

	table->var_list[table->var_count].type = protocol_strdup(table, type);
	if(!table->var_list[table->var_count].type)
		return NULL;

	if(maxlen) {
		table->var_list[table->var_count].maxlen = protocol_strdup(table, maxlen);
		if(!table->var_list[table->var_count].maxlen)
			return NULL;
	} else {
		table->var_list[table->var_count].maxlen = NULL;
	}

	if(def_value) {
		table->var_list[table->var_count].def_value = protocol_strdup(table, def_value);
		if(!table->var_list[table->var_count].def_value)
			return NULL;
	} else {
		table->var_list[table->var_count].def_value = NULL;
	}

	table->type_list[table->type_count-1].var_count++;
	return &table->var_list[table->var_count++];
}

int is_base_type(const char* type)
{
	int i;
	static char* base_type[] = {
		"os_char",
		"os_short",
		"os_int",
		"os_long",
		"os_uchar",
		"os_word",
		"os_dword",
		"os_qword",
		"os_float",
	};
	for(i=0; i<sizeof(base_type)/sizeof(base_type[0]); i++) {
		if(strcmp(base_type[i], type)==0)
			return 1;
	}
	return 0;
}
*/

int check_include_filename(const char* file)
{
	char fk[] = ".proto";
	if(strlen(file)<=strlen(fk)) return 0;
	if(memcmp(file+strlen(file)-strlen(fk), fk, strlen(fk))!=0) return 0;
	return 1;
}

int def_include(const char* name)
{
	if(!check_include_filename(name)) {
		printf("invalid include filename(%s)\n", name);
		return 0;
	}
	if(num_inc>=sizeof(data_include)/sizeof(data_include[0])) {
		printf("so many include filename(%s)\n", name);
		return 0;
	}
	strcpy(data_include[num_inc++].file, name);
	return 1;
}

int def_node(const char* mode, const char* name)
{
	if(num_type>=sizeof(data_type)/sizeof(data_type[0])) {
		printf("so many type(%s)\n", name);
		return 0;
	}
	strcpy(data_type[num_type].mode, mode);
	strcpy(data_type[num_type].name, name);
	data_type[num_type].var_start = num_var;
	data_type[num_type].var_count = 0;
	num_type++;
	return 1;
}

int def_const(const char* type, const char* name, const char* value)
{
	if(num_var>=sizeof(data_variable)/sizeof(data_variable[0])) {
		printf("so many variable(%s:%s)\n", data_type[num_type-1].name, name);
		return 0;
	}
	data_variable[num_var].is_const = 1;
	strcpy(data_variable[num_var].mode, "");
	strcpy(data_variable[num_var].type, type);
	strcpy(data_variable[num_var].name, name);
	strcpy(data_variable[num_var].maxlen, "");
	strcpy(data_variable[num_var].value, value);
	data_type[num_type-1].var_count++;
	num_var++;
	return 1;
}

int def_field(const char* mode, const char* type, const char* name, const char* value)
{
	if(num_var>=sizeof(data_variable)/sizeof(data_variable[0])) {
		printf("so many variable(%s:%s)\n", data_type[num_type-1].name, name);
		return 0;
	}
	data_variable[num_var].is_const = 0;
	strcpy(data_variable[num_var].mode, mode);
	strcpy(data_variable[num_var].type, type);
	strcpy(data_variable[num_var].name, name);
	strcpy(data_variable[num_var].maxlen, "");
	if(value) {
		strcpy(data_variable[num_var].value, value);
	} else {
		strcpy(data_variable[num_var].value, "");
	}
	data_type[num_type-1].var_count++;
	num_var++;
	return 1;
}

int def_array(const char* mode, const char* type, const char* name, const char* count)
{
	if(num_var>=sizeof(data_variable)/sizeof(data_variable[0])) {
		printf("so many variable(%s:%s)\n", data_type[num_type-1].name, name);
		return 0;
	}
	data_variable[num_var].is_const = 0;
	strcpy(data_variable[num_var].mode, mode);
	strcpy(data_variable[num_var].type, type);
	strcpy(data_variable[num_var].name, name);
	strcpy(data_variable[num_var].maxlen, count);
	strcpy(data_variable[num_var].value, "");
	data_type[num_type-1].var_count++;
	num_var++;
	return 1;
}

void def_errmsg(const char* msg)
{
}
