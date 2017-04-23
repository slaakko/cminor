include ../Makefile.common

OBJECTS = Access.o BoundClass.o BoundCompileUnit.o BoundExpression.o BoundFunction.o BoundNode.o BoundNodeVisitor.o BoundStatement.o \
ClassTemplateRepository.o ControlFlowAnalyzer.o Evaluator.o ExpressionBinder.o OverloadResolution.o StatementBinderVisitor.o TypeBinderVisitor.o TypeResolver.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../lib/libcminorbinder.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../lib/libcminorbinder.a

