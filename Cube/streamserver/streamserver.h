#ifndef __STREAMSERVER_INCLUDE_
#define __STREAMSERVER_INCLUDE_

void streamserver_init();
void streamserver_final();

int streamserver_start(SOCK_ADDR* sa);
int streamserver_stop(const SOCK_ADDR* sa);

#endif
