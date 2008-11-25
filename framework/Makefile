BASE_DIR=.

include $(BASE_DIR)/Makefile.def

all:
	cd $(BASE_DIR); mkdir $(OUTPUT_DIR); mkdir $(OUTPUT_DIR)/modules; cd ..
	cd src/skates; $(MAKE) $@
	cd tools; $(MAKE) $@
	cd test/skates; $(MAKE) $@
	cd test/tools; $(MAKE) $@

clean:
	cd src/skates; $(MAKE) $@
	cd tools; $(MAKE) $@
	cd test/skates; $(MAKE) $@
	cd test/tools; $(MAKE) $@

linux:
	cp -f Makefile.def.$@ Makefile.def

freebsd:
	cp -f Makefile.def.$@ Makefile.def

macosx:
	cp -f Makefile.def.$@ Makefile.def

