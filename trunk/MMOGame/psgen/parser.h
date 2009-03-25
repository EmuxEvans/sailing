#pragma once

typedef struct PDL_ARG {
	char name[100];
	char type[100];
	char size[100];
	char count[100];
	char desc[500];
} PDL_ARG;

typedef struct PDL_CMD {
	char name[100];
	char desc[500];
	PDL_ARG* args;
	int args_count;
} PDL_CMD;

extern bool psgen_parser(const char* text);

extern int psgen_getcount();
extern const PDL_CMD* psgen_get(int index);
