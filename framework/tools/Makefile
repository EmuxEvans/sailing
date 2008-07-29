BASE_DIR=..

include ../Makefile.def

PDL_DEPS = pdl.tab.h pdl.tab.c pdl.lex.c
IDL_DEPS = idl.tab.h idl.tab.c idl.lex.c
PDL_OBJS = pdl.tab.o pdl.lex.o pdl_cc.o pdl_codegen.o
IDL_OBJS = idl.tab.o idl.lex.o idl_cc.o idl_codegen.o
BINS=$(OUTPUT_DIR)/pdl_cc $(OUTPUT_DIR)/idl_cc

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

