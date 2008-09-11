BASE_DIR=../..

include $(BASE_DIR)/Makefile.def

INCS=$(OS_INC_DIR) -DDBAPI_ORACLE_$(DBAPI_ORACLE) -DDBAPI_MYSQL_$(DBAPI_MYSQL) -DDBAPI_SQLRELAY_$(DBAPI_SQLREALY)
LIBS=$(OS_LIB_DIR) $(OS_LIB)
OBJS=os_unix.o misc.o mempool.o hashmap.o sock.o log.o dymempool.o stream.o dbapi.o dbapi_sqlite.o sqlite/sqlite3.o threadpool.o timer.o fdwatch_$(FDWATCH_USE).o console.o network.o rqueue.o rpc_net.o rpc_fun.o appbox.o applog.o appbox_unix.o appbox_args.o
MAIN_OBJS=appbox_main.o

.c.o:
	$(CC) $(CC_FLAG) $(INCLUDES) -DMODULE_NAME=\"appbox\" -DAPPBOX_MAIN $< $(OBJ_CC_FLAG) $@
#	$(CC) $(CC_FLAG) $(INCLUDES) -DMODULE_NAME=\"appbox\" -DAPPBOX_MAIN $< -S $(OBJ_CC_FLAG) $@.S

all: $(OUTPUT_DIR)/appbox$(DLL_PREFIX) $(OUTPUT_DIR)/appbox_main$(EXE_PREFIX)

clean:
	rm -rf $(OBJS) $(MAIN_OBJS) *.S $(OUTPUT_DIR)/appbox_main$(EXE_PREFIX) $(OUTPUT_DIR)/appbox$(DLL_PREFIX)

$(OUTPUT_DIR)/appbox_main$(EXE_PREFIX): $(OUTPUT_DIR)/appbox$(DLL_PREFIX) $(MAIN_OBJS)
	cp -f $(OUTPUT_DIR)/appbox$(DLL_PREFIX) .
	$(CC) $(CC_FLAG) ./appbox$(DLL_PREFIX) $(MAIN_OBJS) $(LIBS) $(EXE_CC_FLAG) appbox_main$(EXE_PREFIX)
	cp -f appbox_main$(EXE_PREFIX) $(OUTPUT_DIR)/
	rm -f appbox$(DLL_PREFIX) appbox_main$(EXE_PREFIX)

$(OUTPUT_DIR)/appbox$(DLL_PREFIX): $(OBJS)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(CC_FLAG) $(OBJS) $(LIBS) $(DLL_CC_FLAG) $@
	rm -f appbox$(DLL_PREFIX)

