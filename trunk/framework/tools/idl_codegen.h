#ifndef _IDL_CODEGEN_H_
#define _IDL_CODEGEN_H_

#include "idl_cc.h"

typedef struct runtime_param
{
	void* ptr;
	int typesize;
	int maxlen;
	int in_len;
	int out_len;
}runtime_param;

int gencode(const char* path, const char* filename);

#endif
