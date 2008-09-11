
all:
	cd src/skates; $(MAKE) $@; cd ../..
	cd tools; $(MAKE) $@; cd ..
	cd test/skates; $(MAKE) $@; cd ../..

clean:
	cd src/skates; $(MAKE) $@; cd ../..
	cd tools; $(MAKE) $@; cd ..
	cd test/skates; $(MAKE) $@; cd ../..

linux:
	cp -f Makefile.def.$@ Makefile.def

freebsd:
	cp -f Makefile.def.$@ Makefile.def

macosx:
	cp -f Makefile.def.$@ Makefile.def

