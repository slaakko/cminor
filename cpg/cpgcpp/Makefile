include ../../Makefile.common

OBJECTS = Declaration.o Declarator.o Expression.o Identifier.o Keyword.o Literal.o Statement.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../../lib/libcminorcpgcpp.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../../lib/libcminorcpgcpp.a

