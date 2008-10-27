BASE_DIR=..

include ../Makefile.def

PDL_DEPS = pdl.tab.h pdl.tab.c pdl.lex.c
IDL_DEPS = idl.tab.h idl.tab.c idl.lex.c
PDL_OBJS = pdl.tab.o pdl.lex.o pdl_cc.o pdl_codegen.o
IDL_OBJS = idl.tab.o idl.lex.o idl_cc.o idl_codegen.o
CSM_SRCS = csmng.c $(BASE_DIR)/src/skates/os_unix.c $(BASE_DIR)/src/skates/misc.c $(BASE_DIR)/src/skates/fdwatch_$(FDWATCH_USE).c $(BASE_DIR)/src/skates/log.c $(BASE_DIR)/src/skates/appbox_unix.c $(BASE_DIR)/src/skates/appbox_args.c $(BASE_DIR)/src/skates/sock.c $(BASE_DIR)/src/skates/console.c
PRO_SRCS = prot_gen.c $(BASE_DIR)/src/skates/os_unix.c $(BASE_DIR)/src/skates/misc.c
BINS=$(OUTPUT_DIR)/pdl_cc $(OUTPUT_DIR)/idl_cc $(OUTPUT_DIR)/csmng $(OUTPUT_DIR)/prot_gen

#.y .tab.c:
%.tab.c %.tab.h: %.y
	$(YACC) $(YACC_FLAG) $<

#.l .lex.c:
%.lex.c: %.l
	$(LEX) $(LEX_FLAG) -o$@ $<

.c .o:
	$(CC) $(subst -Wall -Werror,,$(CC_FLAG)) $< $(OBJ_CC_FLAG) $@

all: $(BINS)

clean:
	rm -rf $(BINS) *.o $(PDL_DEPS) $(IDL_DEPS)

$(OUTPUT_DIR)/pdl_cc: $(PDL_DEPS) $(PDL_OBJS)
	$(CC) $(PDL_OBJS) -o $@

$(OUTPUT_DIR)/idl_cc: $(IDL_DEPS) $(IDL_OBJS)
	$(CC) $(IDL_OBJS) -o $@

$(OUTPUT_DIR)/csmng: $(CSM_SRCS)
	$(CC) $(CC_FLAG) $(OS_LIB) -DMODULE_NAME=\"csmng\" $^ -o $@

$(OUTPUT_DIR)/prot_gen: $(PRO_SRCS)
	$(CC) $(CC_FLAG) $(OS_LIB) -I$(BASE_DIR)/inc -DMODULE_NAME=\"prot_gen\" $^ -o $@
