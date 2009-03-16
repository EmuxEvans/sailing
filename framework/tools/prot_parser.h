#ifndef __PROT_PARSER_INCLUDE__
#define __PROT_PARSER_INCLUDE__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DATA_INCLUDE {
	char file[100];
} DATA_INCLUDE;

typedef struct DATA_CONST {
	char type[100];
	char name[100];
	char value[100];
} DATA_CONST;

typedef struct DATA_VARIABLE {
	char mode[100];
	char type[100];
	char prefix[100];
	char name[100];
	char maxlen[100];
	char value[1000];
} DATA_VARIABLE;

typedef struct DATA_TYPE {
	int is_root;
	char mode[100];
	char name[100];
	int  var_start;
	int  var_count;
} DATA_TYPE;

extern DATA_INCLUDE data_include[100];
extern DATA_CONST data_const[1000];
extern DATA_VARIABLE data_variable[5000];
extern DATA_TYPE data_type[1000];
extern int num_inc, num_const, num_var, num_type;

typedef struct DATA_PARAMETER {
	char name[100];
	char type[100];
} DATA_PARAMETER;
typedef struct DATA_FUNCTION {
	char return_type[100];
	char name[100];
	int class_index;
	int parameter_start;
	int parameter_count;
} DATA_FUNCTION;
typedef struct DATA_CLASS {
	int is_root;
	char name[100];
	int function_start;
	int function_count;
	char root[100];
} DATA_CLASS;
extern DATA_PARAMETER data_parameter[100];
extern DATA_FUNCTION data_function[100];
extern DATA_CLASS data_class[100];
extern int num_parm, num_func, num_class;
extern int current_class;

extern int is_break;

typedef struct PARSER_STACK {
	char	o_path[100];
	char	path[100];
	char	file[100];
	char	txt[50*1024];
} PARSER_STACK;
extern PARSER_STACK p_stack[10];
extern int p_stack_depth;

extern int parse_file(const char* file);
extern int parse_pfile(const char* text);

extern const char* get_token_char(const char* buf, char c);
extern const char* get_token_string(const char* buf, char* value, int size);
extern const char* get_token_id(const char* buf, char* value, int size);
extern const char* get_token_keyword(const char* buf, const char* keyword, char* value);
extern const char* get_token_number(const char* buf, char* value, int size);
extern const char* escape_blank(const char* buf);

extern int get_basetype(const char* type);
extern const char* get_type_const(const char* type);
extern const char* get_ctype(const char* type);
extern int check_include_filename(const char* file);
extern void make_define_filename(const char* file, char* out);
extern void make_include_filename(const char* file, char* out);
extern int check_vailid_dtype(const char* type);
extern int check_vailid_otype(const char* type);

#ifdef __cplusplus
}
#endif

#endif
