#include <string.h>

#include "../../inc/skates/errcode.h"
#include "../../inc/skates/os.h"
#include "../../inc/skates/stream.h"

static void memstream_clear(MEM_STREAM* stream);
static void memstream_destroy(MEM_STREAM* stream);
static int memstream_skip(MEM_STREAM* stream, unsigned int len);
static int memstream_seek(MEM_STREAM* stream, unsigned int loc);
static int memstream_put(MEM_STREAM* stream, void* buf, unsigned int len);
static int memstream_get(MEM_STREAM* stream, void* buf, unsigned int len);

static STREAM_INTERFACE memstream = 
STREAM_INTERFACE_DEFINE(
memstream_clear,
memstream_destroy,
memstream_skip,
memstream_seek,
memstream_put,
memstream_get);


void memstream_init(MEM_STREAM* stream, void* buf, unsigned int maxlen, unsigned int len)
{
	stream->i = &memstream;
	stream->buf = buf;
	stream->cur = 0;
	stream->len = len;
	stream->maxlen = maxlen;
}

void* memstream_get_position(MEM_STREAM* stream)
{
	return stream->buf+stream->cur;
}

int memstream_get_length(MEM_STREAM* stream)
{
	return stream->len;
}

void memstream_clear(MEM_STREAM* stream)
{
	stream->cur = 0;
}

void memstream_destroy(MEM_STREAM* stream)
{
	stream->i = NULL;
}

int memstream_skip(MEM_STREAM* stream, unsigned int len)
{
	if(stream->cur+len>stream->maxlen) return ERR_UNKNOWN;
	stream->cur += len;
	if(stream->cur>stream->len) stream->len = stream->cur;
	return ERR_NOERROR;
}

int memstream_seek(MEM_STREAM* stream, unsigned int loc)
{
	if(loc>stream->len) return ERR_UNKNOWN;
	stream->cur = loc;
	return ERR_NOERROR;
}

int memstream_put(MEM_STREAM* stream, void* buf, unsigned int len)
{
	if(stream->cur+len>stream->maxlen) return ERR_UNKNOWN;
	memcpy(stream->buf+stream->cur, buf, len);
	stream->cur += len;
	if(stream->cur>stream->len) stream->len = stream->cur;
	return ERR_NOERROR;
}

int memstream_get(MEM_STREAM* stream, void* buf, unsigned int len)
{
	if(stream->cur+len>stream->len) return ERR_UNKNOWN;
	if(buf!=stream->buf+stream->cur) memcpy(buf, stream->buf+stream->cur, len);
	stream->cur += len;
	if(stream->cur>stream->len) stream->len = stream->cur;
	return ERR_NOERROR;
}

