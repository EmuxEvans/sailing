#include <stdio.h>
#include <string.h>

#include "pdl_cc.h"
#include "pdl_codegen.h"
#include "pdl.tab.h"

int lineno;
INCLUDE i_list[100];
FILTER f_list[30];

int i_count, f_count;

int yyparse();

char client_mode[100] = "";

extern FILE *yyin, *yyout;

#define FILTER_MAX_COUNT		(sizeof(f_list)/sizeof(f_list[0]))
#define INCLUDE_MAX_COUNT		(sizeof(i_list)/sizeof(i_list[0]))
#define COMMAND_MAX_COUNT		(sizeof(f_list[0].c_list)/sizeof(f_list[0].c_list[0]))
#define PARAMETER_MAX_COUNT		(sizeof(f_list[0].c_list[0].p_list)/sizeof(f_list[0].c_list[0].p_list[0]))

int main(int argc, char* argv[])
{
	char str[200], * path, * filename, * dot, * slash;

	memset(i_list, 0, sizeof(i_list));
	memset(f_list, 0, sizeof(f_list));
	i_count = f_count = 0;

	printf("PDL_CC %s\n", __TIME__);
    
	if(argc!=4) {
		printf("invalid parameter!!!!\n");
		return -1;
	}

	if(strcmp(argv[1], "CMD_CLT")!=0 && strcmp(argv[1], "CMD_SVR")!=0) {
		printf("invalid parameter!!!!\n");
		return -1;
	}
	strcpy(client_mode, argv[1]);

	strcpy(str, argv[3]);
	dot = strrchr(str, '.');
	slash = strrchr(str, '/');
	if(dot==NULL || (slash!=NULL && dot<=slash)) {
		printf("invalid file name\n");
		return(-1);
	}
	*dot = '\0';
	if(slash!=NULL) {
		*slash = '\0';
		path = str;
		filename = slash+1;
	} else {
		path = ".";
		filename = str;
	}

	yyin = fopen(argv[3], "r");
	if(yyin==NULL) {
		printf("open %s failed!!!!\n", argv[1]);
		return(-1);
	}

	if(yyparse()!=0) return(-1);
	if(gencode(path, filename, argv[2])!=0) return(-1);
	return(0);
}

void add_filter(char* id, char* name)
{
	strcpy(f_list[f_count].id, id);
	strcpy(f_list[f_count].name, name);
	f_count++;
}

void add_include(const char* filename)
{
	strcpy(i_list[i_count].filename, filename);
	i_count++;
}

void add_command(char* name, char* mode)
{
	FILTER* f;
	COMMAND* c;
	f = f_list + f_count - 1;
	c = f->c_list + f->c_count;
	strcpy(c->name, name);
	strcpy(c->mode, mode);
	f->c_count++;
}

void add_parameter(char* name, char* type, char* len, char* rule)
{
	FILTER* f;
	COMMAND* c;
	PARAMETER* p;
	f = f_list + f_count - 1;
	c = f->c_list + f->c_count;
	p = c->p_list + c->p_count;
	strcpy(p->name, name);
	strcpy(p->type, type);
	if(len!=NULL) {
		strcpy(p->len , len);
	} else {
		strcpy(p->len , "");
	}
	if(rule!=NULL) {
		strcpy(p->rule, rule);
	} else {
		strcpy(p->rule, "");
	}

	if(strcmp(type, "string")==0) {
		sprintf(p->len, "strlen(%s)+1", name);
	}

	c->p_count++;
}

void yyerror(char* s)
{
    fprintf(stderr, "ERROR = %d %s\n", lineno, s);
}
