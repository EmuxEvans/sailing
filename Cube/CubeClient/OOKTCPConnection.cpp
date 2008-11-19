#include <winsock2.h>

#include "cube_pdl.CubeClient.h"
#include "cube_pdl.CubeClient.hook.hpp"
#include "TCPConnection.h"
#include "OOKTCPConnection.h"

COOKTCPConnection* COOKTCPConnection::m_pDefault = NULL;

COOKTCPConnection g_Conn(TRUE);

CubeHookDispatcher* CubeHookDispatcher::GetDispatcher(CLT_USER_CTX* CTX)
{
	return CTX->pConn;
}


COOKTCPConnection::COOKTCPConnection(BOOL bDefault)
{
	_ASSERT(m_pDefault==NULL || !bDefault);
	if(bDefault) m_pDefault = this;
	ZeroMemory(m_arHooker, sizeof(m_arHooker));
	m_nHookerCur = sizeof(m_arHooker)/sizeof(m_arHooker[0]);
	m_ctxUser.pConn = this;
}

COOKTCPConnection::~COOKTCPConnection(void)
{
}

#define KEEPALIVE			(20*1000)

BOOL COOKTCPConnection::Connect(LPCTSTR pszAddr, BOOL bKeepAlive)
{
	m_nInBufLen = 0;
	return CTCPConnection::Connect(pszAddr);
}

void COOKTCPConnection::Disconnect()
{
	CTCPConnection::Disconnect();
}

BOOL COOKTCPConnection::Dispatch(BOOL bBlock)
{
	int nRet, nLen;
	char szTmpBuf[100*1024];

	for(;;) {
		if(bBlock) Wait();
		
		//
		nRet = Receive(m_szInBuf+m_nInBufLen, sizeof(m_szInBuf)-m_nInBufLen);
		if(nRet==-1) return FALSE;
		m_nInBufLen += nRet;
		
		//
		if(m_nInBufLen<2) {
			if(!bBlock) return TRUE;
			continue;
		}

		nLen = (int)(*((WORD*)m_szInBuf));
		if(nLen>m_nInBufLen) {
			if(!bBlock) return TRUE;
			continue;
		}

		//
		MEM_STREAM stream;
		memcpy(szTmpBuf, m_szInBuf+sizeof(WORD), (unsigned int)nLen-sizeof(WORD));
		memstream_init(&stream, szTmpBuf, (unsigned int)nLen-sizeof(WORD), (unsigned int)nLen-sizeof(WORD));
		memmove(m_szInBuf, m_szInBuf+nLen, m_nInBufLen-nLen);
		m_nInBufLen -= nLen;
		//
		nRet = CLT_Dispatcher(&m_ctxUser, (STREAM*)&stream);
		if(nRet!=ERR_NOERROR) return FALSE;
		//
		if(bBlock) return TRUE;
	}
}

void COOKTCPConnection::SendStream(STREAM* stream)
{
	_ASSERT(stream==(STREAM*)&m_ctxUser.stream);
	*((WORD*)m_ctxUser.data) = sizeof(WORD) + (WORD)m_ctxUser.stream.len;
	Send(m_ctxUser.data, sizeof(WORD)+(int)m_ctxUser.stream.len);
}

void COOKTCPConnection::Attach(CubeHook* hooker)
{
	_ASSERT(hooker!=NULL);
	for(int l=0; l<sizeof(m_arHooker)/sizeof(m_arHooker[0]); l++) {
		if(m_arHooker[l]==NULL) {
			m_arHooker[l] = hooker;
			return;
		}
	}
	_ASSERT(0);
}

void COOKTCPConnection::Detach(CubeHook* hooker)
{
	_ASSERT(hooker!=NULL);
	for(int l=0; l<sizeof(m_arHooker)/sizeof(m_arHooker[0]); l++) {
		if(m_arHooker[l]==hooker) {
			m_arHooker[l] = NULL;
			return;
		}
	}
	_ASSERT(0);
}

CubeHook* COOKTCPConnection::FindReset()
{
	m_nHookerCur = -1;
	return FindNext();
}

CubeHook* COOKTCPConnection::FindNext()
{
	if(m_nHookerCur>=(int)(sizeof(m_arHooker)/sizeof(m_arHooker[0]))) return NULL;
	m_nHookerCur++;
	for(; m_nHookerCur<sizeof(m_arHooker)/sizeof(m_arHooker[0]); m_nHookerCur++) 
		if(m_arHooker[m_nHookerCur]!=NULL) return m_arHooker[m_nHookerCur];
	return NULL;
}

CubeHook* COOKTCPConnection::FindGet()
{
	if(m_nHookerCur<sizeof(m_arHooker)/sizeof(m_arHooker[0])) {
		return m_arHooker[m_nHookerCur];
	} else {
		return NULL;
	}
}

int CLT_Newstream(CLT_USER_CTX* ctx, STREAM** ptr)
{
	*ptr = ctx->pConn->NewStream();
	return ERR_NOERROR;
}

int CLT_Send(CLT_USER_CTX* ctx, STREAM* stream)
{
	ctx->pConn->SendStream(stream);
	return ERR_NOERROR;
}

int CLT_Alloc(CLT_USER_CTX* ctx, STREAM* stream, void** ptr, int size)
{
	*ptr = ((MEM_STREAM*)stream)->buf + ((MEM_STREAM*)stream)->cur;
	return ERR_NOERROR;
}

void CLT_Free(CLT_USER_CTX* ctx, void* ptr)
{
}

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
	stream->buf = (unsigned char*)buf;
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
