#ifndef _APPBOX_H_
#define _APPBOX_H_

#define ENTRY_REASON_ATTACH 	1 
#define ENTRY_REASON_DETACH 	2

typedef struct APPBOX_SETTING
{
	char*	name;
	int		type;
	void*	ptr;
	unsigned int* len;
	unsigned int maxlen;
} APPBOX_SETTING;

#define CONFIGDATA_UNKNOWN				0
#define CONFIGDATA_INTEGER				1
#define CONFIGDATA_FLOAT				2
#define CONFIGDATA_STRING				3
#define CONFIGDATA_BINARY				4
#define CONFIGDATA_ENDPOINT				5

#define APPBOX_SETTING_BEGIN(name)						\
	APPBOX_SETTING name [] = {
#define APPBOX_SETTING_INTEGER(name, value)				\
	{ name, CONFIGDATA_INTEGER, (void*)&value, NULL, 0 },
#define APPBOX_SETTING_FLOAT(name, value)				\
	{ name, CONFIGDATA_FLOAT, (void*)&value, NULL, 0 },
#define APPBOX_SETTING_STRING(name, value, maxlen)		\
	{ name, CONFIGDATA_STRING, (void*)value, NULL, maxlen },
#define APPBOX_SETTING_BINARY(name, value, len, maxlen)	\
	{ name, CONFIGDATA_BINARY, (void*)value, &len, maxlen },
#define APPBOX_SETTING_ENDPOINT(name, value)	\
	{ name, CONFIGDATA_ENDPOINT, (void*)&value, NULL, 0 },
#define APPBOX_SETTING_END(name)						\
	{ NULL, 0, NULL, NULL, 0 } };

ZION_API int appbox_config_load(const char* filename);
ZION_API int appbox_config_init();
ZION_API int appbox_config_get(const char* module_name, APPBOX_SETTING* tab);

ZION_API int appbox_init();
ZION_API int appbox_final();
ZION_API int appbox_load_modules();
ZION_API int appbox_unload_modules();

ZION_API int appbox_reg_command(const char* module, const char* name, CONSOLE_CALLBACK callback);
ZION_API int appbox_unreg_command(const char* module, const char* name, CONSOLE_CALLBACK callback);

#endif
