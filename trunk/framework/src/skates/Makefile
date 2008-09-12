BASE_DIR=../..

include $(BASE_DIR)/Makefile.def

SQLITE_SRC=	alter.c analyze.c attach.c auth.c bitvec.c btmutex.c btree.c build.c callback.c complete.c date.c delete.c expr.c fault.c fts3.c\
			fts3_hash.c fts3_icu.c fts3_porter.c fts3_tokenizer.c fts3_tokenizer1.c func.c global.c hash.c insert.c journal.c legacy.c loadext.c\
			main.c malloc.c mem1.c mem2.c mem3.c mem4.c mem5.c mem6.c mutex.c mutex_os2.c mutex_unix.c mutex_w32.c opcodes.c os.c os_os2.c os_unix.c\
			os_win.c pager.c parse.c pcache.c pragma.c prepare.c printf.c random.c resolve.c rtree.c select.c shell.c status.c table.c tokenize.c\
			trigger.c update.c utf.c util.c vacuum.c vdbe.c vdbeapi.c vdbeaux.c vdbeblob.c vdbefifo.c vdbemem.c vtab.c walker.c where.c
SQLITE_INC=	btree.h btreeInt.h fts3.h fts3_hash.h fts3_tokenizer.h hash.h hwtime.h keywordhash.h\
			mutex.h opcodes.h os.h os_common.h pager.h parse.h pcache.h rtree.h sqlite3.h sqlite3ext.h\
			sqliteInt.h sqliteLimit.h vdbe.h vdbeInt.h

SQLITE_FULL_INC=$(foreach inc,$(SQLITE_INC),sqlite/$(inc))
SQLITE_FULL_SRC=$(foreach src,$(SQLITE_SRC),sqlite/$(src))
SQLITE_FULL_OBJ=$(subst .c,.o,$(SQLITE_FULL_SRC))
SQLITE_FULL_LIB=sqlite/sqlite.a

INCS=$(OS_INC_DIR) -DDBAPI_MYSQL_$(DBAPI_MYSQL)
LIBS=$(OS_LIB_DIR) $(OS_LIB) $(APPBOX_LIB)
OBJS=os_unix.o misc.o mempool.o hashmap.o sock.o log.o dymempool.o stream.o dbapi.o dbapi_sqlite.o dbapi_mysql.o threadpool.o timer.o fdwatch_$(FDWATCH_USE).o console.o network.o rqueue.o rpc_net.o rpc_fun.o appbox.o applog.o appbox_unix.o appbox_args.o $(SQLITE_FULL_LIB)
MAIN_OBJS=appbox_main.o

.c.o:
	$(CC) $(CC_FLAG) $(INCS) -DMODULE_NAME=\"appbox\" -DAPPBOX_MAIN $< $(OBJ_CC_FLAG) $@
#	$(CC) $(CC_FLAG) $(INCS) -DMODULE_NAME=\"appbox\" -DAPPBOX_MAIN $< -S $(OBJ_CC_FLAG) $@.S

all: $(OUTPUT_DIR)/appbox$(STA_LIB_PREFIX) $(OUTPUT_DIR)/appbox$(DLL_PREFIX) $(OUTPUT_DIR)/appbox_main$(EXE_PREFIX)

clean:
	rm -rf $(OBJS) $(MAIN_OBJS) $(SQLITE_FULL_OBJ) $(SQLITE_FULL_LIB) *.S $(OUTPUT_DIR)/appbox_main$(EXE_PREFIX) $(OUTPUT_DIR)/appbox$(DLL_PREFIX) $(OUTPUT_DIR)/appbox$(STA_LIB_PREFIX)

$(OUTPUT_DIR)/appbox_main$(EXE_PREFIX): $(OUTPUT_DIR)/appbox$(DLL_PREFIX) $(MAIN_OBJS)
	cp -f $(OUTPUT_DIR)/appbox$(DLL_PREFIX) .
	$(CC) $(CC_FLAG) ./appbox$(DLL_PREFIX) $(MAIN_OBJS) $(EXE_CC_FLAG) appbox_main$(EXE_PREFIX)
	cp -f appbox_main$(EXE_PREFIX) $(OUTPUT_DIR)/
	rm -f appbox$(DLL_PREFIX) appbox_main$(EXE_PREFIX)

$(OUTPUT_DIR)/appbox$(DLL_PREFIX): $(OBJS)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(CC_FLAG) $(OBJS) $(LIBS) $(DLL_CC_FLAG) $@
	rm -f appbox$(DLL_PREFIX)

$(OUTPUT_DIR)/appbox$(STA_LIB_PREFIX): $(OBJS)
	$(STA_LIB) $@ $^

$(SQLITE_FULL_LIB): $(SQLITE_FULL_INC) $(SQLITE_FULL_SRC)
	cd sqlite; make ; cd ..

