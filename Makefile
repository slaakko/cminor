all: bld

bld:
	$(MAKE) -C ast
	$(MAKE) -C binder
	$(MAKE) -C build
	$(MAKE) -C emitter
	$(MAKE) -C machine
	$(MAKE) -C parser
	$(MAKE) -C pl
	$(MAKE) -C pom
	$(MAKE) -C symbols
	$(MAKE) -C util
	$(MAKE) -C buildsys
	$(MAKE) -C cminor
	$(MAKE) -C cminor2html
	$(MAKE) -C compiler
	$(MAKE) -C vmlib
	$(MAKE) -C db
	$(MAKE) -C dump
	$(MAKE) -C rt
	$(MAKE) -C vm
	$(MAKE) -C cpg/cpgcpp
	$(MAKE) -C cpg/cpgsyntax
	$(MAKE) -C cpg/cpg

prefix := /usr

install:
	mkdir -p $(prefix)/bin
	cp bin/* $(prefix)/bin

clean:
	$(MAKE) -C ast clean
	$(MAKE) -C binder clean
	$(MAKE) -C build clean
	$(MAKE) -C emitter clean
	$(MAKE) -C machine clean
	$(MAKE) -C parser clean
	$(MAKE) -C pl clean
	$(MAKE) -C pom clean
	$(MAKE) -C symbols clean
	$(MAKE) -C util clean
	$(MAKE) -C buildsys clean
	$(MAKE) -C cminor clean
	$(MAKE) -C cminor2html clean
	$(MAKE) -C compiler clean
	$(MAKE) -C vmlib clean
	$(MAKE) -C db clean
	$(MAKE) -C dump clean
	$(MAKE) -C rt clean
	$(MAKE) -C vm clean
	$(MAKE) -C cpg/cpgcpp clean
	$(MAKE) -C cpg/cpgsyntax clean
	$(MAKE) -C cpg/cpg clean

