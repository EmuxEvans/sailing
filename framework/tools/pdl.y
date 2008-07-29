%{

#include <stdio.h>
#include <stdlib.h>

#include "pdl_cc.h"

int num_tab = 0;
int l;

int yylex();

#define OUTPUT_DEBUG

%}

%token KW_CMD_CLT
%token KW_CMD_SVR
%token KW_FILTER
%token KW_CDEF
%token KW_STRING
%token KW_NUMBER

%token KW_ID

%token KW_L_KH
%token KW_R_KH
%token KW_L_FKH
%token KW_R_FKH
%token KW_DH
%token KW_FH

%token KW_STR_CON

%%

start:
    /*empty*/ { $$=0; }
  | definition_plus {
        $$=$1;
    }
    ;

definition_plus:
    definition { $$=$1; }
  | definition_plus definition {
        $$=$1;
    }
  | error {
		fprintf(stderr, "definition_plus error %d\n", lineno);
		exit(-1);
		$$=0;
	}
    ;

definition:
//    /*empty*/ { $$=0; }
    cdef_define { $$=$1; }
  | filter_define { $$=$1; }
  | command_define { $$=$1; }
  | error {
        fprintf(stderr, "definition : error %d\n", lineno);
		exit(-1);
        $$=0;
    }
    ;

cdef_define:
	KW_CDEF {
		add_include($1);
		$$=$1;
	}
	;

filter_define:
	KW_FILTER KW_NUMBER KW_ID KW_FH {
		add_filter($2, $3);
		$$=$1;
	}
  |	KW_FILTER KW_ID KW_ID KW_FH {
		add_filter($2, $3);
		$$=$1;
	}
	;

command_define:
    KW_CMD_SVR KW_ID KW_L_KH KW_R_KH KW_FH {
		printf("CMD_SVR %s\n", $2);
		add_command($2, "CMD_SVR");
		$$=$1;
	}
  | KW_CMD_SVR KW_ID KW_L_KH parameter_list KW_R_KH KW_FH {
		printf("CMD_SVR %s\n", $2);
		add_command($2, "CMD_SVR");
        $$=$1;
    }
  | KW_CMD_CLT KW_ID KW_L_KH KW_R_KH KW_FH {
		printf("CMD_CLT %s\n", $2);
		add_command($2, "CMD_CLT");
		$$=$1;
	}
  | KW_CMD_CLT KW_ID KW_L_KH parameter_list KW_R_KH KW_FH {
		printf("CMD_CLT %s\n", $2);
		add_command($2, "CMD_CLT");
        $$=$1;
    }
    ;

parameter_list:
    parameter { $$=$1; }
  | parameter_list KW_DH parameter { $$=$1; }
  | error {
		fprintf(stderr, "parameter_list error %d\n", lineno);
		exit(-1);
        $$=0;
	}
	;

parameter:
  | string_type		{ $$=$1; }
  | string_rule_type{ $$=$1; }
  | value_type		{ $$=$1; }
  | ptr_type		{ $$=$1; }
  | ptr_rule_type	{ $$=$1; }
  | error {
        fprintf(stderr, "parameter error %d\n", lineno);
		exit(-1);
		$$=0;
    }
    ;

value_type:
    // TYPE NAME
    KW_ID KW_ID { // [IN] type name
		add_parameter($2, $1, NULL, NULL);
        $$=$1;
    }
	;

ptr_type:
    // TYPE NAME[MAX_LEN]
    KW_ID KW_ID KW_L_FKH KW_STR_CON KW_R_FKH {
		add_parameter($2, $1, $4, NULL);
        $$=$1;
    }
    ;

ptr_rule_type:
    // TYPE["rule"] NAME[MAX_LEN]
    KW_ID KW_L_FKH KW_STR_CON KW_R_FKH KW_ID KW_L_FKH KW_STR_CON KW_R_FKH {
		add_parameter($5, $1, $7, $3);
        $$=$1;
    }
    ;

string_type:
	// string NAME[]
	KW_STRING KW_ID KW_L_FKH KW_R_FKH {
		add_parameter($2, "string", NULL, NULL);
	}
	;

string_rule_type:
	// string["rule"] name[]
	KW_STRING KW_L_FKH KW_STR_CON KW_R_FKH KW_ID KW_L_FKH KW_R_FKH {
		add_parameter($5, "string", NULL, $3);
	}
	;

%%

