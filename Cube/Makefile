BASE_DIR=.
include ../framework/Makefile.def

INCS=-I../framework/inc
SRCS=cube_server.c cube_pdl_impl.c cube_pdl.CubeServer.c
OBJS=$(subst .c,.o,$(SRCS))
MODULE_NAME=cube

SS_OBJS=streamserver/streamserver.o streamserver/main.o

.c.o  :;$(CC)  $(CC_FLAG) $(INCS) -DMODULE_NAME=\"$(MODULE_NAME)\" -DMODULE_$(MODULE_NAME) -r $< $(OBJ_CC_FLAG) $@

all: CubeServer/cube_pdl.CubeServer.c $(OUTPUT_DIR)/modules/cube$(DLL_PREFIX) $(OUTPUT_DIR)/streamserver$(EXE_PREFIX)

clean:
	rm -f CubeServer/*.o CubeServer/*.CubeServer.*

CubeServer/cube_pdl.CubeServer.c: cube_pdl.pdl
	rm -f CubeServer/cube_pdl.CubeServer.c
	rm -f CubeServer/cube_pdl.CubeServer.h
	cp -f cube_pdl.pdl cube_pdl.CubeServer.pdl
	$(OUTPUT_DIR)/pdl_cc CMD_SVR SVR cube_pdl.CubeServer.pdl
	rm -f cube_pdl.CubeServer.pdl
	mv cube_pdl.CubeServer.c CubeServer/
	mv cube_pdl.CubeServer.h CubeServer/

$(OUTPUT_DIR)/modules/cube$(DLL_PREFIX): $(foreach obj,$(OBJS),CubeServer/$(obj))
	cd $(OUTPUT_DIR); $(CC) $(DLL_CC_FLAG) modules/cube$(DLL_PREFIX) $(foreach obj,$(OBJS),../CubeServer/$(obj))

$(OUTPUT_DIR)/streamserver$(EXE_PREFIX): $(SS_OBJS)
	$(CC) $(OS_LIB) $(OUTPUT_DIR)/appbox.a $^ $(EXE_CC_FLAG) $@ $(OUTPUT_DIR)/appbox.a

