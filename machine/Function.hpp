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

class GenObject;

class Emitter
{
public:
    virtual ~Emitter();
    virtual void SetFirstInstIndex(int32_t index) = 0;
    virtual int32_t FistInstIndex() const = 0;
    virtual bool CreatePCRange() const = 0;
    virtual bool SetPCRangeEnd() const = 0;
    virtual void DoCreatePCRange(int32_t start) = 0;
    virtual void DoSetPCRangeEnd(int32_t end) = 0;
    virtual void Visit(GenObject& genObject) = 0;
    virtual void BackpatchConDis(int32_t target) = 0;
};

class Function
{
public:
    Function();
    Function(Constant callName_, Constant friendlyName_, uint32_t id_, ConstantPool* constantPool_);
    Constant CallName() const { return callName; }
    Constant FriendlyName() const { return friendlyName; }
    uint32_t Id() const { return id; }
    ConstantPool& GetConstantPool() { Assert(constantPool, "constant pool not set"); return *constantPool; }
    void SetConstantPool(ConstantPool* constantPool_) { constantPool = constantPool_; }
    void Write(Writer& writer);
    void Read(Reader& reader);
    uint32_t NumLocals() const { return numLocals; }
    void SetNumLocals(uint32_t numLocals_);
    uint32_t NumParameters() const { return numParameters; }
    void SetNumParameters(uint32_t numParameters_);
    int NumInsts() const { return int(instructions.size()); }
    Instruction* GetInst(int index) const { return instructions[index].get(); }
    void AddInst(std::unique_ptr<Instruction>&& inst);
    void SetInst(int32_t index, std::unique_ptr<Instruction>&& inst);
    void Dump(CodeFormatter& formatter);
    bool IsMain() const { return isMain; }
    void SetMain() { isMain = true; }
    void SetEmitter(Emitter* emitter_) { emitter = emitter_; }
    Emitter* GetEmitter() const { return emitter; }
private:
    Constant callName;
    Constant friendlyName;
    uint32_t id;
    ConstantPool* constantPool;
    std::vector<std::unique_ptr<Instruction>> instructions;
    uint32_t numLocals;
    uint32_t numParameters;
    bool isMain;
    Emitter* emitter;
};

class FunctionTable
{
public:
    static void Init();
    static void Done();
    FunctionTable();
    static FunctionTable& Instance() { Assert(instance, "function table not initialized"); return *instance; }
    void AddFunction(Function* fun);
    Function* GetFunction(StringPtr functionCallName) const;
    Function* GetMain() const { return main; }
private:
    static std::unique_ptr<FunctionTable> instance;
    std::unordered_map<StringPtr, Function*, StringPtrHash> functionMap;
    Function* main;
};

class VmFunction
{
public:
    void SetName(Constant name_) { name = name_; }
    Constant Name() const { return name; }
    virtual ~VmFunction();
    virtual void Execute(Frame& frame) = 0;
private:
    Constant name;
};

class VmFunctionTable
{
public:
    static void Init();
    static void Done();
    static VmFunctionTable& Instance() { Assert(instance, "virtual machine function table not initialized"); return *instance; }
    void RegisterVmFunction(VmFunction* vmFunction);
    VmFunction* GetVmFunction(StringPtr vmFuntionName) const;
private:
    static std::unique_ptr<VmFunctionTable> instance;
    std::unordered_map<StringPtr, VmFunction*, StringPtrHash> vmFunctionMap;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_FUNCTION_INCLUDED
