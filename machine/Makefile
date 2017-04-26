include ../Makefile.common

OBJECTS = Arena.o Class.o CminorException.o Constant.o Error.o FileRegistry.o Frame.o Function.o \
GarbageCollector.o GenObject.o Instruction.o LocalVariable.o Log.o Machine.o MachineFunctionVisitor.o \
Object.o OsInterface.o Reader.o Runtime.o Stack.o Stats.o Thread.o Type.o VariableReference.o Writer.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

ifeq ($(config),debug)
    soname=libcminormachined.so.0.1.0
    sobasename=libcminormachined
else
    soname=libcminormachine.so.0.1.0
    sobasename=libcminormachine
endif

../lib/$(soname): $(OBJECTS)
	$(CXX) -shared -o $@ $(OBJECTS)
	ln -f ../lib/$(soname) ../lib/$(sobasename).so.0
	ln -f ../lib/$(soname) ../lib/$(sobasename).so

clean:
	rm -f *.o
	rm -f ../lib/$(soname)

