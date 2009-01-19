BASE_DIR=../..

include $(BASE_DIR)/Makefile.def

INCLUDES=$(OS_INC_DIR) -I$(BASE_DIR)/inc
LIBS=$(OS_LIB_DIR) $(OS_LIB)
BINS=test_network$(EXE_PREFIX) test_fdwatch$(EXE_PREFIX) test_rqueue$(EXE_PREFIX) test_dbapi$(EXE_PREFIX) test_console$(EXE_PREFIX) test_log$(EXE_PREFIX) test_dymempool$(EXE_PREFIX) test_sock$(EXE_PREFIX) test_threadpool$(EXE_PREFIX) test_timer$(EXE_PREFIX) test_rpcnet$(EXE_PREFIX) test_rpcfun$(EXE_PREFIX) test_syncrpc$(EXE_PREFIX) test_asyncrpc$(EXE_PREFIX) test_srp6a$(EXE_PREFIX)

OBJS_NETWORK=test_network.o
OBJS_FDWATCH=test_fdwatch.o
OBJS_RQUEUE=test_rqueue.o
OBJS_DBAPI=test_dbapi.o
OBJS_CONSOLE=test_console.o
OBJS_LOG=test_log.o
OBJS_DYMEMPOOL=test_dymempool.o
OBJS_SOCK=test_sock.o
OBJS_THREADPOOL=test_threadpool.o
OBJS_TIMER=test_timer.o
OBJS_RPCNET=test_rpcnet.o
OBJS_RPCFUN=test_rpcfun.o
OBJS_SYNCRPC=test_syncrpc.o sync_rpc.clt.o sync_rpc.svr.o
OBJS_ASYNCRPC=test_asyncrpc.o async_rpc.clt.o async_rpc.svr.o
DEPS_SYNCRPC=sync_rpc.h sync_rpc.clt.c sync_rpc.svr.c
OBJS_SRP6A=test_srp6a.o
DEPS_ASYNCRPC=async_rpc.h async_rpc.clt.c async_rpc.svr.c

%.o: %.c
	$(CC) $(CC_FLAG) $(INCLUDES) -DMODULE_NAME=\"appbox\" $< $(OBJ_CC_FLAG) $@

%.h %.clt.c %.svr.c: %.idl
	$(OUTPUT_DIR)/idl_cc $<

all: $(BINS)

clean:
	rm -rf appbox$(STA_LIB_PREFIX) *.o $(BINS) $(DEPS_SYNCRPC) $(DEPS_ASYNCRPC)

appbox$(STA_LIB_PREFIX):
	cd ../../src/skates; $(MAKE) $(OUTPUT_DIR)/appbox$(STA_LIB_PREFIX)
	cp $(OUTPUT_DIR)/appbox$(STA_LIB_PREFIX) .

test_network$(EXE_PREFIX): $(OBJS_NETWORK) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $^ -o $@

test_fdwatch$(EXE_PREFIX): $(OBJS_FDWATCH) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $^ -o $@

test_rqueue$(EXE_PREFIX): $(OBJS_RQUEUE) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $^ -o $@

test_dbapi$(EXE_PREFIX): $(OBJS_DBAPI) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $(MYSQL_LIB) $^ -o $@

test_console$(EXE_PREFIX): $(OBJS_CONSOLE) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $^ -o $@

test_log$(EXE_PREFIX): $(OBJS_LOG) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $^ -o $@

test_dymempool$(EXE_PREFIX): $(OBJS_DYMEMPOOL) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $^ -o $@

test_sock$(EXE_PREFIX): $(OBJS_SOCK) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $^ -o $@

test_threadpool$(EXE_PREFIX): $(OBJS_THREADPOOL) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $^ -o $@

test_timer$(EXE_PREFIX): $(OBJS_TIMER) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $^ -o $@

test_rpcnet$(EXE_PREFIX): $(OBJS_RPCNET) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $^ -o $@

test_rpcfun$(EXE_PREFIX): $(OBJS_RPCFUN) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $^ -o $@

test_syncrpc$(EXE_PREFIX): $(DEPS_SYNCRPC) $(OBJS_SYNCRPC) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $(OBJS_SYNCRPC) appbox$(STA_LIB_PREFIX) -o $@

test_asyncrpc$(EXE_PREFIX): $(DEPS_ASYNCRPC) $(OBJS_ASYNCRPC) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $(OBJS_ASYNCRPC) appbox$(STA_LIB_PREFIX) -o $@

test_srp6a$(EXE_PREFIX): $(OBJS_SRP6A) appbox$(STA_LIB_PREFIX)
	gcc $(INCLUDES) $(LIBS) $^ -o $@

