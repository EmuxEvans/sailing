#ifndef _PDL_CC_H_
#define _PDL_CC_H_

typedef struct PARAMETER {
	char name[100];
	char type[100];
	char len[100];
	char rule[100];
} PARAMETER;

typedef struct COMMAND {
	char name[100];
	char mode[100];
	PARAMETER p_list[10];
	int p_count;
} COMMAND;

typedef struct FILTER {
	char name[100];
	char id[100];

	COMMAND c_list[100];
	int c_count;
} FILTER;

typedef struct INCLUDE {
	char filename[100];
} INCLUDE;

void add_filter(char* id, char* name);
void add_include(const char* filename);
void add_command(char* name, char* mode);
void add_parameter(char* name, char* type, char* len, char* rule);

void yyerror(char * s);

extern int lineno;
extern INCLUDE i_list[100];
extern FILTER f_list[30];
extern int i_count, f_count;


#define YYSTYPE_IS_DECLARED
typedef char * string;
#define YYSTYPE string

#endif

