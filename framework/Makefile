
all:
	cd src; $(MAKE) $@; cd ..
	cd tools; $(MAKE) $@; cd ..
	cd test; $(MAKE) $@; cd ..

clean:
	cd src; $(MAKE) $@; cd ..
	cd tools; $(MAKE) $@; cd ..
	cd test; $(MAKE) $@; cd ..

linux:
	cp -f Makefile.def.$@ Makefile.def

freebsd:
	cp -f Makefile.def.$@ Makefile.def

