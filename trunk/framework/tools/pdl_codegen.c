#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pdl_cc.h"

static void gen_plist(char* str, PARAMETER p_list[], int p_count);

static void gen_include(FILE* fp, const char* name);
static void gen_source(FILE* fp, const char* inc_name, const char* name);

static void gen_src_stub(FILE* fp, const char* name, int f, int c);
static void gen_src_proxy(FILE* fp, const char* name, int f, int c);

#define CMD_ID_START	1

extern char client_mode[100];

int gencode(const char* path, const char* filename, const char* name)
{
	char fn_i[100], fn_s[100];
	FILE *i, *s;

	sprintf(fn_i, "%s/%s.h", path, filename);
	sprintf(fn_s, "%s/%s.c", path, filename);

	i = fopen(fn_i, "w");
	s = fopen(fn_s, "w");
	if(i==NULL || s==NULL) {
		if(i!=NULL) fclose(i);
		if(s!=NULL) fclose(s);
		return -1;
	}

	gen_include(i, name);
	gen_source(s, filename, name);

	fclose(i);
	fclose(s);
	return 0;
}

static void gen_plist(char* str, PARAMETER p_list[], int p_count)
{
	int p;
	str[0] = '\0';
	for(p=0; p<p_count; p++) {
		sprintf(str+strlen(str), ", ");
		sprintf(str+strlen(str), "%s%s%s %s",
			p_list[p].len[0]=='\0'?"":"const ",
			strcmp(p_list[p].type, "string")==0?"char":p_list[p].type,
			(p_list[p].len[0]=='\0'?"":"*"),
			p_list[p].name);
	}
}

static void gen_include(FILE* fp, const char* name)
{
	int i, f, c;
	char pm[2000];
	fprintf(fp, "#ifndef _%s_INCLUDE\n", name);
	fprintf(fp, "#define _%s_INCLUDE\n", name);
	fprintf(fp, "\n");

	fprintf(fp, "#include <skates.h>\n");
	fprintf(fp, "\n");
	fprintf(fp, "#ifdef __cplusplus\n");
	fprintf(fp, "extern \"C\" {\n");
	fprintf(fp, "#endif\n");
	fprintf(fp, "\n");

	fprintf(fp, "// C_DEF : BEGIN\n");
	for(i=0; i<i_count; i++) {
		if(i_list[i].filename[strlen(i_list[i].filename)-1]==0x0d) i_list[i].filename[strlen(i_list[i].filename)-1] = '\0';
		fprintf(fp, "%s\n", i_list[i].filename);
	}
	fprintf(fp, "// C_DEF : END\n\n");

	fprintf(fp, "// implement&stub functions : BEGIN\n");
	for(f=0; f<f_count; f++) {
		for(c=0; c<f_list[f].c_count; c++) {
			//
			gen_plist(pm, f_list[f].c_list[c].p_list, f_list[f].c_list[c].p_count);

			//
			if(strcmp(f_list[f].c_list[c].mode, client_mode)==0) {
				fprintf(fp, "void %s_%s(%s_USER_CTX* user_ctx%s);\n",
					f_list[f].name,
					f_list[f].c_list[c].name,
					name,
					pm);
			} else {
				fprintf(fp, "int %s_%s(%s_USER_CTX* user_ctx%s);\n",
					f_list[f].name,
					f_list[f].c_list[c].name,
					name,
					pm);
			}
		}
	}
	fprintf(fp, "// implement&stub functions : END\n\n");

	fprintf(fp, "// extern dispatcher function\n");
	fprintf(fp, "int %s_Dispatcher(%s_USER_CTX* ctx, STREAM* stream);\n", name, name);
	fprintf(fp, "// user define function\n");
	fprintf(fp, "int %s_Newstream(%s_USER_CTX* ctx, STREAM** ptr);\n", name, name);
	fprintf(fp, "int %s_Send(%s_USER_CTX* ctx, STREAM* stream);\n", name, name);
	fprintf(fp, "int %s_Alloc(%s_USER_CTX* ctx, STREAM* stream, void** ptr, int size);\n", name, name);
	fprintf(fp, "void %s_Free(%s_USER_CTX* ctx, void* ptr);\n", name, name);
	fprintf(fp, "\n");

	fprintf(fp, "#ifdef __cplusplus\n");
	fprintf(fp, "};\n");
	fprintf(fp, "#endif\n");
	fprintf(fp, "\n");

	fprintf(fp, "#endif\n");
}

void gen_source(FILE* fp, const char* inc_name, const char* name)
{
	int f, c;

	fprintf(fp, "#include <stdlib.h>\n");
	fprintf(fp, "#include <string.h>\n");
	fprintf(fp, "#include \"%s.h\"\n", inc_name);
	fprintf(fp, "\n");

	fprintf(fp, "// Define command : BEGIN\n");
	for(f=0; f<f_count; f++) {
		for(c=0; c<f_list[f].c_count; c++) {
			//
			if(strcmp(f_list[f].c_list[c].mode, client_mode)==0) {
				fprintf(fp, "int %s_%s_stub(%s_USER_CTX* ctx, STREAM* stream);\n",
					f_list[f].name,
					f_list[f].c_list[c].name,
					name);
			} else {
			}
		}
	}
	fprintf(fp,"// Define command : END\n");
	fprintf(fp, "\n");

	fprintf(fp,"// extern dispatcher function : BEGIN\n");
	fprintf(fp,"int %s_Dispatcher(%s_USER_CTX* ctx, STREAM* stream)\n", name, name);
	fprintf(fp,"{\n");
	fprintf(fp,"	unsigned int code;\n");
	fprintf(fp,"\n");
	fprintf(fp,"	if(stream_get(stream, &code, sizeof(code))!=0) return -1;\n");
	fprintf(fp,"\n");
	fprintf(fp,"	switch(code) {\n");
	for(f=0; f<f_count; f++) {
		for(c=0; c<f_list[f].c_count; c++) {
			//
			if(strcmp(f_list[f].c_list[c].mode, client_mode)==0) {
				fprintf(fp,"	case 0x%04x0000|%s:	return %s_%s_stub(ctx, stream);\n",
					c+CMD_ID_START,
					f_list[f].id,
					f_list[f].name,
					f_list[f].c_list[c].name);
			} else {
			}
		}
	}
	fprintf(fp,"	default: return ERR_UNKNOWN;\n");
	fprintf(fp,"	}\n");
	fprintf(fp,"}\n");
	fprintf(fp,"// extern dispatcher function : END\n");
	fprintf(fp,"\n");

	for(f=0; f<f_count; f++) {
		for(c=0; c<f_list[f].c_count; c++) {
			if(strcmp(f_list[f].c_list[c].mode, client_mode)==0) {
				gen_src_stub(fp, name, f, c);
			} else {
				gen_src_proxy(fp, name, f, c);
			}
		}
	}
}

void gen_src_proxy(FILE* fp, const char* name, int f, int c)
{
	char pm[2000];
	int p;

	gen_plist(pm, f_list[f].c_list[c].p_list, f_list[f].c_list[c].p_count);
	fprintf(fp, "int %s_%s(%s_USER_CTX* user_ctx%s)\n",
					f_list[f].name,
					f_list[f].c_list[c].name,
					name,
					pm);
	fprintf(fp, "{\n");

	fprintf(fp,"	// define\n");
	fprintf(fp,"	struct {\n");
	fprintf(fp,"		int filter_id, command_id;\n");
	fprintf(fp,"		int len;\n");
	fprintf(fp,"		STREAM* stream;\n");
	fprintf(fp,"		int ret;\n");
	fprintf(fp,"	} __reserve;\n");
	fprintf(fp,"\n");
	fprintf(fp,"	// init\n");
	fprintf(fp,"	__reserve.filter_id = 0;\n");
	fprintf(fp,"	__reserve.command_id = 0;\n");
	fprintf(fp,"	__reserve.ret = %s_Newstream(user_ctx, &__reserve.stream);\n", name);
	fprintf(fp,"	if(__reserve.ret!=ERR_NOERROR) return __reserve.ret;\n");
	fprintf(fp,"	__reserve.filter_id = %s;\n", f_list[f].id);
	fprintf(fp,"	__reserve.ret = stream_put(__reserve.stream, &__reserve.filter_id, sizeof(short));\n");
	fprintf(fp,"	if(__reserve.ret!=ERR_NOERROR) {\n");
	fprintf(fp,"		stream_destroy(__reserve.stream);\n");
	fprintf(fp,"		return __reserve.ret;\n");
	fprintf(fp,"	}\n");
	fprintf(fp,"	__reserve.command_id = %d;\n", c+CMD_ID_START);
	fprintf(fp,"	__reserve.ret = stream_put(__reserve.stream, &__reserve.command_id, sizeof(short));\n");
	fprintf(fp,"	if(__reserve.ret!=ERR_NOERROR) {\n");
	fprintf(fp,"		stream_destroy(__reserve.stream);\n");
	fprintf(fp,"		return __reserve.ret;\n");
	fprintf(fp,"	}\n");
	fprintf(fp,"\n");

	fprintf(fp,"	// fill\n");
	for(p=0; p<f_list[f].c_list[c].p_count; p++) {
		if(f_list[f].c_list[c].p_list[p].len[0]!='\0') {
			fprintf(fp,"    // len [%s]\n", f_list[f].c_list[c].p_list[p].name);
			fprintf(fp,"	__reserve.len = %s;\n", f_list[f].c_list[c].p_list[p].len);
			if(f_list[f].c_list[c].p_list[p].rule[0]!='\0') {
				char buf[300];
				char* ct;
				char* t;
				int len;
				ct = f_list[f].c_list[c].p_list[p].rule;
				buf[0] = '\0';
				for(;;) {
					t = strstr(ct, "$$");
					if(t==NULL) break;
					len = strlen(buf);
					memcpy(buf+len, ct, t-ct);
					buf[len+t-ct] = '\0';
					strcat(buf, "__reserve.len");
					ct = t + 2;
				}
				strcat(buf, ct);             

				fprintf(fp,"    // check rule [%s]\n", f_list[f].c_list[c].p_list[p].name);
				fprintf(fp,"    if(!(%s)) {\n", buf);
				fprintf(fp,"        return(ERR_INVALID_DATA);\n");
				fprintf(fp,"    }\n");
			}
			fprintf(fp,"    // write len [%s]\n", f_list[f].c_list[c].p_list[p].name);
			fprintf(fp,"	__reserve.ret = stream_put(__reserve.stream, &__reserve.len, sizeof(short));\n");
			fprintf(fp,"	if(__reserve.ret!=ERR_NOERROR) {\n");
			fprintf(fp,"		stream_destroy(__reserve.stream);\n");
			fprintf(fp,"		return __reserve.ret;\n");
			fprintf(fp,"	}\n");
		}
		fprintf(fp,"    // write data [%s]\n", f_list[f].c_list[c].p_list[p].name);
		fprintf(fp,"	__reserve.ret = stream_put(__reserve.stream, %s%s, sizeof(%s)*(%s));\n",
			f_list[f].c_list[c].p_list[p].len[0]=='\0'?"&":"",
			f_list[f].c_list[c].p_list[p].name,
			strcmp(f_list[f].c_list[c].p_list[p].type, "string")==0?"char":f_list[f].c_list[c].p_list[p].type,
			f_list[f].c_list[c].p_list[p].len[0]=='\0'?"1":"__reserve.len"
			);
		fprintf(fp,"	if(__reserve.ret!=ERR_NOERROR) {\n");
		fprintf(fp,"		stream_destroy(__reserve.stream);\n");
		fprintf(fp,"		return __reserve.ret;\n");
		fprintf(fp,"	}\n");
	}
	fprintf(fp,"\n");

	fprintf(fp,"	// send\n");
	fprintf(fp,"	__reserve.ret = %s_Send(user_ctx, __reserve.stream);\n", name);
	fprintf(fp,"	if(__reserve.ret!=ERR_NOERROR) {\n");
	fprintf(fp,"		stream_destroy(__reserve.stream);\n");
	fprintf(fp,"		return __reserve.ret;\n");
	fprintf(fp,"	}\n");
	fprintf(fp,"\n");

	fprintf(fp,"	// end\n");
	fprintf(fp,"	return ERR_NOERROR;\n");
	fprintf(fp,"}\n");
	fprintf(fp,"\n");
}

void gen_src_stub(FILE* fp, const char* name, int f, int c)
{
	int p, onerror = 0;
	fprintf(fp, "int %s_%s_stub(%s_USER_CTX* user_ctx, STREAM* stream)\n",
		f_list[f].name,
		f_list[f].c_list[c].name,
		name
		);
	fprintf(fp,"{\n");

	fprintf(fp,"	// define\n");
	if(f_list[f].c_list[c].p_count>0) {
	fprintf(fp,"	struct {\n");
	for(p=0; p<f_list[f].c_list[c].p_count; p++) {
		fprintf(fp,"		%s%s %s;\n",
			strcmp(f_list[f].c_list[c].p_list[p].type, "string")==0?"char":f_list[f].c_list[c].p_list[p].type,
			f_list[f].c_list[c].p_list[p].len[0]=='\0'?"":"*",
			f_list[f].c_list[c].p_list[p].name
			);
	}
	fprintf(fp,"	} __reserve;\n");
	}
	fprintf(fp,"	int len, ret;\n");
	fprintf(fp,"\n");

	fprintf(fp,"	// init\n");
	if(f_list[f].c_list[c].p_count>0) {
		fprintf(fp,"	memset(&__reserve, 0, sizeof(__reserve));\n");
	}
	fprintf(fp,"	len = 0;\n");
	fprintf(fp,"	ret = 0;\n");
	fprintf(fp,"\n");

	fprintf(fp,"	// read from stream\n");
	for(p=0; p<f_list[f].c_list[c].p_count; p++) {
		if(f_list[f].c_list[c].p_list[p].len[0]=='\0') {
			fprintf(fp,"	ret = stream_get(stream, &__reserve.%s, sizeof(%s));\n",
				f_list[f].c_list[c].p_list[p].name,
				strcmp(f_list[f].c_list[c].p_list[p].type, "string")==0?"char":f_list[f].c_list[c].p_list[p].type
				);
			fprintf(fp,"	if(ret!=ERR_NOERROR) goto ON_ERROR;\n");
			onerror = 1;
		} else {
			fprintf(fp,"	// get [%s] len\n", f_list[f].c_list[c].p_list[p].name);
			fprintf(fp,"	len = 0;\n");
			fprintf(fp,"	ret = stream_get(stream, &len, sizeof(short));\n");
			fprintf(fp,"	if(ret!=ERR_NOERROR) goto ON_ERROR;\n");
			if(f_list[f].c_list[c].p_list[p].rule[0]!='\0') {
				char buf[300];
				char* ct;
				char* t;
				int len;
				ct = f_list[f].c_list[c].p_list[p].rule;
				buf[0] = '\0';
				for(;;) {
					t = strstr(ct, "$$");
					if(t==NULL) break;
					len = strlen(buf);
					memcpy(buf+len, ct, t-ct);
					buf[len+t-ct] = '\0';
					strcat(buf, "len");
					ct = t + 2;
				}
				strcat(buf, ct);

				fprintf(fp,"	// check rule [%s]\n", f_list[f].c_list[c].p_list[p].name);
				fprintf(fp,"	if(!(%s)) {\n", buf);
				fprintf(fp,"		ret = ERR_INVALID_DATA;\n");
				fprintf(fp,"		goto ON_ERROR;\n");
				fprintf(fp,"	}\n");
			}
			fprintf(fp,"	// alloc memory [%s]\n", f_list[f].c_list[c].p_list[p].name);
			fprintf(fp,"	ret = %s_Alloc(user_ctx, stream, (void**)&__reserve.%s, sizeof(%s)*%s);\n",
						name,
						f_list[f].c_list[c].p_list[p].name,
						strcmp(f_list[f].c_list[c].p_list[p].type, "string")==0?"char":f_list[f].c_list[c].p_list[p].type,
						strcmp(f_list[f].c_list[c].p_list[p].type, "string")==0?"(len+1)":"len"
				);
			fprintf(fp,"	if(ret!=ERR_NOERROR) goto ON_ERROR;\n");
			fprintf(fp,"	// get data [%s]\n", f_list[f].c_list[c].p_list[p].name);
			fprintf(fp,"	ret = stream_get(stream, __reserve.%s, sizeof(%s)*len);\n",
						f_list[f].c_list[c].p_list[p].name,
						strcmp(f_list[f].c_list[c].p_list[p].type, "string")==0?"char":f_list[f].c_list[c].p_list[p].type
				);
			fprintf(fp,"	if(ret!=ERR_NOERROR) goto ON_ERROR;\n");
			if(strcmp(f_list[f].c_list[c].p_list[p].type, "string")==0) {
				fprintf(fp,"	__reserve.%s[len-1] = '\\0';\n", f_list[f].c_list[c].p_list[p].name);
			}
			onerror = 1;
		}
	}
	fprintf(fp,"\n");

	fprintf(fp,"	// call\n");
	fprintf(fp,"	%s_%s(user_ctx", f_list[f].name, f_list[f].c_list[c].name);
	for(p=0; p<f_list[f].c_list[c].p_count; p++) {
		fprintf(fp,", __reserve.%s", f_list[f].c_list[c].p_list[p].name);
	}
	fprintf(fp,");\n");
	fprintf(fp,"\n");

	fprintf(fp,"	// free memory\n");
	for(p=f_list[f].c_list[c].p_count-1; p>=0; p--) {
		if(f_list[f].c_list[c].p_list[p].len[0]=='\0') {
		} else {
			fprintf(fp,"	%s_Free(user_ctx, __reserve.%s);\n",
						name,
						f_list[f].c_list[c].p_list[p].name
				);
		}
	}

	fprintf(fp,"	// end\n");
	fprintf(fp,"	return ERR_NOERROR;\n");
if(onerror!=0) {
	fprintf(fp,"\n");
	fprintf(fp,"ON_ERROR:\n");
	for(p=f_list[f].c_list[c].p_count-1; p>=0; p--) {
		if(f_list[f].c_list[c].p_list[p].len[0]=='\0') {
		} else {
			fprintf(fp,"	if(__reserve.%s!=NULL) %s_Free(user_ctx, __reserve.%s);\n",
						f_list[f].c_list[c].p_list[p].name,
						name,
						f_list[f].c_list[c].p_list[p].name
				);
		}
	}
	fprintf(fp,"\n");
	fprintf(fp,"	return ret;\n");
}
	fprintf(fp,"}\n");
	fprintf(fp,"\n");
}

