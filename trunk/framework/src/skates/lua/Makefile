BASE_DIR=../../..

include $(BASE_DIR)/Makefile.def

LUA_CC_FLAG=$(subst -Werror,,$(subst -Wall,,$(OBJ_CC_FLAG)))

LUA_SRC=	lapi.c lauxlib.c lbaselib.c lcode.c ldblib.c ldebug.c ldo.c ldump.c lfunc.c lgc.c linit.c\
			liolib.c llex.c lmathlib.c lmem.c loadlib.c lobject.c lopcodes.c loslib.c lparser.c lstate.c\
			lstring.c lstrlib.c ltable.c ltablib.c ltm.c lundump.c lvm.c lzio.c print.c

LUA_INC=	lapi.h lauxlib.h lcode.h ldebug.h ldo.h lfunc.h lgc.h llex.h llimits.h lmem.h lobject.h\
			lopcodes.h lparser.h lstate.h lstring.h ltable.h ltm.h lua.h luaconf.h lualib.h lundump.h\
			lvm.h lzio.h

LUA_OBJ= $(subst .c,.o,$(LUA_SRC))

LUA_LIB= lua.a

.c.o:
	$(CC) $< $(LUA_CC_FLAG) $@

all: $(LUA_LIB)

clean:
	rm -rf $(LUA_LIB) $(LUA_OBJ)

$(LUA_LIB): $(LUA_OBJ)
	ar rcs $@ $^

