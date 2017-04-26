.PHONY : build install clean

all: build

build:
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

ifeq ($(config),debug)
    machinesoname=libcminormachined
else
    machinesoname=libcminormachine
endif

install:
	mkdir -p $(prefix)/bin
	cp bin/* $(prefix)/bin
	cp lib/$(machinesoname).so.0.1.0 $(prefix)/lib
	ln -f $(prefix)/lib/$(machinesoname).so.0.1.0 $(prefix)/lib/$(machinesoname).so.0
	ln -f $(prefix)/lib/$(machinesoname).so.0.1.0 $(prefix)/lib/$(machinesoname).so

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

