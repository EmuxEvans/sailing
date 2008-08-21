#ifndef _LOG_H_
#define _LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_ERROR				0
#define LOG_WARNING				1
#define LOG_INFO				2
#define LOG_DEBUG				3

struct LOG_CTX;
typedef struct LOG_CTX* LOG_HANDLE;

ZION_API LOG_HANDLE log_open(const char* path);
ZION_API int log_close(LOG_HANDLE handle);

ZION_API int log_write(LOG_HANDLE handle, int level, const char* fmt, ...);
ZION_API int log_puts(LOG_HANDLE handle, int level, const char* msg, unsigned int msglen);

#ifdef __cplusplus
}
#endif

#endif

