#ifndef _NETWORK_INCLUDE_
#define _NETWORK_INCLUDE_

#ifdef __cplusplus
extern "C" {
#endif

struct NETWORK_CONNECTION;
typedef struct NETWORK_CONNECTION* NETWORK_HANDLE;

typedef void (*NETWORK_ONACCEPT)(void* userptr, SOCK_HANDLE sock, const SOCK_ADDR* pname);
typedef void (*NETWORK_ONCONNECT)(NETWORK_HANDLE handle, void* userptr);
typedef void (*NETWORK_ONDATA)(NETWORK_HANDLE handle, void* userptr);
typedef void (*NETWORK_ONDISCONNECT)(NETWORK_HANDLE handle, void* userptr);

typedef struct NETWORK_EVENT {
	NETWORK_ONCONNECT		OnConnect;
	NETWORK_ONDATA			OnData;
	NETWORK_ONDISCONNECT	OnDisconnect;

	MEMPOOL_HANDLE			recvbuf_pool;
	char*					recvbuf_buf;
	unsigned int			recvbuf_max;
} NETWORK_EVENT;

typedef struct NETWORK_DOWNBUF {
	RLIST_ITEM		item;
	unsigned int	len;
	char			buf[0];
} NETWORK_DOWNBUF;

ZION_API void network_init(unsigned int downbuf_size);
ZION_API void network_final();

ZION_API int network_tcp_register(SOCK_ADDR* sa, NETWORK_ONACCEPT OnAccept, void* userptr);
ZION_API int network_tcp_unregister(const SOCK_ADDR* sa);

ZION_API NETWORK_HANDLE network_add(SOCK_HANDLE handle, NETWORK_EVENT* event, void* userptr);
ZION_API int network_del(NETWORK_HANDLE handle);

ZION_API unsigned int network_downbuf_size();
ZION_API NETWORK_DOWNBUF* network_downbuf_alloc();
ZION_API void network_downbuf_free(NETWORK_DOWNBUF* downbuf);

ZION_API unsigned int network_downbufs_alloc(NETWORK_DOWNBUF* downbufs[], unsigned int count, unsigned int size);
ZION_API int network_downbufs_fill(NETWORK_DOWNBUF* downbufs[], unsigned int count, unsigned int start, const void* data, unsigned int data_len);
ZION_API void network_downbufs_free(NETWORK_DOWNBUF* downbufs[], unsigned int count);

ZION_API unsigned int network_recvbuf_len(NETWORK_HANDLE handle);
ZION_API int network_recvbuf_get(NETWORK_HANDLE handle, void* buf, unsigned int start, unsigned int len);
ZION_API int network_recvbuf_commit(NETWORK_HANDLE handle, unsigned int len);

ZION_API int network_send(NETWORK_HANDLE handle, NETWORK_DOWNBUF* downbufs[], unsigned int count);
ZION_API int network_disconnect(NETWORK_HANDLE handle);

ZION_API SOCK_HANDLE network_get_sock(NETWORK_HANDLE handle);
ZION_API void* network_get_userptr(NETWORK_HANDLE handle);
ZION_API const SOCK_ADDR* network_get_peername(NETWORK_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif
