include ../../Makefile.common

OBJECTS = Main.o

LIBRARIES = ../../lib/libcminorcpgcpp.a ../../lib/libcminorcpgsyntax.a ../../lib/libcminorpl.a ../../lib/libcminorpom.a ../../lib/libcminorutil.a

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

ifeq ($(config),debug)
    exename=cminorcpgd
else
    exename=cminorcpg
endif

../../bin/$(exename): $(OBJECTS) $(LIBRARIES)
	$(CXX) $(CXXFLAGS) -o ../../bin/$(exename) $(OBJECTS) \
-lcminorcpgsyntax -lcminorcpgcpp -lcminorpl -lcminorpom -lcminorutil -lboost_filesystem -lboost_iostreams -lboost_system -ldl -ltinfo -lz

install:
	mkdir -p $(prefix)/bin
	cp ../bin/$(exename) $(prefix)/bin

clean:
	rm -f *.o
	rm -f ../../bin/$(exename)

