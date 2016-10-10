// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_FRAME_INCLUDED
#define CMINOR_MACHINE_FRAME_INCLUDED
#include <cminor/machine/OperandStack.hpp>
#include <cminor/machine/LocalVariable.hpp>

namespace cminor { namespace machine {

class Machine;
class ObjectPool;
class ConstantPool;
class Function;
class Instruction;

class Frame
{
public:
    Frame(Machine& machine_, Thread& thread_, Function& fun_);
    Machine& GetMachine() { return machine; }
    int32_t Id() const { return id; }
    Thread& GetThread() { return thread; }
    Function& Fun() { return fun; }
    ObjectPool& GetObjectPool() { return objectPool; }
    ConstantPool& GetConstantPool() { return constantPool; }
    OperandStack& OpStack() { return opStack; }
    const OperandStack& OpStack() const { return opStack; }
    const LocalVariableVector& Locals() const { return locals; }
    LocalVariable& Local(int32_t index) { return locals[index];  }
    Instruction* GetNextInst();
    int32_t PC() const { return pc; }
private:
    Machine& machine;
    int32_t id;
    Thread& thread;
    Function& fun;
    ObjectPool& objectPool;
    ConstantPool& constantPool;
    OperandStack opStack;
    LocalVariableVector locals;
    int32_t pc;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FRAME_INCLUDED
