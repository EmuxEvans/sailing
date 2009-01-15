BASE_DIR=../..

include $(BASE_DIR)/Makefile.def

SQLITE_SRC=	alter.c analyze.c attach.c auth.c bitvec.c btmutex.c btree.c build.c callback.c complete.c date.c delete.c expr.c fault.c fts3.c\
			fts3_hash.c fts3_icu.c fts3_porter.c fts3_tokenizer.c fts3_tokenizer1.c func.c global.c hash.c insert.c journal.c legacy.c loadext.c\
			main.c malloc.c mem1.c mem2.c mem3.c mem4.c mem5.c mem6.c mutex.c mutex_os2.c mutex_unix.c mutex_w32.c opcodes.c os.c os_os2.c os_unix.c\
			os_win.c pager.c parse.c pcache.c pragma.c prepare.c printf.c random.c resolve.c rtree.c select.c status.c table.c tokenize.c\
			trigger.c update.c utf.c util.c vacuum.c vdbe.c vdbeapi.c vdbeaux.c vdbeblob.c vdbefifo.c vdbemem.c vtab.c walker.c where.c
SQLITE_INC=	btree.h btreeInt.h fts3.h fts3_hash.h fts3_tokenizer.h hash.h hwtime.h keywordhash.h\
			mutex.h opcodes.h os.h os_common.h pager.h parse.h pcache.h rtree.h sqlite3.h sqlite3ext.h\
			sqliteInt.h sqliteLimit.h vdbe.h vdbeInt.h
SQLITE_FULL_INC=$(foreach inc,$(SQLITE_INC),sqlite/$(inc))
SQLITE_FULL_SRC=$(foreach src,$(SQLITE_SRC),sqlite/$(src))
SQLITE_FULL_OBJ=$(subst .c,.o,$(SQLITE_FULL_SRC))
SQLITE_FULL_LIB=sqlite/sqlite.a

LUA_SRC=    lapi.c lauxlib.c lbaselib.c lcode.c ldblib.c ldebug.c ldo.c ldump.c lfunc.c lgc.c linit.c\
            liolib.c llex.c lmathlib.c lmem.c loadlib.c lobject.c lopcodes.c loslib.c lparser.c lstate.c\
            lstring.c lstrlib.c ltable.c ltablib.c ltm.c lundump.c lvm.c lzio.c print.c

LUA_INC=    lapi.h lauxlib.h lcode.h ldebug.h ldo.h lfunc.h lgc.h llex.h llimits.h lmem.h lobject.h\
            lopcodes.h lparser.h lstate.h lstring.h ltable.h ltm.h lua.h luaconf.h lualib.h lundump.h\
            lvm.h lzio.h
LUA_FULL_INC=$(foreach inc,$(LUA_INC),lua/$(inc))
LUA_FULL_SRC=$(foreach src,$(LUA_SRC),lua/$(src))
LUA_FULL_OBJ=$(subst .c,.o,$(LUA_FULL_SRC))
LUA_FULL_LIB=lua/lua.a
LUA_RPC_FILE=protocol_lua.h protocol_lua.clt.c protocol_lua.svr.c

TOMMATH_SRC=bn_error.c bn_fast_mp_invmod.c bn_fast_mp_montgomery_reduce.c bn_fast_s_mp_mul_digs.c bn_fast_s_mp_mul_high_digs.c \
			bn_fast_s_mp_sqr.c bn_mp_2expt.c bn_mp_abs.c bn_mp_add.c bn_mp_add_d.c bn_mp_addmod.c bn_mp_and.c bn_mp_clamp.c \
			bn_mp_clear.c bn_mp_clear_multi.c bn_mp_cmp.c bn_mp_cmp_d.c bn_mp_cmp_mag.c bn_mp_cnt_lsb.c bn_mp_copy.c bn_mp_count_bits.c \
			bn_mp_div.c bn_mp_div_2.c bn_mp_div_2d.c bn_mp_div_3.c bn_mp_div_d.c bn_mp_dr_is_modulus.c bn_mp_dr_reduce.c bn_mp_dr_setup.c \
			bn_mp_exch.c bn_mp_expt_d.c bn_mp_exptmod.c bn_mp_exptmod_fast.c bn_mp_exteuclid.c bn_mp_fread.c bn_mp_fwrite.c bn_mp_gcd.c \
			bn_mp_get_int.c bn_mp_grow.c bn_mp_init.c bn_mp_init_copy.c bn_mp_init_multi.c bn_mp_init_set.c bn_mp_init_set_int.c \
			bn_mp_init_size.c bn_mp_invmod.c bn_mp_invmod_slow.c bn_mp_is_square.c bn_mp_jacobi.c bn_mp_karatsuba_mul.c bn_mp_karatsuba_sqr.c \
			bn_mp_lcm.c bn_mp_lshd.c bn_mp_mod.c bn_mp_mod_2d.c bn_mp_mod_d.c bn_mp_montgomery_calc_normalization.c bn_mp_montgomery_reduce.c \
			bn_mp_montgomery_setup.c bn_mp_mul.c bn_mp_mul_2.c bn_mp_mul_2d.c bn_mp_mul_d.c bn_mp_mulmod.c bn_mp_n_root.c bn_mp_neg.c bn_mp_or.c \
			bn_mp_prime_fermat.c bn_mp_prime_is_divisible.c bn_mp_prime_is_prime.c bn_mp_prime_miller_rabin.c bn_mp_prime_next_prime.c \
			bn_mp_prime_rabin_miller_trials.c bn_mp_prime_random_ex.c bn_mp_radix_size.c bn_mp_radix_smap.c bn_mp_rand.c bn_mp_read_radix.c \
			bn_mp_read_signed_bin.c bn_mp_read_unsigned_bin.c bn_mp_reduce.c bn_mp_reduce_2k.c bn_mp_reduce_2k_l.c bn_mp_reduce_2k_setup.c \
			bn_mp_reduce_2k_setup_l.c bn_mp_reduce_is_2k.c bn_mp_reduce_is_2k_l.c bn_mp_reduce_setup.c bn_mp_rshd.c bn_mp_set.c bn_mp_set_int.c \
			bn_mp_shrink.c bn_mp_signed_bin_size.c bn_mp_sqr.c bn_mp_sqrmod.c bn_mp_sqrt.c bn_mp_sub.c bn_mp_sub_d.c bn_mp_submod.c \
			bn_mp_to_signed_bin.c bn_mp_to_signed_bin_n.c bn_mp_to_unsigned_bin.c bn_mp_to_unsigned_bin_n.c bn_mp_toom_mul.c bn_mp_toom_sqr.c \
			bn_mp_toradix.c bn_mp_toradix_n.c bn_mp_unsigned_bin_size.c bn_mp_xor.c bn_mp_zero.c bn_prime_tab.c bn_reverse.c bn_s_mp_add.c \
			bn_s_mp_exptmod.c bn_s_mp_mul_digs.c bn_s_mp_mul_high_digs.c bn_s_mp_sqr.c bn_s_mp_sub.c bncore.c
TOMMATH_INC=
TOMMATH_FULL_INC=$(foreach inc,$(TOMMATH_INC),tommath/$(inc))
TOMMATH_FULL_SRC=$(foreach src,$(TOMMATH_SRC),tommath/$(src))
TOMMATH_FULL_OBJ=$(subst .c,.o,$(TOMMATH_FULL_SRC))
TOMMATH_FULL_LIB=tommath/tommath.a 

INCS=$(OS_INC_DIR) $(APPBOX_INC) -DDBAPI_MYSQL_$(DBAPI_MYSQL) -I../../inc
LIBS=$(OS_LIB_DIR) $(APPBOX_LIB) $(OS_LIB)
OBJS=		os_unix.o misc.o mempool.o hashmap.o sock.o log.o dymempool.o stream.o\
			protocol_lua.clt.o protocol_lua.svr.o\
			protocol.o protocol_lua.o protocol_lua_client.o protocol_lua_debug.o\
			dbapi.o dbapi_sqlite.o dbapi_mysql.o threadpool.o timer.o fdwatch_$(FDWATCH_USE).o\
			sha1.o bignum.o srp6a.o\
			console.o network.o rqueue.o rpc_net.o rpc_fun.o appbox.o applog.o appbox_unix.o appbox_args.o
MAIN_OBJS=appbox_main.o
CSMNG_OBJS=csmng.o
CSCLT_OBJS=csclt.o
.c.o:
	$(CC) $(CC_FLAG) $(INCS) -DXMALLOC=xmalloc -DMODULE_NAME=\"appbox\" -DAPPBOX_MAIN $< $(OBJ_CC_FLAG) $@
#	$(CC) $(CC_FLAG) $(INCS) -DMODULE_NAME=\"appbox\" -DAPPBOX_MAIN $< -S $(OBJ_CC_FLAG) $@.S

%.h: %.idl; $(OUTPUT_DIR)/idl_cc $<
%.clt.c: %.idl; $(OUTPUT_DIR)/idl_cc $<
%.svr.c: %.idl; $(OUTPUT_DIR)/idl_cc $<

all: $(OUTPUT_DIR)/appbox$(STA_LIB_PREFIX) $(OUTPUT_DIR)/appbox$(DLL_PREFIX) $(OUTPUT_DIR)/appbox_main$(EXE_PREFIX) $(OUTPUT_DIR)/csmng$(EXE_PREFIX) $(OUTPUT_DIR)/csclt$(EXE_PREFIX)

clean:
	rm -rf *.o lua/*.o sqlite/*.o tommath/*.o
	rm -rf *.S lua/*.S sqlite/*.S tommath/*.S
	rm -rf *.a lua/*.a sqlite/*.a tommath/*.a
	rm -rf $(SQLITE_FULL_LIB) $(LUA_FULL_OBJ) $(LUA_FULL_LIB)
	rm -rf $(OUTPUT_DIR)/appbox_main$(EXE_PREFIX) $(OUTPUT_DIR)/csmng$(EXE_PREFIX)
	rm -rf $(OUTPUT_DIR)/appbox$(DLL_PREFIX) $(OUTPUT_DIR)/appbox$(STA_LIB_PREFIX)
	rm -rf $(LUA_RPC_FILE)

$(OUTPUT_DIR)/csmng$(EXE_PREFIX): $(OUTPUT_DIR)/appbox$(STA_LIB_PREFIX) $(CSMNG_OBJS)
	$(CC) $(CC_FLAG) $(CSMNG_OBJS) $(OUTPUT_DIR)/appbox$(STA_LIB_PREFIX) $(OS_LIB) $(EXE_CC_FLAG) $@

$(OUTPUT_DIR)/csclt$(EXE_PREFIX): $(OUTPUT_DIR)/appbox$(STA_LIB_PREFIX) $(CSCLT_OBJS)
	$(CC) $(CC_FLAG) $(CSCLT_OBJS) $(OUTPUT_DIR)/appbox$(STA_LIB_PREFIX) $(OS_LIB) $(EXE_CC_FLAG) $@

$(OUTPUT_DIR)/appbox_main$(EXE_PREFIX): $(OUTPUT_DIR)/appbox$(DLL_PREFIX) $(MAIN_OBJS)
	cp -f $(OUTPUT_DIR)/appbox$(DLL_PREFIX) .
	$(CC) $(CC_FLAG) ./appbox$(DLL_PREFIX) $(MAIN_OBJS) $(OS_LIB) $(EXE_CC_FLAG) appbox_main$(EXE_PREFIX)
	cp -f appbox_main$(EXE_PREFIX) $(OUTPUT_DIR)/
	rm -f appbox$(DLL_PREFIX) appbox_main$(EXE_PREFIX)

$(OUTPUT_DIR)/appbox$(DLL_PREFIX): $(LUA_RPC_FILE) $(OBJS) $(SQLITE_FULL_LIB) $(LUA_FULL_LIB) $(TOMMATH_FULL_LIB)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(CC_FLAG) $(OBJS) $(SQLITE_FULL_LIB) $(LUA_FULL_LIB) $(TOMMATH_FULL_OBJ) $(LIBS) $(DLL_CC_FLAG) $@
	rm -f appbox$(DLL_PREFIX)

$(OUTPUT_DIR)/appbox$(STA_LIB_PREFIX): $(LUA_RPC_FILE) $(OBJS) $(SQLITE_FULL_LIB) $(LUA_FULL_LIB)
	$(STA_LIB) $@ $(OBJS) $(SQLITE_FULL_OBJ) $(LUA_FULL_OBJ)

$(SQLITE_FULL_LIB): $(SQLITE_FULL_INC) $(SQLITE_FULL_SRC)
	cd sqlite; $(MAKE) sqlite.a

$(LUA_FULL_LIB): $(LUA_FULL_INC) $(LUA_FULL_SRC)
	cd lua; $(MAKE) lua.a

$(LUA_FULL_LIB): $(LUA_FULL_INC) $(LUA_FULL_SRC)
	cd lua; $(MAKE) lua.a

$(TOMMATH_FULL_LIB): $(TOMMATH_FULL_INC) $(TOMMATH_FULL_SRC)
	cd tommath; $(MAKE) tommath.a

