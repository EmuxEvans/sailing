#ifndef _CONSOLE_INCLUDE_
#define _CONSOLE_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

#define CONSOLE_HOOKERNAME_LEN			30
#define CONSOLE_ITEM_NAME_LEN			20
#define CONSOLE_ITEM_TYPE_LEN			15
#define CONSOLE_ITEM_ADDR_LEN			40

struct CONSOLE_INSTANCE;
typedef struct CONSOLE_INSTANCE CONSOLE_INSTANCE;

struct CONSOLE_CONNECTION;
typedef struct CONSOLE_CONNECTION CONSOLE_CONNECTION;

typedef int (*CONSOLE_CALLBACK)(CONSOLE_CONNECTION* conn, const char* name, const char* line);

ZION_API CONSOLE_INSTANCE* console_create(SOCK_ADDR* sa, unsigned int maxconns, unsigned int maxhooker);
ZION_API CONSOLE_INSTANCE* console_create_csmng(SOCK_ADDR* sa, unsigned int maxconns, unsigned int maxhooker, unsigned int maxitems, const char* config_path);
ZION_API int console_destroy(CONSOLE_INSTANCE* instance);

ZION_API int console_hook(CONSOLE_INSTANCE* instance, const char* name, CONSOLE_CALLBACK func);
ZION_API int console_unhook(CONSOLE_INSTANCE* instance, const char* name, CONSOLE_CALLBACK func);

ZION_API int console_puts(CONSOLE_CONNECTION* conn, int code, const char* str);
ZION_API int console_print(CONSOLE_CONNECTION* conn, int code, const char* fmt, ...);

ZION_API const SOCK_ADDR* console_peername(CONSOLE_CONNECTION* conn);
ZION_API const char* console_peername_str(CONSOLE_CONNECTION* conn);

#ifdef __cplusplus
}
#endif

#endif
