include ../Makefile.common

OBJECTS = Emitter.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../lib/libcminoremitter.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../lib/libcminoremitter.a

