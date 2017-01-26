// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_FRAME_INCLUDED
#define CMINOR_MACHINE_FRAME_INCLUDED
#include <cminor/machine/OperandStack.hpp>
#include <cminor/machine/LocalVariable.hpp>
#include <cminor/machine/VariableReference.hpp>

namespace cminor { namespace machine {

void SetDebugging();

class Machine;
class ManagedMemoryPool;
class ConstantPool;
class Function;
class Instruction;

class Frame
{
public:
    Frame(uint64_t size_, Thread& thread_, Function& fun_);
    ~Frame();
    Thread& GetThread() { return thread; }
    Function& Fun() { return fun; }
    ConstantPool& GetConstantPool();
    OperandStack& OpStack();
    const OperandStack& OpStack() const;
    int NumLocals() const;
    LocalVariable& Local(int32_t index) { Assert(index >= 0 && index <= NumLocals(), "invalid local variable index"); return locals[index]; }
    uint32_t Id() const { return id; }
    Instruction* GetNextInst();
    int32_t PC() const { return pc; }
    void SetPC(int32_t pc_);
    int32_t PrevPC() const { return prevPC; }
    void AddVariableReference(VariableReference* variableReference);
    bool HasBreakpointAt(int32_t pc) const;
    void SetBreakpointAt(int32_t pc);
    void RemoveBreakpointAt(int32_t pc);
    uint64_t Size() const { return size; }
private:
    uint64_t size;
    Thread& thread;
    Function& fun;
    int32_t id;
    int32_t debugContextId;
    int32_t pc;
    int32_t prevPC;
    std::vector<std::unique_ptr<VariableReference>> variableReferences;
    LocalVariable* locals;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FRAME_INCLUDED
