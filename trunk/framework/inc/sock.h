#ifndef _SOCK_H_
#define _SOCK_H_

typedef int		SOCK_HANDLE;

#define SOCK_READ			(1<<1)
#define SOCK_WRITE			(1<<2)
#define SOCK_ACCEPT			SOCK_READ
#define SOCK_REUSEADDR		(1<<3)
#define SOCK_WAIT			(1<<4)
#define SOCK_NONBLOCK		(1<<5)

#define SOCK_INVALID_HANDLE	(-1)
#define SOCK_LINEEND		("\r\n")
#define SOCK_ERROR			(-1)
#define SOCK_INFINITE		(-1)

typedef struct SOCK_ADDR {
	unsigned int	ip;
	unsigned short	port;
} SOCK_ADDR;

typedef struct SOCK_TCP_OPTION {
	int		recvbuf;
	int		sndbuf;
	int		keep_alive;
	int		linger;
} SOCK_TCP_OPTION;

typedef struct SOCK_UDP_OPTION {
	int		recvbuf;
	int		sndbuf;
	int		broadcast;
} SOCK_UDP_OPTION;

ZION_API void sock_init();
ZION_API void sock_final();

ZION_API char* sock_addr2str(const SOCK_ADDR* addr, char* str);
ZION_API SOCK_ADDR* sock_str2addr(const char* str, SOCK_ADDR* addr);
ZION_API void sock_addr(SOCK_ADDR* addr, unsigned int ip, unsigned short port);

ZION_API int sock_nonblock(SOCK_HANDLE fd);
ZION_API void sock_default_tcp_option(SOCK_TCP_OPTION* option);
ZION_API void sock_default_udp_option(SOCK_UDP_OPTION* option);
ZION_API int sock_set_tcp_option(SOCK_HANDLE handle, const SOCK_TCP_OPTION* option);
ZION_API int sock_set_udp_option(SOCK_HANDLE handle, const SOCK_UDP_OPTION* option);

ZION_API SOCK_HANDLE sock_bind(SOCK_ADDR* addr, int flags);
ZION_API int sock_unbind(SOCK_HANDLE fd);

ZION_API SOCK_HANDLE sock_accept(SOCK_HANDLE fd, SOCK_ADDR* addr);
ZION_API SOCK_HANDLE sock_connect(const SOCK_ADDR* addr, int flags);
ZION_API int sock_disconnect(SOCK_HANDLE fd);
ZION_API void sock_close(SOCK_HANDLE fd);

ZION_API int sock_peername(SOCK_HANDLE fd, SOCK_ADDR* addr);
ZION_API int sock_sockname(SOCK_HANDLE fd, SOCK_ADDR* addr);

ZION_API int sock_read(SOCK_HANDLE fd, void* buf, int buf_len);
ZION_API int sock_write(SOCK_HANDLE fd, const void* buf, int buf_len);
ZION_API int sock_readbuf(SOCK_HANDLE fd, void* buf, int buf_len);
ZION_API int sock_writebuf(SOCK_HANDLE fd, const void* buf, int buf_len);

ZION_API int sock_readline(SOCK_HANDLE fd, char* buf, int buf_len);
ZION_API int sock_writeline(SOCK_HANDLE fd, const char* buf);

ZION_API SOCK_HANDLE sock_dgram_bind(SOCK_ADDR* endpoint, int broadcast);
ZION_API int sock_dgram_unbind(SOCK_HANDLE handle);
ZION_API int sock_dgram_send(SOCK_HANDLE handle, const SOCK_ADDR* addr, const char* buf, int buf_len);
ZION_API int sock_dgram_recv(SOCK_HANDLE handle, SOCK_ADDR* addr, char* buf, int buf_len);

#endif
