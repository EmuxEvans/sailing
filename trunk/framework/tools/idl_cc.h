#ifndef _IDL_CC_H_
#define _IDL_CC_H_

#define YYSTYPE_IS_DECLARED
typedef char * string;
#define YYSTYPE string

#define IDL_MODE_VAL			1
#define IDL_MODE_PTR			2

#define IDL_FLAG_NONE			3
#define IDL_FLAG_PTR			4
#define IDL_FLAG_VAL			5

#define PUT_VAL(type, name) put_parameter(IDL_MODE_VAL, IDL_FLAG_NONE, IDL_FLAG_NONE, NULL, NULL, NULL, type, name)

#define PUT_PTR_IN(type, name, ptr_len, f_in, in_len) put_parameter(IDL_MODE_PTR, f_in, IDL_FLAG_NONE, ptr_len, in_len, NULL, type, name)

#define PUT_PTR_OUT(type, name, ptr_len, f_out, out_len) put_parameter(IDL_MODE_PTR, IDL_FLAG_NONE, f_out, ptr_len, NULL, out_len, type, name)

#define PUT_PTR_INOUT(type, name, ptr_len, f_in, f_out, in_len, out_len) put_parameter(IDL_MODE_PTR, f_in, f_out, ptr_len, in_len, out_len, type, name)

void put_include(const char* filename);
void put_parameter(int mode, int f_in, int f_out, char* ptr_len, char* in_len, char* out_len, char* type, char* name);
void put_function(const char* name, const char* mode);

void yyerror(char * s);

extern int lineno;

#define STR_MAX_LEN         100
#define INC_MAX_COUNT		100
#define PAR_MAX_COUNT       30
#define FUN_MAX_COUNT       300

typedef struct PARAMETER {
    int flag_mode;
    int flag_in;
    int flag_out;

    char ptr_len[STR_MAX_LEN];
    char in_len[STR_MAX_LEN];
    char out_len[STR_MAX_LEN];

    char type[STR_MAX_LEN];
    char name[STR_MAX_LEN];
} PARAMETER;

typedef struct FUNCTION {
    char name[STR_MAX_LEN];
    char mode[STR_MAX_LEN];
    int p_count;
    PARAMETER p_list[PAR_MAX_COUNT];
} FUNCTION;

extern char i_list[INC_MAX_COUNT][STR_MAX_LEN];
extern int i_count;
extern FUNCTION f_list[FUN_MAX_COUNT];
extern int f_count;

int process();

#endif

