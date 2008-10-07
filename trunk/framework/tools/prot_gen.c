#include <time.h>
#include <stdio.h>
#include <string.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/misc.h>
#include <skates/protocol.h>

static char c_file[50*1024];
static char h_file[50*1024];
static char mode[100], type[100], name[100], value[2000];
static int is_break;

static int parse_pfile(const char* text);
static int generate_hfile(const char* name, char* inc, unsigned int inc_len);
static int generate_cfile(const char* name, char* src, unsigned int src_len);
static int generate_hlua(const char* name, char* src, unsigned int src_len);
static int generate_clua(const char* name, char* src, unsigned int src_len);

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
static const char* parse_parameter(const char* buf);
static const char* parse_function(const char* buf);
static const char* parse_class(const char* buf);

static int get_type(const char* type);
static const char* get_type_const(const char* type);
static const char* get_ctype(const char* type);
static int check_include_filename(const char* file);
static void make_define_filename(const char* file, char* out);
static void make_include_filename(const char* file, char* out);

static void def_include(const char* name);
static void def_node(const char* mode, const char* name);
static void def_const(const char* type, const char* name, const char* value);
static void def_field(const char* mode, const char* type, const char* prefix, const char* name, const char* value);
static void def_array(const char* mode, const char* type, const char* prefix, const char* name, const char* count);
static void def_function(const char* return_type, const char* name);
static void def_parameter(const char* type, const char* name);
static void def_class_begin(const char* name);
static void def_class_end(const char* name);

static struct {
	char file[100];
} data_include[100];
static struct {
	char type[100];
	char name[100];
	char value[100];
} data_const[1000];
static struct {
	char mode[100];
	char type[100];
	char prefix[100];
	char name[100];
	char maxlen[100];
	char value[1000];
} data_variable[5000];
static struct {
	char mode[100];
	char name[100];
	int  var_start;
	int  var_count;
} data_type[1000];
static int num_inc = 0, num_const = 0, num_var = 0, num_type = 0;

static struct {
	char name[100];
	char type[100];
} data_parameter[100];
static struct {
	char return_type[100];
	char name[100];
	int class_index;
	int parameter_start;
	int parameter_count;
} data_function[100];
static struct {
	char name[100];
	int function_start;
	int function_count;
} data_class[100];
static int num_parm = 0, num_func = 0, num_class = 0;
static int current_class = -1;

static struct {
	char	o_path[100];
	char	path[100];
	char	file[100];
	char	txt[50*1024];
} p_stack[10];
static int p_stack_depth = 0;

static int parse_file(const char* file)
{
	int i, ret;
	char path[100];
	char *a, *b;

	os_getcwd(p_stack[p_stack_depth].o_path, sizeof(p_stack[p_stack_depth].o_path));
	a = strrchr(file, '\\');
	b = strrchr(file, '/');
	if(a==NULL && b==NULL) {
		strcpy(path, p_stack[p_stack_depth].o_path);
		strcpy(p_stack[p_stack_depth].o_path, file);
	} else {
		if(a<b) a = b;
		strcpy(path, file);
		path[a-file] = '\0';
		if(os_chdir(path)!=0) {
			return ERR_UNKNOWN;
		}
		os_getcwd(path, sizeof(path));
		strcpy(p_stack[p_stack_depth].o_path, a+1);
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

int main(int argc, char* argv[])
{
	int ret;
	char file[200];

	if(argc<2) {
		printf("invalid parameter\n");
		return 0;
	}

	ret = parse_file(argv[1]);
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}

	ret = generate_hfile(argv[1], h_file, sizeof(h_file));
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}
	ret = generate_cfile(argv[1], c_file, sizeof(c_file));
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}

	sprintf(file, "%s.h", argv[1]);
	save_textfile(file, h_file, 0);
	sprintf(file, "%s.c", argv[1]);
	save_textfile(file, c_file, 0);

	return 0;
}

int parse_pfile(const char* buf)
{
	const char* tbuf;

	tbuf = buf;
	for(;;) {
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

	return ERR_NOERROR;
}

int generate_hfile(const char* name, char* inc, unsigned int inc_len)
{
	int i, j, k, type, var;
	struct tm   *newTime;
    time_t      szClock;
	char buf[100];

	inc[0] = '\0';
    time(&szClock);
    newTime = localtime(&szClock);

	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "// generate by PROT_GEN.\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "// %s\n", asctime(newTime));
	make_define_filename(name, buf);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#ifndef __%s_include__\n", buf);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#define __%s_include__\n", buf);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	for(i=0; i<num_inc; i++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "#include \"%s.h\"\n", data_include[i].file);
	}
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#ifdef __cplusplus\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "extern \"C\" {\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#endif\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	for(i=0; i<num_const; i++) {
		if(get_type(data_const[i].type)==PROTOCOL_TYPE_OBJECT) {
			printf("invalid const type [%s %s = %s]\n", data_const[i].type, data_const[i].name, data_const[i].value);
			return ERR_UNKNOWN;
		}
		if(get_type(data_const[i].type)==PROTOCOL_TYPE_STRING) {
			if(value[0]!='"') {
				printf("invalid const type [%s %s = %s]\n", data_const[i].type, data_const[i].name, data_const[i].value);
				return ERR_UNKNOWN;
			}
		} else {
			if(value[0]=='"') {
				printf("invalid const type [%s %s = %s]\n", data_const[i].type, data_const[i].name, data_const[i].value);
				return ERR_UNKNOWN;
			}
		}
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "#define %s ((%s)(%s))\n", data_const[i].name, data_const[i].type, data_const[i].value);
	}
	for(type=0; type<num_type; type++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "typedef struct %s {\n", data_type[type].name);

		for(var=data_type[type].var_start; var<data_type[type].var_start+data_type[type].var_count; var++) {
			if(data_variable[var].maxlen[0]=='\0') {
				if(get_type(data_variable[var].type)==PROTOCOL_TYPE_STRING) {
					if(data_variable[var].prefix[0]=='\0') {
						printf("%s::%s string, no length\n", data_type[type].name, data_variable[var].name);
						return ERR_UNKNOWN;
					}
					snprintf(inc+strlen(inc), inc_len-strlen(inc), "	char %s[%s+1];\n", data_variable[var].name, data_variable[var].prefix);
				} else {
					snprintf(inc+strlen(inc), inc_len-strlen(inc), "	%s %s;\n", data_variable[var].type, data_variable[var].name);
				}
			} else {
				snprintf(inc+strlen(inc), inc_len-strlen(inc), "	os_int PROTOCOL_ARRAY_SIZE(%s);\n", data_variable[var].name);
				if(get_type(data_variable[var].type)==PROTOCOL_TYPE_STRING) {
					if(data_variable[var].prefix[0]=='\0') {
						printf("%s::%s string, no length\n", data_type[type].name, data_variable[var].name);
						return ERR_UNKNOWN;
					}
					snprintf(inc+strlen(inc), inc_len-strlen(inc), "	char %s[%s][%s+1];\n", data_variable[var].name, data_variable[var].maxlen, data_variable[var].prefix);
				} else {
					snprintf(inc+strlen(inc), inc_len-strlen(inc), "	%s %s[%s];\n", data_variable[var].type, data_variable[var].name, data_variable[var].maxlen);
				}
			}
		}
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "} %s;\n", data_type[type].name);
	}
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "// YIYI & ERIC 2004-2008.\n");
	for(type=0; type<num_type; type++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "extern PROTOCOL_TYPE PROTOCOL_NAME(%s);\n", data_type[type].name);
	}
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#ifdef __cplusplus\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "}\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#endif\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#ifdef __cplusplus");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	for(i=0; i<num_class; i++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

		snprintf(inc+strlen(inc), inc_len-strlen(inc), "class %s {\n", data_class[i].name);
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "public:\n");
		for(j=data_class[i].function_start; j<data_class[i].function_start+data_class[i].function_count; j++) {
			snprintf(inc+strlen(inc), inc_len-strlen(inc), "	virtual %s %s(", get_ctype(data_function[j].return_type), data_function[j].name);
			for(k=data_function[j].parameter_start; k<data_function[j].parameter_start+data_function[j].parameter_count; k++) {
				if(k==data_function[j].parameter_start) {
					snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s %s", get_ctype(data_parameter[k].type), data_parameter[k].name);
				} else {
					snprintf(inc+strlen(inc), inc_len-strlen(inc), ", %s %s", get_ctype(data_parameter[k].type), data_parameter[k].name);
				}
			}
			snprintf(inc+strlen(inc), inc_len-strlen(inc), ") = NULL;\n");
		}
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "};\n");
	}
	for(i=0; i<num_func; i++) {
		if(data_function[i].class_index>=0) continue;
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

		snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s %s(", get_ctype(data_function[i].return_type), data_function[i].name);
		for(j=data_function[i].parameter_start; j<data_function[i].parameter_start+data_function[i].parameter_count; j++) {
			if(j!=data_function[i].parameter_start) {
				snprintf(inc+strlen(inc), inc_len-strlen(inc), ", ");
			}
			snprintf(inc+strlen(inc), inc_len-strlen(inc), "%s %s", get_ctype(data_parameter[j].type), data_parameter[j].name);
		}
		snprintf(inc+strlen(inc), inc_len-strlen(inc), ");\n");
	}
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#endif");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#endif\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

	return ERR_NOERROR;
}

int generate_cfile(const char* name, char* src, unsigned int src_len)
{
	int type, var;
	struct tm   *newTime;
    time_t      szClock;
	char buf[100];

	src[0] = '\0';
    time(&szClock);
    newTime = localtime(&szClock);

	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "// generate by PROT_GEN.\n");
	snprintf(src+strlen(src), src_len-strlen(src), "// %s\n", asctime(newTime));
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/errcode.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/os.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/protocol_def.h>\n");
	make_include_filename(name, buf);
	snprintf(src+strlen(src), src_len-strlen(src), "#include \"%s.h\"\n", buf);
	for(type=0; type<num_type; type++) {
		int count = 0;
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		snprintf(src+strlen(src), src_len-strlen(src), "static PROTOCOL_VARIABLE __variable_list_%s_[] = {\n", data_type[type].name);
		for(var=data_type[type].var_start; var<data_type[type].var_start+data_type[type].var_count; var++) {
			char stype[100], obj_type[100], prelen[100];

			if(data_variable[var].maxlen[0]=='\0') {
				sprintf(stype, "%s", get_type_const(data_variable[var].type));
				if(get_type(data_variable[var].type)==PROTOCOL_TYPE_STRING) {
					sprintf(prelen, "%s+1", data_variable[var].prefix);
				} else {
					sprintf(prelen, "sizeof(%s)", data_variable[var].type);
				}
			} else {
				sprintf(stype, "%s|PROTOCOL_TYPE_ARRAY", get_type_const(data_variable[var].type));
				if(get_type(data_variable[var].type)==PROTOCOL_TYPE_STRING) {
					sprintf(prelen, "%s+1", data_variable[var].prefix);
				} else {
					sprintf(prelen, "sizeof(%s)", data_variable[var].type);
				}
			}
			if(get_type(data_variable[var].type)==PROTOCOL_TYPE_OBJECT) {
				sprintf(obj_type, "&PROTOCOL_NAME(%s)", data_variable[var].type);
			} else {
				sprintf(obj_type, "NULL");
			}
			snprintf(src+strlen(src), src_len-strlen(src), "	{\"%s\", %s, %s, %s, %s, PROTOCOL_STRUCT_OFFSET(%s, %s)},\n", data_variable[var].name, stype, obj_type, prelen, data_variable[var].maxlen[0]=='\0'?"0":data_variable[var].maxlen, data_type[type].name, data_variable[var].name);

			count++;
		}
		snprintf(src+strlen(src), src_len-strlen(src), "};\n");
		snprintf(src+strlen(src), src_len-strlen(src), "PROTOCOL_TYPE PROTOCOL_NAME(%s) = {\"%s\", &__variable_list_%s_[0], %d, sizeof(%s), {\"\", PROTOCOL_TYPE_FAKEVAR|PROTOCOL_TYPE_OBJECT, &PROTOCOL_NAME(%s), sizeof(%s), 0, 0}};\n", data_type[type].name, data_type[type].name, data_type[type].name, count, data_type[type].name, data_type[type].name, data_type[type].name);
	}
	snprintf(src+strlen(src), src_len-strlen(src), "\n");

	return ERR_NOERROR;
}

int generate_hlua(const char* name, char* src, unsigned int src_len)
{
	return ERR_NOERROR;
}

int generate_clua(const char* name, char* src, unsigned int src_len)
{
	int i, j, k;
	struct tm   *newTime;
    time_t      szClock;
	char buf[200];

	src[0] = '\0';
    time(&szClock);
    newTime = localtime(&szClock);

	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "// generate by PROT_GEN.\n");
	snprintf(src+strlen(src), src_len-strlen(src), "// %s\n", asctime(newTime));
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/errcode.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/os.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/protocol_def.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/protocol_lua.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	make_include_filename(name, buf);
	snprintf(src+strlen(src), src_len-strlen(src), "#include \"%s.lua.h\"\n", buf);

	for(i=0; i<num_class; i++) {
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		for(j=data_class[i].function_start; j<data_class[i].function_start+data_class[i].function_count; j++) {
			snprintf(src+strlen(src), src_len-strlen(src), "static PROTOCOL_LUA_PARAMETER __%s_%s_[]={\n");
			for(k=data_function[j].parameter_start; k<data_function[j].parameter_start+data_function[j].parameter_count; k++) {
				snprintf(src+strlen(src), src_len-strlen(src), "};\n");
			}
			snprintf(src+strlen(src), src_len-strlen(src), "};\n");
		}
	}


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
	buf = get_token_id(buf, type, sizeof(type));
	if(buf==NULL)
		return NULL;

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
	buf = get_token_char(buf, '{');
	if(buf==NULL) return NULL;

	def_class_begin(name);

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

int get_type(const char* type)
{
	int i;
	for(i=0; i<sizeof(base_type)/sizeof(base_type[0]); i++) {
		if(strcmp(base_type[i].name, type)==0)
			return base_type[i].type;
	}
	return PROTOCOL_TYPE_OBJECT;
}

const char* get_type_const(const char* type)
{
	int i;
	for(i=0; i<sizeof(base_type)/sizeof(base_type[0]); i++) {
		if(strcmp(base_type[i].name, type)==0)
			return base_type[i].string;
	}
	return "PROTOCOL_TYPE_OBJECT";
}

const char* get_ctype(const char* type)
{
	static char ctype[100];
	if(get_type(type)==PROTOCOL_TYPE_STRING) {
		snprintf(ctype, sizeof(ctype), "char*");
	} else if(get_type(type)==PROTOCOL_TYPE_OBJECT) {
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
	}
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
	strcpy(data_const[num_const].type, type);
	strcpy(data_const[num_const].name, name);
	strcpy(data_const[num_const].value, value);
	num_const++;
	return;
}

void def_field(const char* mode, const char* type, const char* prefix, const char* name, const char* value)
{
	if(num_var>=sizeof(data_variable)/sizeof(data_variable[0])) {
		printf("so many variable(%s:%s)\n", data_type[num_type-1].name, name);
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

void def_class_begin(const char* name)
{
	current_class = num_class;
	strcpy(data_class[num_class].name, name);
	data_class[num_class].function_start = num_func;
	data_class[num_class].function_count = 0;
	num_class++;
}

void def_class_end(const char* name)
{
	current_class = -1;
}
