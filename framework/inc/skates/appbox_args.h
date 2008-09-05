#ifndef _APPBOX_ARGS_H_
#define _APPBOX_ARGS_H_

ZION_API int appbox_args_parse(int argc, char* argv[]);
ZION_API const char* appbox_args_get(const char* name, const char* defvalue);

#endif

