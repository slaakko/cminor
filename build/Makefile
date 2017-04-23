include ../Makefile.common

OBJECTS = Build.o NativeCompiler.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../lib/libcminorbinder.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../lib/libcminorbinder.a

