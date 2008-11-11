#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../framework/tools/pdl_cc.h"
#include "../../framework/tools/pdl_codegen.h"

static void gen_plist(char* str, PARAMETER p_list[], int p_count);

static void gen_include(FILE* fp, const char* pdl_prefix, const char* name_prefix, const char* class_prefix);
static void gen_source(FILE* fp, const char* pdl_prefix, const char* name_prefix, const char* class_prefix);

int gencode(const char* path, const char* filename, const char* name)
{
	char fn_i[100], fn_s[100];
	FILE *i, *s;

	sprintf(fn_i, "%s.hook.hpp", filename);
	sprintf(fn_s, "%s.hook.cpp", filename);

	i = fopen(fn_i, "w");
	s = fopen(fn_s, "w");
	if(i==NULL || s==NULL) {
		if(i==NULL) fprintf(stderr, "error : Failed to open %s\n", fn_i);
		if(s==NULL) fprintf(stderr, "error : Failed to open %s\n", fn_s);
		if(i!=NULL) fclose(i);
		if(s!=NULL) fclose(s);
		return -1;
	}

	gen_include(i, path, filename, name);
	gen_source(s, path, filename, name);

	fclose(i);
	fclose(s);
	return 0;
}

static void gen_plist(char* str, PARAMETER p_list[], int p_count)
{
	int p;
	str[0] = '\0';
	for(p=0; p<p_count; p++) {
		if(p>0) sprintf(str+strlen(str), ", ");
		sprintf(str+strlen(str), "%s%s%s %s",
			p_list[p].len[0]=='\0'?"":"const ",
			strcmp(p_list[p].type, "string")==0?"char":p_list[p].type,
			(p_list[p].len[0]=='\0'?"":"*"),
			p_list[p].name);
	}
}

void gen_include(FILE* fp, const char* pdl_prefix, const char* name_prefix, const char* class_prefix)
{
	int f, c, p;
	char pm[200];
	fprintf(fp, "#ifndef __CUBE_HOOK_INCLUDE\n");
	fprintf(fp, "#define __CUBE_HOOK_INCLUDE\n");
	fprintf(fp, "\n");

	fprintf(fp, "class %s {\n", class_prefix);
	fprintf(fp, "public:\n");
	for(f=0; f<f_count; f++) {
		for(c=0; c<f_list[f].c_count; c++) {
			if(strcmp(f_list[f].c_list[c].mode, "CMD_CLT")!=0) continue;
			gen_plist(pm, f_list[f].c_list[c].p_list, f_list[f].c_list[c].p_count);
			fprintf(fp, "	virtual void %s_%s(%s) {}\n", f_list[f].name, f_list[f].c_list[c].name, pm);
		}
	}
	fprintf(fp, "};\n");
	fprintf(fp, "\n");

	fprintf(fp, "class %sDispatcher : public %s {\n", class_prefix, class_prefix);
	fprintf(fp, "protected:\n");
	fprintf(fp, "	virtual %s* FindReset() = NULL;\n", class_prefix);
	fprintf(fp, "	virtual %s* FindNext() = NULL;\n", class_prefix);
	fprintf(fp, "	virtual %s* FindGet() = NULL;\n", class_prefix);
	fprintf(fp, "\n");
	fprintf(fp, "public:\n");
	fprintf(fp, "	static %sDispatcher* GetDispatcher(CLT_USER_CTX* CTX);\n", class_prefix);
	fprintf(fp, "\n");
	fprintf(fp, "public:\n");
	for(f=0; f<f_count; f++) {
		for(c=0; c<f_list[f].c_count; c++) {
			if(strcmp(f_list[f].c_list[c].mode, "CMD_CLT")!=0) continue;
			gen_plist(pm, f_list[f].c_list[c].p_list, f_list[f].c_list[c].p_count);
			fprintf(fp, "	virtual void %s_%s(%s) {\n", f_list[f].name, f_list[f].c_list[c].name, pm);
			fprintf(fp, "		FindReset();\n");
			fprintf(fp, "		while(FindGet()!=NULL) {\n");

			fprintf(fp,	"			FindGet()->%s_%s(", f_list[f].name, f_list[f].c_list[c].name);
			for(p=0; p<f_list[f].c_list[c].p_count; p++) {
				if(p>0) fprintf(fp, ", ");
				fprintf(fp, "%s", f_list[f].c_list[c].p_list[p].name);
			}
			fprintf(fp,	");\n");

			fprintf(fp,	"			FindNext();\n");
			fprintf(fp, "		}\n");
			fprintf(fp, "	}\n");
			fprintf(fp, "\n");
		}
	}
	fprintf(fp, "};\n");
	fprintf(fp, "\n");

	fprintf(fp, "\n");
	fprintf(fp, "#endif\n");
}

void gen_source(FILE* fp, const char* pdl_prefix, const char* name_prefix, const char* class_prefix)
{
	int f, c, p;
	char pm[200];

	fprintf(fp, "\n");
	fprintf(fp, "#include \"%s.h\"\n", name_prefix);
	fprintf(fp, "#include \"%s.hook.hpp\"\n", name_prefix);
	fprintf(fp, "\n");

	for(f=0; f<f_count; f++) {
		for(c=0; c<f_list[f].c_count; c++) {
			if(strcmp(f_list[f].c_list[c].mode, "CMD_CLT")!=0) continue;
			gen_plist(pm, f_list[f].c_list[c].p_list, f_list[f].c_list[c].p_count);
			fprintf(fp, "void %s_%s(%s%s)\n",
				f_list[f].name, f_list[f].c_list[c].name,
				f_list[f].c_list[c].p_count==0?"CLT_USER_CTX* user_ctx":"CLT_USER_CTX* user_ctx, ",
				pm);
			fprintf(fp, "{\n");
			fprintf(fp, "	CubeHookDispatcher::GetDispatcher(user_ctx)->%s_%s(", f_list[f].name, f_list[f].c_list[c].name);
			for(p=0; p<f_list[f].c_list[c].p_count; p++) {
				if(p>0) fprintf(fp, ", ");
				fprintf(fp, "%s", f_list[f].c_list[c].p_list[p].name);
			}
			fprintf(fp, ");\n");

			fprintf(fp, "}\n");
			fprintf(fp, "\n");
		}
	}
}
