#include <time.h>
#include <stdio.h>
#include <string.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/misc.h>
#include <skates/protocol.h>

#include "prot_parser.h"

static char mode[100], type[100], name[100], prelen[100], value[2000];
int is_break;

DATA_INCLUDE data_include[100];
DATA_CONST data_const[1000];
DATA_VARIABLE data_variable[5000];
DATA_TYPE data_type[1000];
int num_inc = 0, num_const = 0, num_var = 0, num_type = 0;

DATA_PARAMETER data_parameter[100];
DATA_FUNCTION data_function[100];
DATA_CLASS data_class[100];
int num_parm = 0, num_func = 0, num_class = 0;
int current_class = -1;

static int check_file();

static const char* parse_include(const char* buf);
static const char* parse_const(const char* buf);
static const char* parse_node_def(const char* buf);
static const char* parse_field_def(const char* buf);
static const char* parse_array_def(const char* buf);
static const char* parse_parameter(const char* buf);
static const char* parse_function(const char* buf);
static const char* parse_class(const char* buf);

static void def_include(const char* name);
static void def_node(const char* mode, const char* name);
static void def_const(const char* type, const char* name, const char* value);
static void def_field(const char* mode, const char* type, const char* prefix, const char* name, const char* value);
static void def_array(const char* mode, const char* type, const char* prefix, const char* name, const char* count);
static void def_function(const char* return_type, const char* name);
static void def_parameter(const char* type, const char* name);
static void def_class_begin(const char* name, const char* root);
static void def_class_end(const char* name);

PARSER_STACK p_stack[10];
int p_stack_depth = 0;

int parse_file(const char* file)
{
	int i, ret;
	char path[100];
	char *a, *b;

	os_getcwd(p_stack[p_stack_depth].o_path, sizeof(p_stack[p_stack_depth].o_path));
	a = strrchr(file, '\\');
	b = strrchr(file, '/');
	if(a==NULL && b==NULL) {
		strcpy(path, p_stack[p_stack_depth].o_path);
		strcpy(p_stack[p_stack_depth].file, file);
	} else {
		if(a<b) a = b;
		strcpy(path, file);
		path[a-file] = '\0';
		if(os_chdir(path)!=0) {
			return ERR_UNKNOWN;
		}
		os_getcwd(path, sizeof(path));
		strcpy(p_stack[p_stack_depth].file, a+1);
	}
	strcpy(p_stack[p_stack_depth].path, path);
	for(i=0; i<p_stack_depth; i++) {
		if(strcmp(p_stack[p_stack_depth].path, p_stack[i].path)!=0) continue;
		if(strcmp(p_stack[p_stack_depth].file, p_stack[i].file)!=0) continue;
		os_chdir(p_stack[p_stack_depth].o_path);
		return ERR_UNKNOWN;
	}
	p_stack_depth++;

	ret = load_textfile(p_stack[p_stack_depth-1].file, p_stack[p_stack_depth-1].txt, sizeof(p_stack[p_stack_depth-1].txt));
	if(ret<0) {
		printf("can't load file(%s)\n", p_stack[p_stack_depth-1].file);
		ret = ERR_UNKNOWN;
	} else {
		ret = parse_pfile(p_stack[p_stack_depth-1].txt);
		if(ret!=ERR_NOERROR) {
			printf("error: parse!\n");
		} else {
			// done
		}
	}

	os_chdir(p_stack[p_stack_depth-1].o_path);
	p_stack_depth--;

	return ERR_NOERROR;
}

int parse_pfile(const char* buf)
{
	const char* tbuf;
	is_break = 0;

	tbuf = buf;
	for(;;) {
		if(is_break) {
			return ERR_UNKNOWN;
		}

		buf = escape_blank(tbuf);
		if(*buf=='\0') break;

		tbuf = parse_include(buf);
		if(tbuf) continue;

		tbuf = parse_const(buf);
		if(tbuf!=NULL) continue;

		tbuf = parse_node_def(buf);
		if(tbuf) continue;

		tbuf = parse_class(buf);
		if(tbuf) continue;

		return ERR_UNKNOWN;
	}

	if(is_break || !check_file()) {
		return ERR_UNKNOWN;
	}

	return ERR_NOERROR;
}

int check_file()
{
	int c, f, p;

	for(c=0; c<num_class; c++) {
		if(!data_class[c].is_root) continue;
		for(f=data_class[c].function_start; f<data_class[c].function_start+data_class[c].function_count; f++) {
			if(strcmp(data_function[f].return_type, "void")!=0) {
				if(!check_vailid_otype(data_function[f].return_type)) {
					printf("invalid return type %s::%s %s\n", data_class[c].name, data_function[f].name, data_function[f].return_type);
					return 0;
				}
			}
			for(p=data_function[f].parameter_start; p<data_function[f].parameter_start+data_function[f].parameter_count; p++) {
				if(!check_vailid_otype(data_parameter[p].type)) {
					printf("invalid parameter type %s::%s %s %s\n", data_class[c].name, data_function[f].name, data_parameter[p].name, data_parameter[p].type);
					return 0;
				}
			}
		}
	}

	return 1;
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
			end = tbuf - buf;
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
		tbuf++;
	}
	if(tbuf==buf) return NULL;
	if(tbuf-buf>=sizeof(name)) return NULL;
	memcpy(name, buf, tbuf-buf);
	name[tbuf-buf] = '\0';

	buf = get_token_char(tbuf, ';');
	if(buf==NULL) return NULL;

	def_include(name);

	return buf;
}

const char* parse_node_def(const char* buf)
{
	const char* tbuf;

	tbuf = buf;
	tbuf = get_token_keyword(buf, "struct", mode);
	if(tbuf==NULL) return NULL;
	buf = tbuf;

	buf = get_token_id(buf, name, sizeof(name));
	if(buf==NULL) return NULL;
	buf = get_token_char(buf, '{');
	if(buf==NULL) return NULL;

	// add node
	def_node(mode, name);

	// node body
	tbuf = buf;
	for(;;) {
		buf = tbuf;

		tbuf = get_token_char(buf, '}');
		if(tbuf!=NULL) break;

		tbuf = parse_field_def(buf);
		if(tbuf!=NULL) continue;

		tbuf = parse_array_def(buf);
		if(tbuf!=NULL) continue;

		//
		return NULL;
	}

	buf = get_token_char(tbuf, ';');
	if(buf==NULL) return NULL;

	return buf;
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
	buf = get_token_char(buf, '=');
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
	tbuf = get_token_char(tbuf, ';');
	if(tbuf==NULL) return NULL;

	def_const(type, name, value);

	return tbuf;
}

const char* parse_field_def(const char* buf)
{
	const char* tbuf;
	char slen[100];

	tbuf = get_token_keyword(buf, "option", mode);
	if(tbuf==NULL) {
		tbuf = get_token_keyword(buf, "acquire", mode);
		if(tbuf==NULL) {
			strcpy(mode, "acquire");
		}
	}
	if(tbuf)
		buf = tbuf;

	buf = get_token_id(buf, type, sizeof(type));
	if(buf==NULL) return NULL;
	tbuf = get_token_char(buf, '<');
	if(tbuf) {
		const char* end;
		end = strchr(tbuf, '>');
		if(!end) return NULL;
		if(end-tbuf>=sizeof(slen)) return NULL;
		memcpy(slen, tbuf, end-tbuf);
		slen[end-tbuf] = '\0';
		buf = end + 1;
	} else {
		slen[0] = '\0';
	}
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

	def_field(mode, type, slen, name, value);

	return buf;
}

const char* parse_array_def(const char* buf)
{
	const char* tbuf;
	char slen[100];

	tbuf = get_token_keyword(buf, "option", mode);
	if(tbuf==NULL) {
		tbuf = get_token_keyword(buf, "acquire", mode);
		if(tbuf==NULL) {
			strcpy(mode, "acquire");
		}
	}
	if(tbuf)
		buf = tbuf;

	buf = get_token_id(buf, type, sizeof(type));
	if(buf==NULL) return NULL;
	tbuf = get_token_char(buf, '<');
	if(tbuf) {
		const char* end;
		end = strchr(tbuf, '>');
		if(!end) return NULL;
		if(end-tbuf>=sizeof(slen)) return NULL;
		memcpy(slen, tbuf, end-tbuf);
		slen[end-tbuf] = '\0';
		buf = end + 1;
	} else {
		slen[0] = '\0';
	}
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

	def_array(mode, type, slen, name, value);

	return buf;
}

const char* parse_parameter(const char* buf)
{
	const char* tbuf;

	buf = get_token_id(buf, type, sizeof(type));
	if(buf==NULL)
		return NULL;

	tbuf = get_token_char(buf, '<');
	if(tbuf) {
		tbuf = get_token_number(tbuf, prelen, sizeof(prelen));
		if(!tbuf) {
			return NULL;
		}
		tbuf = get_token_char(tbuf, '>');
		if(!tbuf) {
			return NULL;
		}
		buf = tbuf;
	}


	buf = get_token_id(buf, name, sizeof(name));
	if(buf==NULL)
		return NULL;

	def_parameter(type, name);

	return buf;
}

const char* parse_function(const char* buf)
{
	const char* tbuf;

	buf = get_token_id(buf, mode, sizeof(mode));
	if(buf==NULL)
		return NULL;

	buf = get_token_id(buf, name, sizeof(name));
	if(buf==NULL) return NULL;

	buf = get_token_char(buf, '(');
	if(buf==NULL) return NULL;

	def_function(mode, name);

	tbuf = buf;
	for(;;) {
		buf = tbuf;

		tbuf = parse_parameter(buf);
		if(tbuf) {
			// call
			buf = tbuf;
			tbuf = get_token_char(buf, ',');
			if(tbuf) continue;
		}

		buf = get_token_char(buf, ')');
		if(buf) break;

		return NULL;
	}

	buf = get_token_char(buf, ';');
	if(buf==NULL) return NULL;

	return buf;
}

const char* parse_class(const char* buf)
{
	const char* tbuf;

	buf = get_token_keyword(buf, "interface", mode);
	if(buf==NULL)
		return NULL;

	buf = get_token_id(buf, name, sizeof(name));
	if(buf==NULL) return NULL;
	tbuf = buf;
	buf = get_token_char(buf, '{');
	if(buf==NULL) {
		buf = get_token_char(tbuf, ':');
		if(buf==NULL) return NULL;
		buf = get_token_keyword(buf, "interface", value);
		if(buf==NULL) return NULL;
		buf = get_token_char(buf, '{');
		if(buf==NULL) return NULL;
	} else {
		value[0] = '\0';
	}

	def_class_begin(name, value);

	tbuf = buf;
	for(;;) {
		buf = tbuf;

		tbuf = get_token_char(buf, '}');
		if(tbuf!=NULL) break;

		tbuf = parse_function(buf);
		if(tbuf!=NULL) continue;

		//
		return NULL;
	}

	buf = get_token_char(tbuf, ';');
	if(buf==NULL) return NULL;

	def_class_end(name);

	return buf;
}

static struct {
	const char* name;
	int type;
	const char* string;
} base_type[] = {
	{"os_char",		PROTOCOL_TYPE_CHAR,		"PROTOCOL_TYPE_CHAR"},
	{"os_short",	PROTOCOL_TYPE_SHORT,	"PROTOCOL_TYPE_SHORT"},
	{"os_int",		PROTOCOL_TYPE_INT,		"PROTOCOL_TYPE_INT"},
	{"os_long",		PROTOCOL_TYPE_LONG,		"PROTOCOL_TYPE_LONG"},
	{"os_byte",		PROTOCOL_TYPE_BYTE,		"PROTOCOL_TYPE_BYTE"},
	{"os_word",		PROTOCOL_TYPE_WORD,		"PROTOCOL_TYPE_WORD"},
	{"os_dword",	PROTOCOL_TYPE_DWORD,	"PROTOCOL_TYPE_DWORD"},
	{"os_qword",	PROTOCOL_TYPE_QWORD,	"PROTOCOL_TYPE_QWORD"},
	{"os_float",	PROTOCOL_TYPE_FLOAT,	"PROTOCOL_TYPE_FLOAT"},
	{"string",		PROTOCOL_TYPE_STRING,	"PROTOCOL_TYPE_STRING"},
};

int get_basetype(const char* type)
{
	int i;
	for(i=0; i<sizeof(base_type)/sizeof(base_type[0]); i++) {
		if(strcmp(base_type[i].name, type)==0)
			return base_type[i].type;
	}
	return 0;
}

const char* get_type_const(const char* type)
{
	int i;
	for(i=0; i<sizeof(base_type)/sizeof(base_type[0]); i++) {
		if(strcmp(base_type[i].name, type)==0)
			return base_type[i].string;
	}
	return "PROTOCOL_TYPE_STRUCT";
}

const char* get_ctype(const char* type)
{
	static char ctype[100];
	if(strcmp(type, "void")==0) {
		snprintf(ctype, sizeof(ctype), "void");
	} else if(get_basetype(type)==PROTOCOL_TYPE_STRING) {
		snprintf(ctype, sizeof(ctype), "char*");
	} else if(get_basetype(type)==0) {
		snprintf(ctype, sizeof(ctype), "%s*", type);
	} else {
		snprintf(ctype, sizeof(ctype), "%s", type);
	}
	return ctype;
}

int check_include_filename(const char* file)
{
	char fk[] = ".proto";
	if(strlen(file)<=strlen(fk)) return 0;
	if(memcmp(file+strlen(file)-strlen(fk), fk, strlen(fk))!=0) return 0;
	return 1;
}

void make_define_filename(const char* file, char* out)
{
	int i;
	make_include_filename(file, out);
	for(i=0; i<(int)strlen(out); i++) {
		if(out[i]=='.') out[i] = '_';
		if(out[i]>='a' && out[i]<='z') {
			out[i] = out[i] - 'a' + 'A';
		}
	}
}

int check_vailid_dtype(const char* type)
{
	int i;
	if(get_basetype(type)!=0) return 1;
	for(i=0; i<num_type; i++) {
		if(strcmp(data_type[i].name, type)==0) break;
	}
	if(i<num_type) return 1;
	return 0;
}

int check_vailid_otype(const char* type)
{
	int i;
	if(check_vailid_dtype(type)) return 1;
	for(i=0; i<num_class; i++) {
		if(strcmp(data_class[i].name, type)==0) break;
	}
	if(i<num_class) return 1;
	return 0;
}

void make_include_filename(const char* file, char* out)
{
	char *left, *right;
	left = strrchr(file, '\\');
	right = strrchr(file, '/');
	if(!left && !right) {
		strcpy(out, file);
	} else {
		if(right>left) left = right;
		strcpy(out, left+1);
	}
}

void def_include(const char* name)
{
	int ret;

	if(!check_include_filename(name)) {
		printf("invalid include filename(%s)\n", name);
		is_break = 1;
		return;
	}
	if(p_stack_depth==1) {
		if(num_inc>=sizeof(data_include)/sizeof(data_include[0])) {
			printf("so many include filename(%s)\n", name);
			is_break = 1;
			return;
		}
		strcpy(data_include[num_inc++].file, name);
	}

	ret = parse_file(name);
	if(ret!=ERR_NOERROR) {
		is_break = 1;
	}

	return;
}

void def_node(const char* mode, const char* name)
{
	if(num_type>=sizeof(data_type)/sizeof(data_type[0])) {
		printf("so many type(%s)\n", name);
		is_break = 1;
		return;
	}
	if(check_vailid_dtype(name)) {
		printf("so many type(%s)\n", name);
		is_break = 1;
		return;
	}
	data_type[num_type].is_root = (p_stack_depth==1);
	strcpy(data_type[num_type].mode, mode);
	strcpy(data_type[num_type].name, name);
	data_type[num_type].var_start = num_var;
	data_type[num_type].var_count = 0;
	num_type++;
	return;
}

void def_const(const char* type, const char* name, const char* value)
{
	if(num_const>=sizeof(data_const)/sizeof(data_const[0])) {
		printf("so many const(%s)\n", name);
		is_break = 1;
		return;
	}
	if(p_stack_depth==1) {
		if(get_basetype(type)==0) {
			printf("invalid const type");
			is_break = 1;
			return;
		}
		strcpy(data_const[num_const].type, type);
		strcpy(data_const[num_const].name, name);
		strcpy(data_const[num_const].value, value);
		num_const++;
	}
	return;
}

void def_field(const char* mode, const char* type, const char* prefix, const char* name, const char* value)
{
	if(num_var>=sizeof(data_variable)/sizeof(data_variable[0])) {
		printf("so many variable(%s:%s)\n", data_type[num_type-1].name, name);
		is_break = 1;
		return;
	}

	if(!check_vailid_dtype(type)) {
		printf("invalid data type(%s::%s %s)\n", data_type[num_type-1].name, name, type);
		is_break = 1;
		return;
	}
	strcpy(data_variable[num_var].mode, mode);
	strcpy(data_variable[num_var].type, type);
	strcpy(data_variable[num_var].prefix, prefix);
	strcpy(data_variable[num_var].name, name);
	strcpy(data_variable[num_var].maxlen, "");
	if(value) {
		strcpy(data_variable[num_var].value, value);
	} else {
		strcpy(data_variable[num_var].value, "");
	}
	data_type[num_type-1].var_count++;
	num_var++;
	return;
}

void def_array(const char* mode, const char* type, const char* prefix, const char* name, const char* count)
{
	if(num_var>=sizeof(data_variable)/sizeof(data_variable[0])) {
		printf("so many variable(%s:%s)\n", data_type[num_type-1].name, name);
		is_break = 1;
		return;
	}
	if(!check_vailid_dtype(type)) {
		printf("invalid data type(%s:%s)\n", data_type[num_type-1].name, name);
		is_break = 1;
		return;
	}
	strcpy(data_variable[num_var].mode, mode);
	strcpy(data_variable[num_var].type, type);
	strcpy(data_variable[num_var].prefix, prefix);
	strcpy(data_variable[num_var].name, name);
	strcpy(data_variable[num_var].maxlen, count);
	strcpy(data_variable[num_var].value, "");
	data_type[num_type-1].var_count++;
	num_var++;
	return;
}

void def_function(const char* return_type, const char* name)
{
	strcpy(data_function[num_func].return_type, return_type);
	strcpy(data_function[num_func].name, name);
	data_function[num_func].parameter_start = num_parm;
	data_function[num_func].parameter_count = 0;
	data_function[num_func].class_index = current_class;
	if(current_class>=0) {
		data_class[current_class].function_count++;
	}
	num_func++;
}

void def_parameter(const char* type, const char* name)
{
	data_function[num_func-1].parameter_count++;
	strcpy(data_parameter[num_parm].type, type);
	strcpy(data_parameter[num_parm].name, name);
	num_parm++;
}

void def_class_begin(const char* name, const char* root)
{
	current_class = num_class;
	strcpy(data_class[num_class].name, name);
	strcpy(data_class[num_class].root, root);
	data_class[num_class].function_start = num_func;
	data_class[num_class].function_count = 0;
	data_class[num_class].is_root = (p_stack_depth==1);
	num_class++;
}

void def_class_end(const char* name)
{
	current_class = -1;
}
