%{
#include <stdio.h>
#include <string.h>
#include "idl_cc.h"

#include "idl.tab.h"

//#define YY_NEVER_INTERACTIVE 1
#define YY_NO_UNPUT

#include <string.h>
#include <stdlib.h>

#define OUTPUT_DEBUG

string my_strdup(string v)
{
    string ret;
    ret = (string)malloc(strlen(v)+1);
    strcpy(ret, v);
    return ret;
}

void string_constant();
void eat_comment();
void include_read();

//%option yylineno
%}

%option noyywrap

str             \"[^\"]*\"
idstart         [A-Za-z"_"]
idchar          [A-Za-z"_"0-9]
id              {idstart}{idchar}*
whitespace      [" "\t]+
return          [\n]

%%

"IN"		{ return KW_IN; }
"OUT"		{ return KW_OUT; }
"C_DEF"		{ include_read(); return KW_CDEF; }
"!"			{ include_read(); return KW_CDEF; }

"//"		{ eat_comment(); }

"("             { return KW_L_KH; }
")"             { return KW_R_KH; }
"["             { return KW_L_FKH; }
"]"             { return KW_R_FKH; }
","             { return KW_DH; }
";"             { return KW_FH; }

{str}           { string_constant(); return KW_STR_CON; }
{id}            { yylval = my_strdup(yytext); return KW_ID; }
{whitespace}    { }
{return}        { lineno++; }

.               { }

%%

void string_constant()
{
    int l = strlen(yytext)-2;
    yylval = malloc(l+1);
    strncpy(yylval, &yytext[1], l);
    yylval[l] = 0;
}

void eat_comment()
{
    char c;
    while ((c = input()) != '\n' && c != 0);
}

void include_read()
{
    char filename[100];
	int l;
	for(l=0; l<sizeof(filename); l++) {
		filename[l] = input();
		if(filename[l]=='\n' || filename[l]=='\0') {
			filename[l] = '\0';
			while(filename[0]==' ' || filename[0]=='\t') {
				memmove(filename, filename+1, strlen(filename+1)+1);
			}
			for(l=strlen(filename)-1; l>=0; l++) {
				if(filename[l]==' ' || filename[0]=='\t') {
					filename[l] = '\0';
				} else {
					break;
				}
			}
			yylval = strdup(filename);
			return;
		}
	}
	printf("%d line too long\n", lineno);
}

