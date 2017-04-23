include ../Makefile.common

OBJECTS = CodeFormatter.o MappedInputFile.o Mutex.o Path.o Prime.o Random.o Sha1.o System.o TextUtils.o Unicode.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../lib/libcminorutil.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../lib/libcminorutil.a

