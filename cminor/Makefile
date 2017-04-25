include ../Makefile.common

OBJECTS = Main.o

LIBRARIES = ../lib/libcminorutil.a

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

ifeq ($(config),debug)
    exename=cminord
else
    exename=cminor
endif

../bin/$(exename): $(OBJECTS) $(LIBRARIES)
	$(CXX) $(CXXFLAGS) -o ../bin/$(exename) $(OBJECTS) \
-lcminorutil -lboost_filesystem -lboost_iostreams -lboost_system

install:
	mkdir -p $(prefix)/bin
	cp ../bin/$(exename) $(prefix)/bin

clean:
	rm -f *.o
	rm -f ../bin/$(exename)

