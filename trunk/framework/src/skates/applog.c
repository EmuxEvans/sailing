#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/log.h"
#include "../../inc/skates/applog.h"
#include "../../inc/skates/threadpool.h"

static const char* loglevel_strs[] = {"ERROR", "WARNING", "INFO", "DEBUG",};

#ifndef	SYSLOG_DISABLE

static LOG_HANDLE syslog_handle = NULL;
static int syslog_enable_t = 1;

int syslog_open(const char* url)
{
	syslog_handle = log_open(url);
	return syslog_handle!=NULL?ERR_NOERROR:ERR_UNKNOWN;
}

void syslog_enable(int enable)
{
	syslog_enable_t = enable;
}

int syslog_close()
{
	return log_close(syslog_handle);
}

void syslog_write(int level, const char* src, const char* fmt, ...)
{
	time_t timet;
	struct tm time_tm;
	char buf[10240];
	int count;
	va_list valist;

	if(!syslog_enable_t || syslog_handle==NULL) return;

	time(&timet);
	memcpy(&time_tm, localtime(&timet), sizeof(time_tm));

	sprintf(buf, "[%7s] [%15s] %02d-%02d %02d:%02d:%02d (%d) ", 
			loglevel_strs[level], src,
//			time_tm.tm_year+1900,
			time_tm.tm_mon+1, time_tm.tm_mday, 
			time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec,
			threadpool_getindex()
			);
	count = (int)strlen(buf);

	va_start(valist, fmt);
	count += vsnprintf(buf+count, sizeof(buf)-count, fmt, valist);
	va_end(valist);

	log_puts(syslog_handle, level, buf, count);
}

#endif

#ifndef DBGLOG_DISABLE

static LOG_HANDLE dbglog_handle = NULL;
static int dbglog_enable_t = 1;

int dbglog_open(const char* url)
{
	dbglog_handle = log_open(url);
	return dbglog_handle!=NULL?ERR_NOERROR:ERR_UNKNOWN;
}

void dbglog_enable(int enable)
{
	dbglog_enable_t = enable;
}

int dbglog_close()
{
	return log_close(dbglog_handle);
}

void dbglog_write(int level, const char* src, const char* fmt, ...)
{
	time_t timet;
	struct tm time_tm;
	char buf[2048];
	int count;
	va_list valist;

	if(!dbglog_enable_t || dbglog_handle==NULL) return;

	time(&timet);
	memcpy(&time_tm, localtime(&timet), sizeof(time_tm));

	sprintf(buf, "[%7s] [%15s] %02d-%02d %02d:%02d:%02d (%d) ", 
			loglevel_strs[level], src,
//			time_tm.tm_year+1900,
			time_tm.tm_mon+1, time_tm.tm_mday, 
			time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec,
			threadpool_getindex()
			);
	count = (int)strlen(buf);

	va_start(valist, fmt);
	count += vsnprintf(buf+count, sizeof(buf)-count, fmt, valist);
	va_end(valist);

	log_puts(dbglog_handle, level, buf, (unsigned int)count);
}

#endif
