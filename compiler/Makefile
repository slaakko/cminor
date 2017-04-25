include ../Makefile.common

OBJECTS = Main.o

LIBRARIES = ../lib/libcminorast.a ../lib/libcminorbinder.a ../lib/libcminorbuild.a ../lib/libcminoremitter.a ../lib/libcminormachine.a ../lib/libcminorparser.a \
../lib/libcminorpl.a ../lib/libcminorpom.a ../lib/libcminorsymbols.a ../lib/libcminorutil.a

LLVMLIBS = $(shell llvm-config --libs)

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

ifeq ($(config),debug)
    exename=cminorcd
else
    exename=cminorc
endif

../bin/$(exename): $(OBJECTS) $(LIBRARIES)
	$(CXX) $(CXXFLAGS) -o ../bin/$(exename) $(OBJECTS) \
-lcminorbuild -lcminoremitter -lcminorbinder -lcminorparser -lcminorsymbols -lcminorast -lcminormachine -lcminorast -lcminorpl -lcminorpom -lcminorutil \
-lboost_filesystem -lboost_iostreams -lboost_system \
$(LLVMLIBS) -ldl -ltinfo -lz

install:
	mkdir -p $(prefix)/bin
	cp ../bin/$(exename) $(prefix)/bin

clean:
	rm -f *.o
	rm -f ../bin/$(exename)

