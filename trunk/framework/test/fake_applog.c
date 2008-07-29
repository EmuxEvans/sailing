#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/log.h"
#include "../inc/applog.h"

static const char* loglevel_strs[] = {"ERROR", "WARNING", "INFO", "DEBUG",};

#ifndef	SYSLOG_DISABLE

static int syslog_enable_t = 1;

int syslog_open(const char* url)
{
	return ERR_NOERROR;
}

void syslog_enable(int enable)
{
	syslog_enable_t = enable;
}

int syslog_close()
{
	return ERR_NOERROR;
}

void syslog(int level, const char* src, const char* fmt, ...)
{
	time_t timet;
	struct tm time_tm;
	char buf[10240];
	int count;
	va_list valist;

	if(!syslog_enable_t) return;

	time(&timet);
	memcpy(&time_tm, localtime(&timet), sizeof(time_tm));

	sprintf(buf, "[%7s] [%15s] %02d-%02d %02d:%02d:%02d ", 
			loglevel_strs[level], src,
//			time_tm.tm_year+1900,
			time_tm.tm_mon+1, time_tm.tm_mday, 
			time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec
			);
	count = (int)strlen(buf);

	va_start(valist, fmt);
	count += vsnprintf(buf+count, sizeof(buf)-count, fmt, valist);
	va_end(valist);

	printf("%s\n", buf);
}

#endif

#ifndef DBGLOG_DISABLE

static int dbglog_enable_t = 1;

int dbglog_open(const char* url)
{
	return ERR_NOERROR;
}

void dbglog_enable(int enable)
{
	dbglog_enable_t = enable;
}

int dbglog_close()
{
	return ERR_NOERROR;
}

void dbglog(int level, const char* src, const char* fmt, ...)
{
	time_t timet;
	struct tm time_tm;
	char buf[2048];
	int count;
	va_list valist;

	if(!dbglog_enable_t) return;

	time(&timet);
	memcpy(&time_tm, localtime(&timet), sizeof(time_tm));

	sprintf(buf, "[%7s] [%15s] %02d-%02d %02d:%02d:%02d ", 
			loglevel_strs[level], src,
//			time_tm.tm_year+1900,
			time_tm.tm_mon+1, time_tm.tm_mday, 
			time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec
			);
	count = (int)strlen(buf);

	va_start(valist, fmt);
	count += vsnprintf(buf+count, sizeof(buf)-count, fmt, valist);
	va_end(valist);

	printf("%s\n", buf);
}

#endif
