include ../Makefile.common

ifeq ($(config),debug)
    machinesoname=cminormachined
else
    machinesoname=cminormachine
endif

OBJECTS = Command.o CommandGrammar.o Main.o Shell.o

LIBRARIES = ../lib/libcminorast.a ../lib/libcminorbinder.a ../lib/libcminorbuild.a ../lib/libcminoremitter.a ../lib/lib$(machinesoname).so.0.1.0 ../lib/libcminorparser.a \
../lib/libcminorpl.a ../lib/libcminorpom.a ../lib/libcminorsymbols.a ../lib/libcminorutil.a ../lib/libcminorvmlib.a

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

ifeq ($(config),debug)
    exename=cminordbd
else
    exename=cminordb
endif

../bin/$(exename): $(OBJECTS) $(LIBRARIES)
	$(CXX) $(CXXFLAGS) -o ../bin/$(exename) $(OBJECTS) \
-lcminorvmlib -lcminorbuild -lcminoremitter -lcminorbinder -lcminorparser -lcminorsymbols -lcminorast -l$(machinesoname) -lcminorast -lcminorpl -lcminorpom -lcminorutil \
-lboost_filesystem -lboost_iostreams -lboost_system \
-ldl -ltinfo -lz

install:
	mkdir -p $(prefix)/bin
	cp ../bin/$(exename) $(prefix)/bin

clean:
	rm -f *.o
	rm -f ../bin/$(exename)

