// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_FUNCTION_INCLUDED
#define CMINOR_MACHINE_FUNCTION_INCLUDED
#include <cminor/machine/Constant.hpp>
#include <cminor/machine/Instruction.hpp>
#include <ostream>

namespace cminor { namespace machine {

class Emitter
{
public:
    virtual ~Emitter();
    virtual void SetFirstInstIndex(int32_t index) = 0;
    virtual int32_t FistInstIndex() const = 0;
};

class Function
{
public:
    Function();
    Function(Constant fullName_, int32_t id_, ConstantPool* constantPool_);
    Constant FullName() const { return fullName; }
    int32_t Id() const { return id; }
    ConstantPool& GetConstantPool() { Assert(constantPool, "constant pool not set"); return *constantPool; }
    void SetConstantPool(ConstantPool* constantPool_) { constantPool = constantPool_; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    int32_t NumLocals() const { return numLocals; }
    void SetNumLocals(int32_t numLocals_);
    int32_t NumParameters() const { return numParameters; }
    void SetNumParameters(int32_t numParameters_);
    int NumInsts() const { return int(instructions.size()); }
    Instruction* GetInst(int index) const { return instructions[index].get(); }
    void AddInst(std::unique_ptr<Instruction>&& inst);
    void Dump(CodeFormatter& formatter);
    bool IsMain() const { return isMain; }
    void SetMain() { isMain = true; }
    void SetEmitter(Emitter* emitter_) { emitter = emitter_; }
    Emitter* GetEmitter() const { return emitter; }
private:
    Constant fullName;
    int32_t id;
    ConstantPool* constantPool;
    std::vector<std::unique_ptr<Instruction>> instructions;
    int32_t numLocals;
    int32_t numParameters;
    bool isMain;
    Emitter* emitter;
};

class FunctionTable
{
public:
    static void Init();
    FunctionTable();
    static FunctionTable& Instance() { Assert(instance, "function table not initialized");  return *instance; }
    void AddFunction(Function* fun);
    Function* GetFunction(StringPtr functionFullName) const;
    Function* GetMain() const { return main; }
private:
    static std::unique_ptr<FunctionTable> instance;
    std::unordered_map<StringPtr, Function*, StringPtrHash> functionMap;
    Function* main;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FUNCTION_INCLUDED
