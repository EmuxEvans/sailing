#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/errcode.h"
#include "../inc/os.h"
#include "../inc/sock.h"

unsigned int ZION_CALLBACK TcpThreadFunc(void * point);
unsigned int ZION_CALLBACK UdpThreadFunc(void * point);

int main(int argc, char ** argv)
{
	os_thread_t hThreadTcp,hThreadUdp;
	unsigned int Tcpid, Udpid;
	if(argc !=2)
	{
		printf("please input the parameters!!");
		exit(1);
	}
	
	os_thread_begin(&hThreadUdp, UdpThreadFunc, &Udpid);
	os_thread_begin(&hThreadTcp, TcpThreadFunc, &Tcpid);

	printf("udp listen thread start sucess!!\n");
	os_thread_wait(hThreadUdp, NULL);
	os_thread_close(hThreadUdp);

	printf("tcp listen thread start sucess!!\n");
	os_thread_wait(hThreadTcp, NULL);
	os_thread_close(hThreadTcp);

	return 0;
}

unsigned int ZION_CALLBACK TcpThreadFunc(void * point)
{
	char *p = (char*)point;
	SOCK_HANDLE handle;
	SOCK_ADDR  sock_addr;
	char read_buf[1024];
	int ret = ERR_NOERROR;
	
	sock_init();
	if(sock_str2addr(p, &sock_addr)==NULL) 
	{
		printf("tcp sock_str2addr function error\n");
		goto FINISH_STATE;
	}
	
	
	handle = sock_bind(&sock_addr, 0);
	if(handle == SOCK_INVALID_HANDLE)
	{
		printf("tcp bind function error\n");
		goto ERROR_STATE;
	}
	
	printf("tcp %s wait for client connect....\n", p);

	handle =  sock_accept(handle, NULL);
	if(handle == SOCK_INVALID_HANDLE)
	{
		printf("tcp socket_accept function error\n");
		goto ERROR_STATE;
	}

	printf("tcp client connect access\n");

	while(1)
	{
		memset(read_buf, 0, sizeof(read_buf));
		//sock_readbuf(handle, (void*)read_buf, sizeof(read_buf));
		sock_read(handle, (void*)read_buf, sizeof(read_buf));
		printf("tcp client send '%s'\n", read_buf);
		sock_write(handle, (void*)read_buf, (int)strlen(read_buf));
	}


ERROR_STATE:
	sock_unbind(handle);
FINISH_STATE:
	sock_final();

	return 0;
}

unsigned int ZION_CALLBACK UdpThreadFunc(void * point)
{
	char *p = (char*)point;
	SOCK_HANDLE handle;
	SOCK_ADDR  sock_addr;
	char read_buf[1024];
	int ret = ERR_NOERROR;
	
	sock_init();
	if(sock_str2addr(p, &sock_addr) == NULL)
	{
		printf("udp sock_str2addr function error\n");
		goto FINISH_STATE;
	}
	
	
	handle = sock_dgram_bind(&sock_addr, 0);
	if(handle == SOCK_INVALID_HANDLE)
	{
		printf("udp bind function error\n");
		goto ERROR_STATE;
	}

	if(ret != ERR_NOERROR)
	{
		printf("udp sock_wait function error\n");
		goto ERROR_STATE;
	}
	

	sock_dgram_send(handle, &sock_addr, "12345",5);
	while(1)
	{
		memset(read_buf, 0, sizeof(read_buf));
		//sock_readbuf(handle, (void*)read_buf, sizeof(read_buf));
		sock_dgram_recv(handle, &sock_addr, read_buf, sizeof(read_buf));
		printf("udp client send '%s'\n", read_buf);
		sock_dgram_send(handle, &sock_addr, read_buf, sizeof(read_buf));
		
	}


ERROR_STATE:
	sock_dgram_unbind(handle);
FINISH_STATE:
	sock_final();
	
	return 0;
}
