BASE_DIR=../../..

include $(BASE_DIR)/Makefile.def

SQLITE_CC_FLAG=$(subst -Werror,,$(subst -Wall,,$(OBJ_CC_FLAG)))

SQLITE_SRC=	alter.c analyze.c attach.c auth.c bitvec.c btmutex.c btree.c build.c callback.c complete.c date.c delete.c expr.c fault.c fts3.c\
			fts3_hash.c fts3_icu.c fts3_porter.c fts3_tokenizer.c fts3_tokenizer1.c func.c global.c hash.c insert.c journal.c legacy.c loadext.c\
			main.c malloc.c mem1.c mem2.c mem3.c mem4.c mem5.c mem6.c mutex.c mutex_os2.c mutex_unix.c mutex_w32.c opcodes.c os.c os_os2.c os_unix.c\
			os_win.c pager.c parse.c pcache.c pragma.c prepare.c printf.c random.c resolve.c rtree.c select.c status.c table.c tokenize.c\
			trigger.c update.c utf.c util.c vacuum.c vdbe.c vdbeapi.c vdbeaux.c vdbeblob.c vdbefifo.c vdbemem.c vtab.c walker.c where.c

SQLITE_INC=	btree.h btreeInt.h fts3.h fts3_hash.h fts3_tokenizer.h hash.h hwtime.h keywordhash.h\
			mutex.h opcodes.h os.h os_common.h pager.h parse.h pcache.h rtree.h sqlite3.h sqlite3ext.h\
			sqliteInt.h sqliteLimit.h vdbe.h vdbeInt.h

SQLITE_OBJ= $(subst .c,.o,$(SQLITE_SRC))

SQLITE_LIB= sqlite.a

.c.o:
	$(CC) -DSQLITE_CORE=1 $< $(SQLITE_CC_FLAG) $@

all: $(SQLITE_LIB)

clean:
	rm -rf $(SQLITE_LIB) $(SQLITE_OBJ)

$(SQLITE_LIB): $(SQLITE_OBJ)
	ar rcs $@ $^

