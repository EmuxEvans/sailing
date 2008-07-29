%{

#include <stdio.h>
#include <stdlib.h>

#include "idl_cc.h"

int num_tab = 0;
int l;

int yylex();

#define OUTPUT_DEBUG

%}

%token KW_IN
%token KW_OUT
%token KW_CDEF

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
  | function_define { $$=$1; }
  | error {
        fprintf(stderr, "definition : error %d\n", lineno);
		exit(-1);
        $$=0;
    }
    ;

cdef_define:
	KW_CDEF {
		put_include($1);
		$$=$1;
	}
    ;

function_define:
    KW_ID  KW_L_KH KW_R_KH KW_FH {
		printf("function %s\n", $1);
		put_function($1, NULL);
		$$=$1;
	}
  | KW_ID KW_L_KH parameter_list KW_R_KH KW_FH {
		printf("function %s\n", $1);
		put_function($1, NULL);
        $$=$1;
    }
  | KW_L_FKH KW_ID KW_R_FKH KW_ID  KW_L_KH KW_R_KH KW_FH {
		printf("function %s\n", $4);
		put_function($4, $2);
		$$=$1;
	}
  | KW_L_FKH KW_ID KW_R_FKH KW_ID KW_L_KH parameter_list KW_R_KH KW_FH {
		printf("function %s\n", $4);
		put_function($4, $2);
        $$=$1;
    }
  | error {
		fprintf(stderr, "function_define error %d\n", lineno);
		exit(-1);
		$$=0;
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
    value_type   { $$=$1; }
  | ptr_type     { $$=$1; }
  | error {
        fprintf(stderr, "parameter error %d\n", lineno);
		exit(-1);
		$$=0;
    }
    ;

value_type:
    // [IN] TYPE NAME
    KW_L_FKH KW_IN KW_R_FKH KW_ID KW_ID { // [IN] type name
        PUT_VAL($4, $5);
        $$=$1;
    }
  | error {
		fprintf(stderr, "value_type error %d\n", lineno);
		exit(-1);
        $$=0;
	}
    ;

ptr_type:
    // [IN(MAX_LEN)] TYPE NAME[]
    KW_L_FKH KW_IN KW_L_KH KW_STR_CON KW_R_KH KW_R_FKH KW_ID KW_ID KW_L_FKH KW_R_FKH {
		PUT_PTR_IN($7, $8, $4, IDL_FLAG_PTR, NULL);
        $$=$1;
    }
    // [IN] TYPE NAME[MAX_LEN]
  | KW_L_FKH KW_IN KW_R_FKH KW_ID KW_ID KW_L_FKH KW_STR_CON KW_R_FKH {
		PUT_PTR_IN($4, $5, $7, IDL_FLAG_PTR, NULL);
        $$=$1;
    }
    // [OUT] TYPE NAME[MAX_LEN];
  | KW_L_FKH KW_OUT KW_R_FKH KW_ID KW_ID KW_L_FKH KW_STR_CON KW_R_FKH {
        PUT_PTR_OUT($4, $5, $7, IDL_FLAG_PTR, NULL);
        $$=$1;
    }
    // [IN OUT] TYPE NAME[MAX_LEN]
  | KW_L_FKH KW_IN KW_OUT KW_R_FKH KW_ID KW_ID KW_L_FKH KW_STR_CON KW_R_FKH {
        PUT_PTR_INOUT($5, $6, $8, IDL_FLAG_PTR, IDL_FLAG_PTR, NULL, NULL);
        $$=$1;
    }
    // [IN(IN_LEN) OUT] TYPE NAME[MAX_LEN]
  | KW_L_FKH KW_IN KW_L_KH KW_STR_CON KW_R_KH KW_OUT KW_R_FKH KW_ID KW_ID KW_L_FKH KW_STR_CON KW_R_FKH {
        PUT_PTR_INOUT($8, $9, $11, IDL_FLAG_VAL, IDL_FLAG_PTR, $4, NULL);
        $$=$1;
    }
    // [OUT(MAX_LEN)] TYPE NAME[MAX_LEN]
  | KW_L_FKH KW_OUT KW_L_KH KW_STR_CON KW_R_KH KW_R_FKH KW_ID KW_ID KW_L_FKH KW_STR_CON KW_R_FKH {
        PUT_PTR_OUT($7, $8, $10, IDL_FLAG_VAL, $4);
        $$=$1;
    }
    // [IN OUT(MAX_LEN)] TYPE NAME[MAX_LEN]
  | KW_L_FKH KW_IN KW_OUT KW_L_KH KW_STR_CON KW_R_KH KW_R_FKH KW_ID KW_ID KW_L_FKH KW_STR_CON KW_R_FKH {
        PUT_PTR_INOUT($8, $9, $11, IDL_FLAG_PTR, IDL_FLAG_VAL, NULL, $5);
        $$=$1;
    }
    // [IN(IN_LEN) OUT(OUT_LEN)] TYPE NAME[MAX_LEN]
  | KW_L_FKH KW_IN KW_L_KH KW_STR_CON KW_R_KH KW_OUT KW_L_KH KW_STR_CON KW_R_KH KW_R_FKH KW_ID KW_ID KW_L_FKH KW_STR_CON KW_R_FKH {
        PUT_PTR_INOUT($11, $12, $14, IDL_FLAG_VAL, IDL_FLAG_VAL, $4, $8);
        $$=$1;
    }
    ;

%%

