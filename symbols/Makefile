include ../Makefile.common

OBJECTS = Assembly.o BasicTypeFun.o ConstantPoolInstallerVisitor.o ConstantSymbol.o DelegateSymbol.o EnumSymbol.o \
EnumTypeFun.o FunctionSymbol.o GlobalFlags.o IndexerSymbol.o MachineFunctionTable.o MappingSymbolVisitor.o ObjectFun.o \
PropertySymbol.o RefTypeFun.o StringFun.o Symbol.o SymbolCreatorVisitor.o SymbolReader.o SymbolTable.o SymbolWriter.o \
Value.o VariableSymbol.o Warning.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../lib/libcminorsymbols.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../lib/libcminorsymbols.a

