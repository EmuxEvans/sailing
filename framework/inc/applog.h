#ifndef _APPLOG_H_
#define _APPLOG_H_

#define LOG_ERROR				0
#define LOG_WARNING				1
#define LOG_INFO				2
#define LOG_DEBUG				3

#ifndef	SYSLOG_DISABLE
	#define SYSLOG	syslog
#else
	#define SYSLOG(...)
#endif
#ifndef	DBGLOG_DISABLE
	#define DBGLOG	dbglog
#else
	#define DBGLOG(...)
#endif

#ifndef	SYSLOG_DISABLE
ZION_API int syslog_open(const char* url);
ZION_API void syslog_enable(int enable);
ZION_API int syslog_close();
ZION_API void syslog(int level, const char* src, const char* fmt, ...);
#endif

#ifndef DBGLOG_DISABLE
ZION_API int dbglog_open(const char* url);
ZION_API void dbglog_enable(int enable);
ZION_API int dbglog_close();
ZION_API void dbglog(int level, const char* src, const char* fmt, ...);
#endif

#endif

