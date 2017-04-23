include ../Makefile.common

OBJECTS = AstReader.o AstWriter.o BasicType.o Class.o Clone.o CompileUnit.o Constant.o Delegate.o Enumeration.o Expression.o Function.o Identifier.o Interface.o \
Literal.o Namespace.o Node.o Parameter.o Project.o Solution.o SourceToken.o Specifier.o Statement.o Template.o Visitor.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../lib/libcminorast.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../lib/libcminorast.a

