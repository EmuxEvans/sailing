#include <stdio.h>
#include "pdl.tab.h"

int yylex();

int lineno;

YYSTYPE yylval;

int main(int argc, char* argv[])

{

	yylex(); /* start the analysis*/

//	printf(" No of words: %d\n", wordCount);

	return 0;
}

int yywrap()
{
	return 1;
}

