include ../../Makefile.common

OBJECTS = CodeGenerator.o CodeGeneratorVisitor.o Compiler.o Composite.o Element.o Grammar.o Library.o LibraryFile.o ParserFileContent.o ParserFile.o Primary.o \
Primitive.o Project.o ProjectFile.o Rule.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../../lib/libcminorcpgsyntax.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../../lib/libcminorcpgsyntax.a

