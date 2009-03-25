#include <iostream>
#include <string>
#include <vector>

#include "parser.h"
#include "codegen.h"

static const char* type2type(const char* name)
{
	if(strcmp(name, "char")==0)		return "PROPERTY_TYPE_CHAR";
	if(strcmp(name, "short")==0)	return "PROPERTY_TYPE_SHORT";
	if(strcmp(name, "int")==0)		return "PROPERTY_TYPE_INT";
	if(strcmp(name, "byte")==0)		return "PROPERTY_TYPE_BYTE";
	if(strcmp(name, "word")==0)		return "PROPERTY_TYPE_WORD";
	if(strcmp(name, "dword")==0)	return "PROPERTY_TYPE_DWORD";
	if(strcmp(name, "float")==0)	return "PROPERTY_TYPE_FLOAT";
	if(strcmp(name, "string")==0)	return "PROPERTY_TYPE_STRING";
	return NULL;
}

static const char* type2type(const PDL_ARG* arg)
{
	if(strcmp(arg->type, "char")==0)	return "sizeof(char)";
	if(strcmp(arg->type, "short")==0)	return "sizeof(short)";
	if(strcmp(arg->type, "int")==0)		return "sizeof(int)";
	if(strcmp(arg->type, "byte")==0)	return "sizeof(unsigned char)";
	if(strcmp(arg->type, "word")==0)	return "sizeof(unsigned short)";
	if(strcmp(arg->type, "dword")==0)	return "sizeof(unsigned int)";
	if(strcmp(arg->type, "float")==0)	return "sizeof(float)";
	if(strcmp(arg->type, "string")==0)	return arg->size;
	return NULL;
}

bool code_check()
{
	return true;
}

bool code_gen_inc(const char* name, FILE* fp)
{
	fprintf(fp,"\n");
	for(int c=0; c<psgen_getcount(); c++) {
		fprintf(fp,"typedef struct %s {\n", psgen_get(c)->name);
		for(int a=0; a<psgen_get(c)->args_count; a++) {
			if(strcmp(psgen_get(c)->args[a].type, "string")==0) {
				fprintf(fp,"	char %s[%s+1];\n", psgen_get(c)->args[a].name, psgen_get(c)->args[a].size);
			} else {
				fprintf(fp,"	%s %s;\n", psgen_get(c)->args[a].type, psgen_get(c)->args[a].name);
			}
		}
		fprintf(fp,"} %s;\n", psgen_get(c)->name);
		fprintf(fp,"\n");
	}
	return true;
}

bool code_gen_inl(const char* name, FILE* fp)
{
	fprintf(fp,"\n");
	for(int c=0; c<psgen_getcount(); c++) {
		fprintf(fp,"class CPropertySet_%s : public CPropertySet<%d>\n", psgen_get(c)->name, psgen_get(c)->args_count);
		fprintf(fp,"{\n");
		fprintf(fp,"public:\n");
		fprintf(fp,"	virtual void InitData() {\n");
		for(int a=0; a<psgen_get(c)->args_count; a++) {
			fprintf(fp,"		SetProperty(%d, \"%s\", %s, offsetof(%s, %s), %s, %s, \"%s\");\n",
				a,
				psgen_get(c)->args[a].name,
				type2type(psgen_get(c)->args[a].type),
				psgen_get(c)->name, psgen_get(c)->args[a].name,
				type2type(psgen_get(c)->args + a),
				psgen_get(c)->args[a].count,
				psgen_get(c)->args[a].desc
				);

		}
		fprintf(fp,"	}\n");
		fprintf(fp,"};\n");
		fprintf(fp,"static CPropertySet_%s ps_%s_%s;\n", psgen_get(c)->name, name, psgen_get(c)->name);
	}
	fprintf(fp,"\n");
	fprintf(fp,"static IPropertySet* %s_PropertySets[] = {\n", name);
	for(int c=0; c<psgen_getcount(); c++) {
		fprintf(fp,"&ps_%s_%s,\n", name, psgen_get(c)->name);
	}
	fprintf(fp,"};\n");
	fprintf(fp,"\n");
	fprintf(fp,"int %s_GetPropertySetCount()\n", name);
	fprintf(fp,"{\n");
	fprintf(fp,"	return sizeof(%s_PropertySets)/sizeof(%s_PropertySets[0]);\n", name, name);
	fprintf(fp,"}\n");
	fprintf(fp,"\n");
	fprintf(fp,"IPropertySet* %s_GetPropertySet(int nIndex)\n", name);
	fprintf(fp,"{\n");
	fprintf(fp,"	if(nIndex<0 || nIndex>=sizeof(%s_PropertySets)/sizeof(%s_PropertySets[0])) return NULL;\n", name, name);
	fprintf(fp,"	return %s_PropertySets[nIndex];\n", name);
	fprintf(fp,"}\n");
	fprintf(fp,"\n");
	fprintf(fp,"IPropertySet* %s_GetPropertySet(const char* pName)\n", name);
	fprintf(fp,"{\n");
	fprintf(fp,"	for(int l=0; l<sizeof(%s_PropertySets)/sizeof(%s_PropertySets[0]); l++) {\n", name, name);
	fprintf(fp,"		if(strcmp(%s_PropertySets[l]->GetName(), pName)==0) return %s_PropertySets[l];\n", name, name);
	fprintf(fp,"	}\n");
	fprintf(fp,"	return NULL;\n");
	fprintf(fp,"}\n");
	return true;
}
