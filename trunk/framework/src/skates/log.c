#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/log.h"

// STATIC CONFIG : Start
#define FILE_BUFFER_SIZE		2*1024*1024	
// STATIC CONFIG : End

#define FILE_MODE_SINGLE		0
#define FILE_MODE_ONCE			1
#define FILE_MODE_EVERYDAY		2
#define FILE_MODE_EVERYWEEK		3
#define FILE_MODE_EVERYMOUTH	4

typedef struct FILE_STREAM_CTX {
	FILE*			fp;
	char			path[200];
	char			name[32];
	char			filename[300];
	int				mode;
	time_t			covertime;
} FILE_STREAM_CTX;

typedef struct CONSOLE_STREAM_CTX {
	int				fd;
} CONSOLE_STREAM_CTX;

typedef struct LOG_CTX {
	struct {
		char*					buf;
		unsigned int			cur;
		unsigned int			len;
		unsigned int			max;
		os_thread_t				thread;
		os_sem_t				inque;
		os_sem_t				ouque;
		os_mutex_t				mtx;
		int						inque_size;
		int						ouque_size;
	} stream;

	int							type;
	union {
		void*					ptr;
		FILE_STREAM_CTX*		file;
		CONSOLE_STREAM_CTX*		console;
	};
} LOG_CTX;

static unsigned int ZION_CALLBACK log_thread(void* arg);
static int stream_write(LOG_CTX* ctx, const char* msg, unsigned int len);
static int stream_read(LOG_CTX* ctx, char* msg, unsigned int len);
static const char* get_level_string(int level);

static int file_open(LOG_CTX* ctx, const char* url);
static int file_close(LOG_CTX* ctx);
static int file_write(LOG_CTX* ctx, int level, const char* msg, unsigned int len);

static int con_open(LOG_CTX* ctx, const char* url);
static int con_close(LOG_CTX* ctx);
static int con_write(LOG_CTX* ctx, int level, const char* msg, unsigned int len);

static FILE* file_create(FILE_STREAM_CTX* ctx);

static struct {
	const char* name;
	int tbuflen;
	int size;
	int (*func_open)(LOG_CTX* ctx, const char* url);
	int (*func_close)(LOG_CTX* ctx);
	int (*func_write)(LOG_CTX* ctx, int level, const char* msg, unsigned int msglen);
} map[] = {
	{ "file://",	FILE_BUFFER_SIZE,	sizeof(FILE_STREAM_CTX),	file_open,	file_close,	file_write },
	{ "console://",	0,					sizeof(CONSOLE_STREAM_CTX),	con_open,	con_close,	con_write },
};

LOG_HANDLE log_open(const char* url)
{
	int ret, type;
	LOG_CTX* ctx = NULL;

	for(type=0; type<sizeof(map)/sizeof(map[0]); type++) {
		if(memcmp(map[type].name, url, strlen(map[type].name))==0) {
			ctx = (LOG_CTX*)malloc(sizeof(LOG_CTX)+map[type].size+map[type].tbuflen);
			memset(ctx, 0, sizeof(LOG_CTX)+map[type].size+map[type].tbuflen);
			break;
		}
	}
	if(ctx==NULL) return NULL;

	ctx->type	= type;
	ctx->ptr	= (void*)(ctx+1);

	if(map[type].tbuflen==0) {
		memset(&ctx->stream, 0, sizeof(ctx->stream));
		return ctx;
	}

	ctx->stream.buf		= (char*)ctx + sizeof(LOG_CTX) + map[type].size;
	ctx->stream.cur		= 0;
	ctx->stream.len		= 0;
	ctx->stream.max		= map[type].tbuflen;

	os_sem_init(&ctx->stream.inque, 0);
	os_sem_init(&ctx->stream.ouque, 0);
	os_mutex_init(&ctx->stream.mtx);
	ctx->stream.inque_size	= 0;
	ctx->stream.ouque_size	= 0;

	ret = map[ctx->type].func_open(ctx, url+strlen(map[type].name));
	if(ret!=ERR_NOERROR) { free(ctx); return NULL; }

	os_thread_begin(&ctx->stream.thread, log_thread, (void*)ctx);

	return ctx;
}

int log_close(LOG_HANDLE ctx)
{
	int ret;

	if(ctx->stream.buf!=NULL) {
		os_sem_post(&ctx->stream.inque);
		os_thread_wait(ctx->stream.thread, NULL);

		os_sem_destroy(&ctx->stream.inque);
		os_sem_destroy(&ctx->stream.ouque);
		os_mutex_destroy(&ctx->stream.mtx);
	}

	ret = map[ctx->type].func_close(ctx);
	free(ctx);
	return ret;
}

int log_write(LOG_HANDLE ctx, int level, const char* fmt, ...)
{
	char buf[2*1024];
	int count;
	va_list argptr;

	va_start(argptr, fmt);
	count = vsnprintf(buf, sizeof(buf), fmt, argptr);
	va_end(argptr); 

	return log_puts(ctx, level, buf, count);
}

int log_puts(LOG_HANDLE ctx, int level, const char* msg, unsigned int msglen)
{
	if(msglen==0) msglen = (unsigned int)strlen(msg);
	if(ctx->stream.buf==NULL) return map[ctx->type].func_write(ctx, level, msg, msglen);

	for(;;) {
		os_mutex_lock(&ctx->stream.mtx);
		if(ctx->stream.len+sizeof(msglen)+msglen <= ctx->stream.max) break;
		ctx->stream.ouque_size++;
		os_mutex_unlock(&ctx->stream.mtx);
		os_sem_wait(&ctx->stream.ouque);
	}

	stream_write(ctx, (char*)&msglen, (unsigned int)sizeof(msglen));
	stream_write(ctx, msg, msglen);
	ctx->stream.inque_size++;

	if(ctx->stream.ouque_size>0) {
		ctx->stream.ouque_size--;
		os_sem_post(&ctx->stream.ouque);
	}

	os_sem_post(&ctx->stream.inque);
	os_mutex_unlock(&ctx->stream.mtx);

	get_level_string(level);

	return ERR_NOERROR;
}

unsigned int ZION_CALLBACK log_thread(void* arg)
{
	LOG_CTX* ctx = (LOG_CTX*)arg;
	unsigned int msglen = ctx->stream.len;
	char buf[1000];
	int level = 0;

	for(;;) {
		os_sem_wait(&ctx->stream.inque);

		os_mutex_lock(&ctx->stream.mtx);
		if(ctx->stream.inque_size==0) { os_mutex_unlock(&ctx->stream.mtx); break; }

		stream_read(ctx, (char*)&msglen, (unsigned int)sizeof(msglen));
		stream_read(ctx, buf, msglen);
		buf[msglen] = '\0';

		if(ctx->stream.ouque_size>0) {
			ctx->stream.ouque_size--;
			os_sem_post(&ctx->stream.ouque);
		}

		ctx->stream.inque_size--;
		os_mutex_unlock(&ctx->stream.mtx);

		map[ctx->type].func_write(ctx, level, buf, msglen);
	}
	return 0;
}

int stream_write(LOG_CTX* ctx, const char* buf, unsigned int len)
{
	int start;
	if(ctx->stream.max < ctx->stream.len + len) return ERR_NO_ENOUGH_MEMORY;
	start = ctx->stream.cur + ctx->stream.len;
	if(start + len > ctx->stream.max) {
		memcpy(ctx->stream.buf+start, buf, ctx->stream.max - start);
		memcpy(ctx->stream.buf, buf+ctx->stream.max-start, len - (ctx->stream.max - start));
	} else {
		memcpy(ctx->stream.buf+start, buf, len);
	}
	ctx->stream.len += len;
	return ERR_NOERROR;
}

int stream_read(LOG_CTX* ctx, char* buf, unsigned int len)
{
	if(ctx->stream.len<len) return ERR_NO_DATA;
	if(ctx->stream.cur+len>ctx->stream.max) {
		int elen = ctx->stream.max-ctx->stream.max - ctx->stream.max-ctx->stream.cur;
		memcpy(buf, ctx->stream.buf+ctx->stream.cur, elen);
		memcpy(buf+len, ctx->stream.buf, len-elen);
	} else {
		//memcpy(ctx->stream.buf+ctx->stream.cur, buf, len);
		memcpy(buf, ctx->stream.buf+ctx->stream.cur, len );
	}
	ctx->stream.cur = (ctx->stream.cur+len) % ctx->stream.max;
	ctx->stream.len -= len;
	return ERR_NOERROR;
}

const char* get_level_string(int level)
{
	switch(level) {
	case LOG_ERROR:		return "ERROR";
	case LOG_WARNING:	return "WARNING";
	case LOG_INFO:		return "INFO";
	case LOG_DEBUG:		return "DEBUG";
	default:			return "";
	}
}

int file_open(LOG_CTX* ctx, const char* url)
{
	const char* v;

	v = strstr(url, "@");
	if(v) {
		char mode[20];
		memcpy(&mode, url, v-url);
		mode[v-url] = '\0';
		if(strcmp(mode, "single")==0) {
			ctx->file->mode = FILE_MODE_SINGLE;
		} else if(strcmp(mode, "once")==0) {
			ctx->file->mode =  FILE_MODE_ONCE;
		} else if(strcmp(mode, "day")==0) {
			ctx->file->mode =  FILE_MODE_EVERYDAY;
		} else if(strcmp(mode, "week")==0) {
			ctx->file->mode =  FILE_MODE_EVERYWEEK;
		} else if(strcmp(mode, "mouth")==0) {
			ctx->file->mode =  FILE_MODE_EVERYMOUTH;
		} else {
			return ERR_INVALID_PARAMETER;
		}

		url = v+1;
	} else {
		ctx->file->mode = FILE_MODE_SINGLE;
	}

	v = strrchr(url, '/');
	if(v==NULL) v = strrchr(url, '\\');
	if(v==NULL) v = url; else v++;

	strcpy(ctx->file->name, v);
	memset(ctx->file->path, 0, sizeof(ctx->file->path));
	memcpy(ctx->file->path, url, v-url);

	if(!os_isdir(ctx->file->path) && os_mkdir(ctx->file->path)!=0)
		return ERR_UNKNOWN;

	ctx->file->mode = FILE_MODE_SINGLE;

	ctx->file->fp = file_create(ctx->file);
	if(ctx->file->fp==NULL) return ERR_FDOPEN;

	return ERR_NOERROR;
}

int file_close(LOG_CTX* ctx)
{
	fclose(ctx->file->fp);
	return ERR_NOERROR;
}

int file_write(LOG_CTX* ctx, int level, const char* msg, unsigned int len)
{
	time_t curtime;
	curtime = time(NULL);

	if(ctx->file->fp==NULL || (ctx->file->covertime>0 && ctx->file->covertime>curtime)) {
		if(ctx->file->fp) fclose(ctx->file->fp);
		ctx->file->fp = NULL;
		ctx->file->fp = file_create(ctx->file);
		if(ctx->file->fp==NULL) return ERR_FDOPEN;
	}

	fputs(msg, ctx->file->fp);
	fputs("\n", ctx->file->fp);

	return ERR_NOERROR;
}

int con_open(LOG_CTX* ctx, const char* url)
{
	return ERR_NOERROR;
}

int con_close(LOG_CTX* ctx)
{
	return ERR_NOERROR;
}

int con_write(LOG_CTX* ctx, int level, const char* msg, unsigned int len)
{
	puts(msg);
	puts("\n");
	return ERR_NOERROR;
}

FILE* file_create(FILE_STREAM_CTX *ctx)
{
	time_t timet;
	struct tm time_tm;
	const char* mode;
	char filename[200];

	switch(ctx->mode) {
	case FILE_MODE_SINGLE:
		sprintf(filename, "%s%s.log", ctx->path, ctx->name);
		ctx->covertime = 0;
		mode = "a";
		break;
	case FILE_MODE_ONCE:
		sprintf(filename, "%s%s-%d.log", ctx->path, ctx->name, os_process_getid());
		ctx->covertime = 0;
		mode = "w";
		break;
	case FILE_MODE_EVERYDAY:
		timet = time(NULL);
		memcpy(&time_tm, localtime(&timet), sizeof(time_tm));
 		sprintf(filename, "%s%s-%04d%02d%02d", ctx->path, ctx->name, time_tm.tm_year+1900, time_tm.tm_mon+1,time_tm.tm_mday);
 		ctx->covertime = timet - timet % (60 * 60 * 24) + 60* 60 * 24;
		mode = "a";
		break;
 	case FILE_MODE_EVERYWEEK:
		timet = time(NULL);
		memcpy(&time_tm, localtime(&timet), sizeof(time_tm));
		timet -= time_tm.tm_wday * 3600 * 24;
		memcpy(&time_tm, localtime(&timet), sizeof(time_tm));
 		sprintf(filename, "%s%s-%04d%02d%02d", ctx->path, ctx->name, time_tm.tm_year+1900, time_tm.tm_mon+1,time_tm.tm_mday);
		time_tm.tm_hour = 0; time_tm.tm_sec = 0; time_tm.tm_min = 0;
		ctx->covertime = mktime(&time_tm) + 60 * 60 * 24 * 7;
		mode = "a";
 		break;	
 	case FILE_MODE_EVERYMOUTH:
		timet = time(NULL);
		memcpy(&time_tm, localtime(&timet), sizeof(time_tm));
		timet -= time_tm.tm_mday * 3600 * 24;
		memcpy(&time_tm, localtime(&timet), sizeof(time_tm));
 		sprintf(filename, "%s%s-%04d%02d", ctx->path, ctx->name, time_tm.tm_year+1900, time_tm.tm_mon+1);
		time_tm.tm_hour = 0; time_tm.tm_sec = 0; time_tm.tm_min = 0;
		ctx->covertime = mktime(&time_tm) + 60 * 60 * 24 * 7;
		mode = "a";
 		break;
	default: return NULL;
	}

	return fopen(filename, mode);
}

