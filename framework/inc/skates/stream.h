#ifndef _STREAM_INCLUDE_
#define _STREAM_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

struct STREAM;
struct STREAM_INTERFACE;
typedef struct STREAM			STREAM;
typedef struct STREAM_INTERFACE	STREAM_INTERFACE;

struct STREAM {
	STREAM_INTERFACE* i;
};

struct STREAM_INTERFACE {
	void (*clear)(STREAM* stream);
	void (*destroy)(STREAM* stream);

	int (*skip)(STREAM* stream, unsigned int len);
	int (*seek)(STREAM* stream, unsigned int loc);

	int (*put)(STREAM* stream, const void* buf, unsigned int len);
	int (*get)(STREAM* stream, void* buf, unsigned int len);
};

typedef struct MEM_STREM {
	STREAM_INTERFACE* i;
	unsigned char* buf;
	unsigned int len, cur, maxlen;
} MEM_STREAM;

#define STREAM_INTERFACE_DEFINE(clear, destroy, skip, seek, put, get) \
	{	\
	(void (*)(STREAM* stream))clear,	\
	(void (*)(STREAM* stream))destroy,	\
	\
    (int (*)(STREAM* stream, unsigned int len))skip,	\
	(int (*)(STREAM* stream, unsigned int loc))seek,	\
	\
	(int (*)(STREAM* stream, const void* buf, unsigned int len))put,	\
	(int (*)(STREAM* stream, void* buf, unsigned int len))get,	\
	}


//
ZION_API void memstream_init(MEM_STREAM* stream, void* buf, unsigned int maxlen, unsigned int len);
ZION_API void* memstream_get_position(MEM_STREAM* stream);
ZION_API int memstream_get_length(MEM_STREAM* stream);

//int stream_clear(STREAM* stream)
#define stream_clear(stream)			((STREAM*)(stream))->i->clear((STREAM*)stream)
//int stream_destroy(STREAM* stream)
#define stream_destroy(stream)			((STREAM*)(stream))->i->destroy((STREAM*)stream)

//int stream_skip(STREAM* stream, unsigned int len);
#define stream_skip(stream, len)		((STREAM*)(stream))->i->skip((STREAM*)stream, len)
//int stream_seek(STREAM* stream, unsigned int len);
#define stream_seek(stream, len)		((STREAM*)(stream))->i->seek((STREAM*)stream, len)

//int stream_put(STREAM* stream, void* buf, unsigned int len);
#define stream_put(stream, buf, len)	((STREAM*)(stream))->i->put((STREAM*)stream, buf, len)
//int stream_get(STREAM* stream, void* buf, unsigned int len);
#define stream_get(stream, buf, len)	((STREAM*)(stream))->i->get((STREAM*)stream, buf, len)

#ifdef __cplusplus
}
#endif

#endif

