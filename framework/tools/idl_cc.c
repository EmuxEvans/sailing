#include <stdio.h>
#include <string.h>

#include "idl_cc.h"
#include "idl.tab.h"
#include "idl_codegen.h"

FUNCTION f_list[FUN_MAX_COUNT];
char i_list[INC_MAX_COUNT][STR_MAX_LEN];
int i_count = 0, f_count = 0;
int lineno = 1;

int yyparse();

int process()
{
    int f, p;

    for(f=0; f<f_count; f++) {
        printf("%s p_count=%d {\n", f_list[f].name, f_list[f].p_count);
        for(p=0; p<f_list[f].p_count; p++) {
	        printf("name=%s type=%s mode(%d,%d,%d) ptr_len=%s in_len=%s out_len=%s\n",
                f_list[f].p_list[p].name,
                f_list[f].p_list[p].type,
                f_list[f].p_list[p].flag_mode,
                f_list[f].p_list[p].flag_in,
                f_list[f].p_list[p].flag_out,
                f_list[f].p_list[p].ptr_len,
                f_list[f].p_list[p].in_len,
                f_list[f].p_list[p].out_len
            );
        }
	    printf("} %s; \n", f_list[f].name);
    }
    return(0);
}

extern FILE *yyin, *yyout;

int main(int argc, char* argv[])
{
    int l;
	char str[200], * path, * filename, * dot, * slash;
    
    for(l=0; l<FUN_MAX_COUNT; l++) f_list[l].p_count = 0;

	if(argc!=2) {
		printf("invalid parameter!!!!\n");
		return(-1);
	}

	strcpy(str, argv[1]);
	dot = strrchr(str, '.');
	slash = strrchr(str, '/');
	if(slash==NULL) slash = strrchr(str, '\\');
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

	yyin = fopen(argv[1], "r");
	if(yyin==NULL) {
		printf("open %s failed!!!!\n", argv[1]);
		return(-1);
	}

	printf("%s\n", filename);

    if(yyparse()!=0) return(-1);
//   	if(process()!=0) return(-1);
   	if(gencode(path, filename)!=0) return(-1);
    return(0);
}

void put_include(const char* filename)
{
	strcpy(&i_list[i_count][0], filename);
	i_count++;
}

void put_parameter(int mode, int f_in, int f_out, char* ptr_len, char* in_len, char* out_len, char* type, char* name)
{
    f_list[f_count].p_list[f_list[f_count].p_count].flag_mode = mode;
    f_list[f_count].p_list[f_list[f_count].p_count].flag_in = f_in;
    f_list[f_count].p_list[f_list[f_count].p_count].flag_out = f_out;

    strcpy(f_list[f_count].p_list[f_list[f_count].p_count].ptr_len, ptr_len==NULL?"":ptr_len);
    strcpy(f_list[f_count].p_list[f_list[f_count].p_count].in_len, in_len==NULL?"":in_len);
    strcpy(f_list[f_count].p_list[f_list[f_count].p_count].out_len, out_len==NULL?"":out_len);

    strcpy(f_list[f_count].p_list[f_list[f_count].p_count].type, type);
    strcpy(f_list[f_count].p_list[f_list[f_count].p_count].name, name);

    f_list[f_count].p_count++;
}

void put_function(const char* name, const char* mode)
{
	strcpy(f_list[f_count].name, name);
	if(mode!=NULL) {
		strcpy(f_list[f_count].mode, mode);
	} else {
		strcpy(f_list[f_count].mode, "");
	}
	f_count++;
}

void yyerror(char* s)
{
    fprintf(stderr, "ERROR = %d %s\n", lineno, s);
}

