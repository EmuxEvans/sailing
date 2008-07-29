#ifndef _PACKAGE_INCLUDE_
#define _PACKAGE_INCLUDE_

//
#define PACKAGE_GRACEFUL				0x100

#define PACKAGE_ONCONNECT				1
#define PACKAGE_ONDATA					2
#define PACKAGE_ONDISCONNECT			3

#define PACKAGE_CLOSE_NORMAL				0
#define PACKAGE_CLOSE_INTHREAD				1
#define PACKAGE_CLOSE_NOTCONNECT			2

#define PACKAGE_DISCONNECT_DIRTY			0
#define PACKAGE_DISCONNECT_CLEAN			1

struct PACKAGE_CONNECTION;
typedef struct PACKAGE_CONNECTION* PACKAGE_HANDLE;

typedef void (*PACKAGE_NOTIFY)(PACKAGE_HANDLE handle, int code, const void* recvbuf, unsigned int recvbuf_len);

ZION_API int package_init(unsigned int recvbuf_len);
ZION_API void package_final();

ZION_API int package_connect(SOCK_ADDR* addr, PACKAGE_NOTIFY notify);
ZION_API void package_close(PACKAGE_HANDLE handle, int flags);

ZION_API int package_bind(SOCK_ADDR* addr, PACKAGE_NOTIFY notify);
ZION_API int package_unbind(const SOCK_ADDR* addr);

ZION_API int package_send(PACKAGE_HANDLE handle, const void* buf, unsigned int buflen);
ZION_API void package_disconnect(PACKAGE_HANDLE handle, int flags);

ZION_API void package_setdata(PACKAGE_HANDLE handle, void* ptr);
ZION_API void* package_getdata(PACKAGE_HANDLE handle);

#endif
