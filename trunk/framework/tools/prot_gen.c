#include <time.h>
#include <stdio.h>
#include <string.h>

#include <skates/errcode.h>
#include <skates/os.h>
#include <skates/misc.h>
#include <skates/protocol.h>

#include "prot_parser.h"

static char txt[50*1024];
static int generate_hfile(const char* name, char* inc, unsigned int inc_len);
static int generate_cfile(const char* name, char* src, unsigned int src_len);
static int generate_hlua(const char* name, char* src, unsigned int src_len);
static int generate_clua(const char* name, char* src, unsigned int src_len);

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

	memset(txt, 0, sizeof(txt));
	ret = generate_hfile(argv[1], txt, sizeof(txt));
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}
	sprintf(file, "%s.h", argv[1]);
	save_textfile(file, txt, 0);

	memset(txt, 0, sizeof(txt));
	ret = generate_cfile(argv[1], txt, sizeof(txt));
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}
	sprintf(file, "%s.c", argv[1]);
	save_textfile(file, txt, 0);

	memset(txt, 0, sizeof(txt));
	ret = generate_hlua(argv[1], txt, sizeof(txt));
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}
	sprintf(file, "%s.lua.h", argv[1]);
	save_textfile(file, txt, 0);

	memset(txt, 0, sizeof(txt));
	ret = generate_clua(argv[1], txt, sizeof(txt));
	if(ret!=ERR_NOERROR) {
		printf("error: parse!\n");
		return 0;
	}
	sprintf(file, "%s.lua.cc", argv[1]);
	save_textfile(file, txt, 0);

	return 0;
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
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#ifndef __%s_INCLUDE__\n", buf);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#define __%s_INCLUDE__\n", buf);
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	for(i=0; i<num_inc; i++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "#include \"%s.h\"\n", data_include[i].file);
	}
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#ifdef __cplusplus\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "extern \"C\" {\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#endif\n");
	if(num_const)
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	for(i=0; i<num_const; i++) {
		if(get_basetype(data_const[i].type)==0) {
			printf("invalid const type [%s %s = %s]\n", data_const[i].type, data_const[i].name, data_const[i].value);
			return ERR_UNKNOWN;
		}
		if(get_basetype(data_const[i].type)==PROTOCOL_TYPE_STRING) {
			if(data_const[i].value[0]!='"') {
				printf("invalid const type [%s %s = %s]\n", data_const[i].type, data_const[i].name, data_const[i].value);
				return ERR_UNKNOWN;
			}
		} else {
			if(data_const[i].value[0]=='"') {
				printf("invalid const type [%s %s = %s]\n", data_const[i].type, data_const[i].name, data_const[i].value);
				return ERR_UNKNOWN;
			}
		}
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "#define %s ((%s)(%s))\n", data_const[i].name, data_const[i].type, data_const[i].value);
	}
	for(type=0; type<num_type; type++) {
		if(!data_type[type].is_root) continue;

		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "typedef struct %s {\n", data_type[type].name);

		for(var=data_type[type].var_start; var<data_type[type].var_start+data_type[type].var_count; var++) {
			if(data_variable[var].maxlen[0]=='\0') {
				if(get_basetype(data_variable[var].type)==PROTOCOL_TYPE_STRING) {
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
				if(get_basetype(data_variable[var].type)==PROTOCOL_TYPE_STRING) {
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
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "// YE 2004-2008.\n");
	for(type=0; type<num_type; type++) {
		if(!data_type[type].is_root) continue;
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "extern PROTOCOL_TYPE PROTOCOL_NAME(%s);\n", data_type[type].name);
	}
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#ifdef __cplusplus\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "}\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#endif\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "// class\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#ifdef __cplusplus\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");

	for(i=0; i<num_class; i++) {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "class %s;\n", data_class[i].name);
	}

	for(i=0; i<num_class; i++) {
		if(!data_class[i].is_root) continue;

		snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
		if(data_class[i].root[0]=='\0') {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "class %s {\n", data_class[i].name);
		} else {
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "class %s : %s {\n", data_class[i].name, data_class[i].root);
		}
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
			snprintf(inc+strlen(inc), inc_len-strlen(inc), ") = 0;\n");
		}
		snprintf(inc+strlen(inc), inc_len-strlen(inc), "};\n");
	}
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#endif");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "\n");
	snprintf(inc+strlen(inc), inc_len-strlen(inc), "#endif\n");

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
	snprintf(src+strlen(src), src_len-strlen(src), "#include <stddef.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/errcode.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/os.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/protocol_def.h>\n");
	make_include_filename(name, buf);
	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#include \"%s.h\"\n", buf);
	for(type=0; type<num_type; type++) {
		int count = 0;
		if(!data_type[type].is_root) continue;

		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		snprintf(src+strlen(src), src_len-strlen(src), "static PROTOCOL_VARIABLE __variable_list_%s_[] = {\n", data_type[type].name);
		for(var=data_type[type].var_start; var<data_type[type].var_start+data_type[type].var_count; var++) {
			char stype[100], obj_type[100], prelen[100];

			if(data_variable[var].maxlen[0]=='\0') {
				sprintf(stype, "%s", get_type_const(data_variable[var].type));
				if(get_basetype(data_variable[var].type)==PROTOCOL_TYPE_STRING) {
					sprintf(prelen, "%s+1", data_variable[var].prefix);
				} else {
					sprintf(prelen, "sizeof(%s)", data_variable[var].type);
				}
			} else {
				sprintf(stype, "%s|PROTOCOL_TYPE_ARRAY", get_type_const(data_variable[var].type));
				if(get_basetype(data_variable[var].type)==PROTOCOL_TYPE_STRING) {
					sprintf(prelen, "%s+1", data_variable[var].prefix);
				} else {
					sprintf(prelen, "sizeof(%s)", data_variable[var].type);
				}
			}
			if(get_basetype(data_variable[var].type)==0) {
				sprintf(obj_type, "&PROTOCOL_NAME(%s)", data_variable[var].type);
			} else {
				sprintf(obj_type, "NULL");
			}
			snprintf(src+strlen(src), src_len-strlen(src), "	{\"%s\", %s, %s, %s, %s, (unsigned int)offsetof(%s, %s)},\n", data_variable[var].name, stype, obj_type, prelen, data_variable[var].maxlen[0]=='\0'?"0":data_variable[var].maxlen, data_type[type].name, data_variable[var].name);

			count++;
		}
		snprintf(src+strlen(src), src_len-strlen(src), "};\n");
		snprintf(src+strlen(src), src_len-strlen(src), "PROTOCOL_TYPE PROTOCOL_NAME(%s) = {\"%s\", &__variable_list_%s_[0], %d, sizeof(%s), {\"\", PROTOCOL_TYPE_FAKEVAR|PROTOCOL_TYPE_STRUCT, &PROTOCOL_NAME(%s), sizeof(%s), 0, 0}};\n", data_type[type].name, data_type[type].name, data_type[type].name, count, data_type[type].name, data_type[type].name, data_type[type].name);
	}
	snprintf(src+strlen(src), src_len-strlen(src), "\n");

	return ERR_NOERROR;
}

int generate_hlua(const char* name, char* src, unsigned int src_len)
{
	int i;
	struct tm   *newTime;
    time_t      szClock;
	char buf[200];

	src[0] = '\0';
    time(&szClock);
    newTime = localtime(&szClock);

	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "// generate by PROT_GEN.\n");
	snprintf(src+strlen(src), src_len-strlen(src), "// %s\n", asctime(newTime));
	make_define_filename(name, buf);
	snprintf(src+strlen(src), src_len-strlen(src), "#ifndef __%s_LUA_INCLUDE__\n", buf);
	snprintf(src+strlen(src), src_len-strlen(src), "#define __%s_LUA_INCLUDE__\n", buf);
	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	for(i=0; i<num_inc; i++) {
		snprintf(src+strlen(src), src_len-strlen(src), "#include \"%s.lua.h\"\n", data_include[i].file);
	}
	make_include_filename(name, buf);
	snprintf(src+strlen(src), src_len-strlen(src), "#include \"%s.h\"\n", buf);
	snprintf(src+strlen(src), src_len-strlen(src), "\n");

	snprintf(src+strlen(src), src_len-strlen(src), "#ifdef __cplusplus\n");
	snprintf(src+strlen(src), src_len-strlen(src), "extern \"C\" {\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#endif\n");
	snprintf(src+strlen(src), src_len-strlen(src), "\n");

	for(i=0; i<num_class; i++) {
		snprintf(src+strlen(src), src_len-strlen(src), "extern PROTOCOL_LUA_CLASS PROTOCOL_NAME(%s);\n", data_class[i].name);
	}

	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#ifdef __cplusplus\n");
	snprintf(src+strlen(src), src_len-strlen(src), "}\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#endif\n");

	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	snprintf(src+strlen(src), src_len-strlen(src), "#endif\n");

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
	snprintf(src+strlen(src), src_len-strlen(src), "#include <skates/skates.h>\n");
	snprintf(src+strlen(src), src_len-strlen(src), "\n");
	make_include_filename(name, buf);
	snprintf(src+strlen(src), src_len-strlen(src), "#include \"%s.lua.h\"\n", buf);

	for(i=0; i<num_class; i++) {
		snprintf(src+strlen(src), src_len-strlen(src), "\n");
		snprintf(src+strlen(src), src_len-strlen(src), "\n");

		for(j=data_class[i].function_start; j<data_class[i].function_start+data_class[i].function_count; j++) {
			if(data_function[j].parameter_count) {
				snprintf(src+strlen(src), src_len-strlen(src), "static PROTOCOL_LUA_PARAMETER __%s_%s_[]={\n", data_class[i].name, data_function[j].name);
				for(k=data_function[j].parameter_start; k<data_function[j].parameter_start+data_function[j].parameter_count; k++) {
					snprintf(src+strlen(src), src_len-strlen(src), "\t{");
					if(get_basetype(data_parameter[k].type)!=0) {
						snprintf(src+strlen(src), src_len-strlen(src), "%s, \"%s\", NULL, NULL, \"%s\"", get_type_const(data_parameter[k].type), data_parameter[k].type, data_parameter[k].name);
					} else if(check_vailid_dtype(data_parameter[k].type)) {
						snprintf(src+strlen(src), src_len-strlen(src), "PROTOCOL_TYPE_STRUCT, \"%s\", &PROTOCOL_NAME(%s), NULL, \"%s\"", data_parameter[k].type, data_parameter[k].type, data_parameter[k].name);
					} else if(check_vailid_otype(data_parameter[k].type)) {
						snprintf(src+strlen(src), src_len-strlen(src), "PROTOCOL_TYPE_OBJECT, \"%s\", NULL, &PROTOCOL_NAME(%s), \"%s\"", data_parameter[k].type, data_parameter[k].type, data_parameter[k].name);
					} else {
						printf("invalid type\n");
						return ERR_UNKNOWN;
					}
					snprintf(src+strlen(src), src_len-strlen(src), "},\n");
				}
				snprintf(src+strlen(src), src_len-strlen(src), "};\n");
			} else {
				snprintf(src+strlen(src), src_len-strlen(src), "static PROTOCOL_LUA_PARAMETER __%s_%s_[1];\n", data_class[i].name, data_function[j].name);
			}

			if(strcmp(data_function[j].return_type, "void")!=0) {
				snprintf(src+strlen(src), src_len-strlen(src), "static PROTOCOL_LUA_PARAMETER __%s_%s_return_={\n\t", data_class[i].name, data_function[j].name);
				if(get_basetype(data_function[j].return_type)!=0) {
					snprintf(src+strlen(src), src_len-strlen(src), "%s, \"%s\", NULL, NULL, \"%s\"", get_type_const(data_function[j].return_type), get_ctype(data_function[j].return_type), "");
				} else if(check_vailid_dtype(data_function[j].return_type)) {
					snprintf(src+strlen(src), src_len-strlen(src), "PROTOCOL_TYPE_STRUCT, \"%s\", &PROTOCOL_NAME(%s), NULL, \"%s\"", data_function[j].return_type, data_function[j].return_type, "");
				} else if(check_vailid_otype(data_function[j].return_type)) {
					snprintf(src+strlen(src), src_len-strlen(src), "PROTOCOL_TYPE_OBJECT, \"%s\", NULL, &PROTOCOL_NAME(%s), \"%s\"", data_function[j].return_type, data_function[j].return_type, "");
				} else {
					printf("invalid type\n");
					return ERR_UNKNOWN;
				}
				snprintf(src+strlen(src), src_len-strlen(src), "\n};\n");
			}
		}

		for(j=data_class[i].function_start; j<data_class[i].function_start+data_class[i].function_count; j++) {
			char arg_def[300];
			char arg_list[300];

			snprintf(src+strlen(src), src_len-strlen(src), "static int luafunc_%s_%s(lua_State* L)\n", data_class[i].name, data_function[j].name);
			snprintf(src+strlen(src), src_len-strlen(src), "{\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	if(!luaL_isobject(L, 1, &PROTOCOL_NAME(%s))) {\n", data_class[i].name);
			snprintf(src+strlen(src), src_len-strlen(src), "		luaL_error(L, \"invalid parameter type.\\n\");\n");
			snprintf(src+strlen(src), src_len-strlen(src), "		return 0;\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	}\n");
			snprintf(src+strlen(src), src_len-strlen(src), "	void* obj = luaL_toobject(L, 1, &PROTOCOL_NAME(%s));\n", data_class[i].name);
			if(strcmp(data_function[j].return_type, "void")!=0) {
				snprintf(src+strlen(src), src_len-strlen(src), "	%s return_value;\n", get_ctype(data_function[j].return_type));
			}
			arg_def[0] = '\0';
			arg_list[0] = '\0';
			for(k=data_function[j].parameter_start; k<data_function[j].parameter_start+data_function[j].parameter_count; k++) {
				if(k>data_function[j].parameter_start) {
					snprintf(arg_def+strlen(arg_def), sizeof(arg_def)-strlen(arg_def), ", ");
					snprintf(arg_list+strlen(arg_list), sizeof(arg_list)-strlen(arg_list), ", ");
				}
				snprintf(src+strlen(src), src_len-strlen(src), "	%s %s;\n", get_ctype(data_parameter[k].type), data_parameter[k].name);
				snprintf(arg_def+strlen(arg_def), sizeof(arg_def)-strlen(arg_def), "%s ", get_ctype(data_parameter[k].type));
				snprintf(arg_def+strlen(arg_def), sizeof(arg_def)-strlen(arg_def), "%s ", data_parameter[k].name);
				snprintf(arg_list+strlen(arg_list), sizeof(arg_list)-strlen(arg_list), "%s", data_parameter[k].name);
			}
			for(k=data_function[j].parameter_start; k<data_function[j].parameter_start+data_function[j].parameter_count; k++) {
				snprintf(src+strlen(src), src_len-strlen(src), "	if(protocol_lua_getvalue(L, %d, &__%s_%s_[%d], &%s)!=ERR_NOERROR) {\n",
 k-data_function[j].parameter_start+2, data_class[i].name, data_function[j].name, k-data_function[j].parameter_start,
 data_parameter[k].name);
				snprintf(src+strlen(src), src_len-strlen(src), "		luaL_error(L, \"invalid parameter type.\\n\");\n");
				snprintf(src+strlen(src), src_len-strlen(src), "		return 0;\n");
				snprintf(src+strlen(src), src_len-strlen(src), "	}\n");
			}
			snprintf(src+strlen(src), src_len-strlen(src), "	%s (%s::*%s)(%s);\n", get_ctype(data_function[j].return_type), data_class[i].name, data_function[j].name, arg_def);
			snprintf(src+strlen(src), src_len-strlen(src), "	*((void**)&%s) = ((void**)(*((void**)obj)))[%d];\n", data_function[j].name, j-data_class[i].function_start);
			if(strcmp(data_function[j].return_type, "void")==0) {
				snprintf(src+strlen(src), src_len-strlen(src), "	(((%s*)obj)->*%s)(%s);\n", data_class[i].name, data_function[j].name, arg_list);
				snprintf(src+strlen(src), src_len-strlen(src), "	return 0;\n");
			} else {
				snprintf(src+strlen(src), src_len-strlen(src), "	return_value = (((%s*)obj)->*%s)(%s);\n",
data_class[i].name, data_function[j].name, arg_list);
				snprintf(src+strlen(src), src_len-strlen(src), "	protocol_lua_pushvalue(L, &__%s_%s_return_, &return_value);\n",
data_class[i].name, data_function[j].name);
				snprintf(src+strlen(src), src_len-strlen(src), "	return 1;\n");
			}
			snprintf(src+strlen(src), src_len-strlen(src), "}\n");
		}

		if(data_class[i].function_count) {
			snprintf(src+strlen(src), src_len-strlen(src), "static PROTOCOL_LUA_FUNCTION __%s_[]={\n", data_class[i].name);
			for(j=data_class[i].function_start; j<data_class[i].function_start+data_class[i].function_count; j++) {
				snprintf(src+strlen(src), src_len-strlen(src), "\t{");
				if(strcmp(data_function[j].return_type, "void")!=0) {
					snprintf(src+strlen(src), src_len-strlen(src), "&__%s_%s_return_,", data_class[i].name, data_function[j].name);
				} else {
					snprintf(src+strlen(src), src_len-strlen(src), "NULL, ");
				}
				snprintf(src+strlen(src), src_len-strlen(src), " \"%s\",", data_function[j].name);
				snprintf(src+strlen(src), src_len-strlen(src), " &__%s_%s_[0],", data_class[i].name, data_function[j].name);
				snprintf(src+strlen(src), src_len-strlen(src), " %d,", data_function[j].parameter_count);
				snprintf(src+strlen(src), src_len-strlen(src), " luafunc_%s_%s", data_class[i].name, data_function[j].name);
				snprintf(src+strlen(src), src_len-strlen(src), "},\n");
			}
			snprintf(src+strlen(src), src_len-strlen(src), "};\n");
		} else {
			snprintf(src+strlen(src), src_len-strlen(src), "static PROTOCOL_LUA_FUNCTION __%s_[1];\n", data_class[i].name);
		}

		snprintf(src+strlen(src), src_len-strlen(src), "PROTOCOL_LUA_CLASS PROTOCOL_NAME(%s) = {\n", data_class[i].name);
		if(data_class[i].root[0]=='\0') {
		snprintf(src+strlen(src), src_len-strlen(src), "\t\"%s\", NULL, &__%s_[0], %d\n",
			data_class[i].name, data_class[i].name, data_class[i].function_count);
		} else {
		snprintf(src+strlen(src), src_len-strlen(src), "\t\"%s\", PROTOCOL_NAME(%s), &__%s_[0], %d\n",
			data_class[i].name, data_class[i].root, data_class[i].name, data_class[i].function_count);
		}
		snprintf(src+strlen(src), src_len-strlen(src), "};\n");
	}

	return ERR_NOERROR;
}
