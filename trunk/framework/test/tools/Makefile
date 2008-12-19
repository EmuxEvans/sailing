BASE_DIR=../..

include $(BASE_DIR)/Makefile.def

INCLUDES=$(OS_INC_DIR) -I$(BASE_DIR)/inc
LIBS=$(OS_LIB_DIR) $(OS_LIB)
BINS=test_proto$(EXE_PREFIX) test_proto_lua$(EXE_PREFIX)

PROT_OBJ=test_proto.o sample_p.proto.o
LUA_OBJ=test_proto_lua.o sample_p.proto.o sample_p.proto.lua.o

###%.proto.c: %.proto

%.proto.c %.proto:
	$(OUTPUT_DIR)/prot_gen$(EXE_PREFIX) $(subst .proto.c,.proto,$@)

%.proto.lua.cc %.proto:
	$(OUTPUT_DIR)/prot_gen$(EXE_PREFIX) $(subst .proto.lua.cc,.proto,$@)

%.o: %.c
	$(CC) $(CC_FLAG) $(INCLUDES) -DMODULE_NAME=\"appbox\" $< $(OBJ_CC_FLAG) $@

%.o: %.cc
	g++ $(CC_FLAG) $(INCLUDES) -DMODULE_NAME=\"appbox\" $< $(OBJ_CC_FLAG) $@

all: sample_p.proto.c sample_p.proto.lua.cc $(BINS)

clean:
	rm -rf *.o $(BINS) $(DEPS_SYNCRPC) $(DEPS_ASYNCRPC) *.proto.*

test_proto$(EXE_PREFIX): $(PROT_OBJ)
	gcc $(INCLUDES) $(LIBS) $(PROT_OBJ) $(OUTPUT_DIR)/appbox.a -o $@

test_proto_lua$(EXE_PREFIX): $(LUA_OBJ)
	g++ $(INCLUDES) $(LIBS) $(LUA_OBJ) $(OUTPUT_DIR)/appbox.a -o $@

