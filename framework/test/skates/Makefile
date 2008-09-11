BASE_DIR=../..

include $(BASE_DIR)/Makefile.def

INCLUDES=$(OS_INC_DIR) -I$(BASE_DIR)/inc
LIBS=$(OS_LIB_DIR) $(OS_LIB)
BINS=test_network$(EXE_PREFIX) test_fdwatch$(EXE_PREFIX) test_rqueue$(EXE_PREFIX) test_dbapi$(EXE_PREFIX) test_console$(EXE_PREFIX) test_log$(EXE_PREFIX) test_dymempool$(EXE_PREFIX) test_sock$(EXE_PREFIX) test_threadpool$(EXE_PREFIX) test_timer$(EXE_PREFIX) test_rpcnet$(EXE_PREFIX) test_rpcfun$(EXE_PREFIX) test_syncrpc$(EXE_PREFIX) test_asyncrpc$(EXE_PREFIX)

OBJS_NETWORK=test_network.o $(BASE_DIR)/src/skates/sock.o $(BASE_DIR)/src/skates/fdwatch_$(FDWATCH_USE).o $(BASE_DIR)/src/skates/network.o $(BASE_DIR)/src/skates/mempool.o $(BASE_DIR)/src/skates/os_unix.o
OBJS_FDWATCH=test_fdwatch.o $(BASE_DIR)/src/skates/sock.o $(BASE_DIR)/src/skates/fdwatch_$(FDWATCH_USE).o
OBJS_RQUEUE=test_rqueue.o $(BASE_DIR)/src/skates/sock.o $(BASE_DIR)/src/skates/fdwatch_$(FDWATCH_USE).o $(BASE_DIR)/src/skates/network.o $(BASE_DIR)/src/skates/hashmap.o $(BASE_DIR)/src/skates/mempool.o $(BASE_DIR)/src/skates/rqueue.o fake_applog.o $(BASE_DIR)/src/skates/os_unix.o
OBJS_DBAPI=test_dbapi.o $(BASE_DIR)/src/skates/os_unix.o $(BASE_DIR)/src/skates/mempool.o $(BASE_DIR)/src/skates/hashmap.o $(BASE_DIR)/src/skates/dbapi.o $(BASE_DIR)/src/skates/sqlite/sqlite3.o $(BASE_DIR)/src/skates/dbapi_sqlite.o
OBJS_CONSOLE=test_console.o $(BASE_DIR)/src/skates/sock.o $(BASE_DIR)/src/skates/fdwatch_$(FDWATCH_USE).o $(BASE_DIR)/src/skates/misc.o $(BASE_DIR)/src/skates/os_unix.o $(BASE_DIR)/src/skates/console.o
OBJS_LOG=test_log.o $(BASE_DIR)/src/skates/log.o $(BASE_DIR)/src/skates/os_unix.o
OBJS_DYMEMPOOL=test_dymempool.o $(BASE_DIR)/src/skates/dymempool.o $(BASE_DIR)/src/skates/os_unix.o
OBJS_SOCK=test_sock.o $(BASE_DIR)/src/skates/sock.o $(BASE_DIR)/src/skates/os_unix.o
OBJS_THREADPOOL=test_threadpool.o $(BASE_DIR)/src/skates/os_unix.o $(BASE_DIR)/src/skates/threadpool.o
OBJS_TIMER=test_timer.o $(BASE_DIR)/src/skates/mempool.o $(BASE_DIR)/src/skates/os_unix.o $(BASE_DIR)/src/skates/threadpool.o $(BASE_DIR)/src/skates/timer.o
OBJS_RPCNET=test_rpcnet.o fake_applog.o $(BASE_DIR)/src/skates/os_unix.o $(BASE_DIR)/src/skates/fdwatch_$(FDWATCH_USE).o $(BASE_DIR)/src/skates/mempool.o $(BASE_DIR)/src/skates/rpc_net.o $(BASE_DIR)/src/skates/sock.o $(BASE_DIR)/src/skates/stream.o $(BASE_DIR)/src/skates/threadpool.o
OBJS_RPCFUN=test_rpcfun.o fake_applog.o $(BASE_DIR)/src/skates/os_unix.o $(BASE_DIR)/src/skates/fdwatch_$(FDWATCH_USE).o $(BASE_DIR)/src/skates/mempool.o $(BASE_DIR)/src/skates/rpc_net.o $(BASE_DIR)/src/skates/sock.o $(BASE_DIR)/src/skates/stream.o $(BASE_DIR)/src/skates/threadpool.o $(BASE_DIR)/src/skates/rpc_fun.o $(BASE_DIR)/src/skates/hashmap.o
OBJS_SYNCRPC=fake_applog.o $(BASE_DIR)/src/skates/os_unix.o $(BASE_DIR)/src/skates/fdwatch_$(FDWATCH_USE).o $(BASE_DIR)/src/skates/mempool.o $(BASE_DIR)/src/skates/rpc_net.o $(BASE_DIR)/src/skates/sock.o $(BASE_DIR)/src/skates/stream.o $(BASE_DIR)/src/skates/threadpool.o $(BASE_DIR)/src/skates/rpc_fun.o $(BASE_DIR)/src/skates/hashmap.o sync_rpc.clt.o sync_rpc.svr.o test_syncrpc.o
OBJS_ASYNCRPC=fake_applog.o $(BASE_DIR)/src/skates/os_unix.o $(BASE_DIR)/src/skates/fdwatch_$(FDWATCH_USE).o $(BASE_DIR)/src/skates/mempool.o $(BASE_DIR)/src/skates/rpc_net.o $(BASE_DIR)/src/skates/sock.o $(BASE_DIR)/src/skates/stream.o $(BASE_DIR)/src/skates/threadpool.o $(BASE_DIR)/src/skates/rpc_fun.o $(BASE_DIR)/src/skates/hashmap.o async_rpc.clt.o async_rpc.svr.o test_asyncrpc.o
DEPS_SYNCRPC=sync_rpc.h sync_rpc.clt.c sync_rpc.svr.c
DEPS_ASYNCRPC=async_rpc.h async_rpc.clt.c async_rpc.svr.c

%.o: %.c
	$(CC) $(CC_FLAG) $(INCLUDES) -DMODULE_NAME=\"appbox\" $< $(OBJ_CC_FLAG) $@

%.h %.clt.c %.svr.c: %.idl
	$(OUTPUT_DIR)/idl_cc $<

all: $(BINS)

clean:
	rm -rf *.o $(BINS) $(DEPS_SYNCRPC) $(DEPS_ASYNCRPC)

test_network$(EXE_PREFIX): $(OBJS_NETWORK)
	gcc $(INCLUDES) $(LIBS) $(OBJS_NETWORK) -o $@

test_fdwatch$(EXE_PREFIX): $(OBJS_FDWATCH)
	gcc $(INCLUDES) $(LIBS) $(OBJS_FDWATCH) -o $@

test_rqueue$(EXE_PREFIX): $(OBJS_RQUEUE)
	gcc $(INCLUDES) $(LIBS) $(OBJS_RQUEUE) -o $@

test_dbapi$(EXE_PREFIX): $(OBJS_DBAPI)
	gcc $(INCLUDES) $(LIBS) $(OBJS_DBAPI) -o $@

test_console$(EXE_PREFIX): $(OBJS_CONSOLE)
	gcc $(INCLUDES) $(LIBS) $(OBJS_CONSOLE) -o $@

test_log$(EXE_PREFIX): $(OBJS_LOG)
	gcc $(INCLUDES) $(LIBS) $(OBJS_LOG) -o $@

test_dymempool$(EXE_PREFIX): $(OBJS_DYMEMPOOL)
	gcc $(INCLUDES) $(LIBS) $(OBJS_DYMEMPOOL) -o $@

test_sock$(EXE_PREFIX): $(OBJS_SOCK)
	gcc $(INCLUDES) $(LIBS) $(OBJS_SOCK) -o $@

test_threadpool$(EXE_PREFIX): $(OBJS_THREADPOOL)
	gcc $(INCLUDES) $(LIBS) $(OBJS_THREADPOOL) -o $@

test_timer$(EXE_PREFIX): $(OBJS_TIMER)
	gcc $(INCLUDES) $(LIBS) $(OBJS_TIMER) -o $@

test_rpcnet$(EXE_PREFIX): $(OBJS_RPCNET)
	gcc $(INCLUDES) $(LIBS) $(OBJS_RPCNET) -o $@

test_rpcfun$(EXE_PREFIX): $(OBJS_RPCFUN)
	gcc $(INCLUDES) $(LIBS) $(OBJS_RPCFUN) -o $@

test_syncrpc$(EXE_PREFIX): $(DEPS_SYNCRPC) $(OBJS_SYNCRPC)
	gcc $(INCLUDES) $(LIBS) $(OBJS_SYNCRPC) -o $@

test_asyncrpc$(EXE_PREFIX): $(DEPS_ASYNCRPC) $(OBJS_ASYNCRPC)
	gcc $(INCLUDES) $(LIBS) $(OBJS_ASYNCRPC) -o $@
