#ifndef _async_rpc_H_
#define _async_rpc_H_

// C_DEF
#include <string.h>

// test_rpc1
int test_rpc1(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap, int r1, const int* r2, int* r3, int* r4);
int test_rpc1_impl(RPCNET_GROUP* group, int r1, const int* r2, int* r3, int* r4);
// test_rpc2
int test_rpc2(RPCNET_GROUP* group, RPCFUN_ASYNC_OVERLAPPED* overlap, int* r1, int* r2, int* r3, int* r4);
int test_rpc2_impl(RPCNET_GROUP* group, int* r1, int* r2, int* r3, int* r4);

extern RPCFUN_FUNCTION_DESC __async_rpc_desc[3];

#endif

