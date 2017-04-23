include ../Makefile.common

OBJECTS = BasicType.o Class.o CompileUnit.o Constant.o Delegate.o Enumeration.o Expression.o Function.o Identifier.o Interface.o Keyword.o Literal.o Parameter.o \
ParsingContext.o ProjectFile.o SolutionFile.o SourceToken.o Specifier.o Statement.o Template.o TypeExpr.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../lib/libcminorparser.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../lib/libcminorparser.a

