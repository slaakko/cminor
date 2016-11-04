// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_INSTRUCTION_INCLUDED
#define CMINOR_MACHINE_INSTRUCTION_INCLUDED
#include <cminor/machine/CodeFormatter.hpp>
#include <cminor/machine/Reader.hpp>
#include <cminor/machine/Writer.hpp>
#include <cminor/machine/Error.hpp>
#include <cminor/machine/Frame.hpp>
#include <cminor/machine/Constant.hpp>
#include <unordered_map>
#include <string>
#include <memory>

namespace cminor { namespace machine {

class Machine;
class Function;

class Instruction
{
public:
    Instruction(const std::string& name_);
    Instruction(const std::string& name_, const std::string& groupName_, const std::string& typeName_);
    Instruction(const Instruction&) = default;
    virtual ~Instruction();
    virtual Instruction* Clone() const = 0;
    virtual void Encode(Writer& writer);
    virtual Instruction* Decode(Reader& reader);
    virtual void SetIndex(int32_t index_);
    void SetParent(Instruction* parent_) { parent = parent_; }
    void SetOpCode(uint8_t opCode_) { opCode = opCode_; }
    uint8_t OpCode() const { return opCode; }
    const std::string& Name() const { return name; }
    const std::string& GroupName() const { return groupName; }
    const std::string& TypeName() const { return typeName; }
    virtual void Execute(Frame& frame);
    virtual void Dump(CodeFormatter& formatter);
    virtual bool IsRoot() const { return false; }
    virtual void GetOpCodes(std::string& opCodes);
    virtual void SetTarget(int32_t target);
private:
    uint8_t opCode;
    std::string name;
    std::string groupName;
    std::string typeName;
    Instruction* parent;
};

class InvalidInst : public Instruction
{
public:
    InvalidInst();
    Instruction* Clone() const override { return new InvalidInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame);
};

class NopInst : public Instruction
{
public:
    NopInst();
    Instruction* Clone() const override { return new NopInst(*this); }
};

class ContainerInst : public Instruction
{
public:
    ContainerInst(Machine& machine_, const std::string& name_, bool root_);
    void SetInst(uint8_t opCode, Instruction* inst);
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    Instruction* Clone() const override { Assert(false, "not cloneable"); return nullptr; }
    bool IsRoot() const override { return root; }
private:
    bool root;
    Machine& machine;
    std::vector<std::unique_ptr<Instruction>> childInsts;
};

class InstructionTypeGroup
{
public:
    InstructionTypeGroup();
    InstructionTypeGroup(const std::string& instGroupName_);
    void AddInst(Instruction* inst);
    std::unique_ptr<Instruction> CreateInst(const std::string& typeName) const;
    const std::string& InstGroupName() const { return instGroupName; }
private:
    std::string instGroupName;
    std::unordered_map<std::string, Instruction*> instructionMap;
};


template<ValueType type>
class LoadDefaultValueInst : public Instruction
{
public:
    LoadDefaultValueInst(const std::string& name_, const std::string& typeName_) : Instruction(name_, "def", typeName_)
    {
    }
    Instruction* Clone() const override { return new LoadDefaultValueInst<type>(*this); }
    void Execute(Frame& frame) override
    {
        frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(0), type));
    }
};

template<typename OperandT, typename UnaryOpT, ValueType type>
class UnaryOpInst : public Instruction
{
public:
    UnaryOpInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_) : Instruction(name_, groupName_, typeName_)
    {
    }
    Instruction* Clone() const override { return new UnaryOpInst<OperandT, UnaryOpT, type>(*this); }
    void Execute(Frame& frame) override
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == type, ValueTypeStr(type) + " operand expected");
        OperandT value = static_cast<OperandT>(operand.Value());
        frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(UnaryOpT()(value)), type));
    }
};

template<typename OperandT, typename BinaryOpT, ValueType type>
class BinaryOpInst : public Instruction
{
public:
    BinaryOpInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_) : Instruction(name_, groupName_, typeName_)
    {
    }
    Instruction* Clone() const override { return new BinaryOpInst<OperandT, BinaryOpT, type>(*this); }
    void Execute(Frame& frame) override
    {
        IntegralValue rightOperand = frame.OpStack().Pop();
        Assert(rightOperand.GetType() == type, ValueTypeStr(type) + " operand expected");
        IntegralValue leftOperand = frame.OpStack().Pop();
        Assert(leftOperand.GetType() == type, ValueTypeStr(type) + " operand expected");
        OperandT left = static_cast<OperandT>(leftOperand.Value());
        OperandT right = static_cast<OperandT>(rightOperand.Value());
        frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(BinaryOpT()(left, right)), type));
    }
};

template<typename OperandT>
struct ShiftLeft
{
    OperandT operator()(const OperandT& left, const OperandT& right) const
    {
        return left << right;
    }
};

template<typename OperandT>
struct ShiftRight
{
    OperandT operator()(const OperandT& left, const OperandT& right) const
    {
        return left >> right;
    }
};

template<typename OperandT, typename RelationT, ValueType type>
class BinaryPredInst : public Instruction
{
public:
    BinaryPredInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_) : Instruction(name_, groupName_, typeName_)
    {
    }
    Instruction* Clone() const override { return new BinaryPredInst<OperandT, RelationT, type>(*this); }
    void Execute(Frame& frame) override
    {
        IntegralValue rightOperand = frame.OpStack().Pop();
        Assert(rightOperand.GetType() == type, ValueTypeStr(type) + " operand expected");
        IntegralValue leftOperand = frame.OpStack().Pop();
        Assert(leftOperand.GetType() == type, ValueTypeStr(type) + " operand expected");
        OperandT left = static_cast<OperandT>(leftOperand.Value());
        OperandT right = static_cast<OperandT>(rightOperand.Value());
        bool result = RelationT()(left, right);
        frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
    }
};

class LogicalNotInst : public Instruction
{
public:
    LogicalNotInst();
    Instruction* Clone() const override { return new LogicalNotInst(*this); }
    void Execute(Frame& frame) override;
};

class IndexParamInst : public Instruction
{
public:
    IndexParamInst(const std::string& name_);
    IndexParamInst(const IndexParamInst&) = default;
    void SetIndex(int32_t index_) override { index = index_; }
    int32_t Index() const { return index; }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Dump(CodeFormatter& formatter) override;
private:
    int32_t index;
};

class LoadLocalInst : public IndexParamInst
{
public:
    LoadLocalInst();
    Instruction* Clone() const override { return new LoadLocalInst(*this); }
    void Execute(Frame& frame) override;
};

class StoreLocalInst : public IndexParamInst
{
public:
    StoreLocalInst();
    Instruction* Clone() const override { return new StoreLocalInst(*this); }
    void Execute(Frame& frame) override;
};

class LoadFieldInst : public IndexParamInst
{
public:
    LoadFieldInst();
    Instruction* Clone() const override { return new LoadFieldInst(*this); }
    void Execute(Frame& frame) override;
};

class StoreFieldInst : public IndexParamInst
{
public:
    StoreFieldInst();
    Instruction* Clone() const override { return new StoreFieldInst(*this); }
    void Execute(Frame& frame) override;
};

class LoadElemInst : public Instruction
{
public:
    LoadElemInst();
    Instruction* Clone() const override { return new LoadElemInst(*this); }
    void Execute(Frame& frame) override;
};

class StoreElemInst : public Instruction
{
public:
    StoreElemInst();
    Instruction* Clone() const override { return new StoreElemInst(*this); }
    void Execute(Frame& frame) override;
};

class LoadConstantInst : public IndexParamInst
{
public:
    LoadConstantInst();
    Instruction* Clone() const override { return new LoadConstantInst(*this); }
    void Execute(Frame& frame) override;
};

class ReceiveInst : public Instruction
{
public:
    ReceiveInst();
    Instruction* Clone() const override { return new ReceiveInst(*this); }
    void Execute(Frame& frame) override;
};

template<typename SourceT, typename TargetT, ValueType type>
class ConversionInst : public Instruction
{
public:
    ConversionInst(const std::string& name_) : Instruction(name_)
    {
    }
    Instruction* Clone() const override { return new ConversionInst<SourceT, TargetT, type>(*this); }
    void Execute(Frame& frame) override
    {
        SourceT source = static_cast<SourceT>(frame.OpStack().Pop().Value());
        TargetT target = static_cast<TargetT>(source);
        frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(target), type));
    }
};

const int32_t endOfFunction = -1;

class JumpInst : public IndexParamInst
{
public:
    JumpInst();
    Instruction* Clone() const override { return new JumpInst(*this); };
    void SetTarget(int32_t target) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
};

class JumpTrueInst : public IndexParamInst
{
public:
    JumpTrueInst();
    Instruction* Clone() const override { return new JumpTrueInst(*this); };
    void SetTarget(int32_t target) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
};

class JumpFalseInst : public IndexParamInst
{
public:
    JumpFalseInst();
    Instruction* Clone() const override { return new JumpFalseInst(*this); };
    void SetTarget(int32_t target) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
};

class EnterBlockInst : public Instruction
{
public:
    EnterBlockInst();
    Instruction* Clone() const override { return new EnterBlockInst(*this); };
    void Execute(Frame& frame) override;
};

class ExitBlockInst : public Instruction
{
public:
    ExitBlockInst();
    Instruction* Clone() const override { return new ExitBlockInst(*this); };
    void Execute(Frame& frame) override;
};

class CallInst : public Instruction
{
public:
    CallInst();
    Instruction* Clone() const override { return new CallInst(*this); }
    void SetFunctionCallName(Constant functionCallName);
    StringPtr GetFunctionCallName() const;
    void SetFunction(Function* fun);
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
private:
    Constant function;
};

class VirtualCallInst : public Instruction
{
public:
    VirtualCallInst();
    void SetNumArgs(int32_t numArgs_) { numArgs = numArgs_; }
    void SetVmtIndex(int32_t vmtIndex_) { vmtIndex = vmtIndex_; }
    Instruction* Clone() const override { return new VirtualCallInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
private:
    int32_t numArgs;
    int32_t vmtIndex;
};

class InterfaceCallInst : public Instruction
{
public:
    InterfaceCallInst();
    Instruction* Clone() const override { return new InterfaceCallInst(*this); }
    void SetNumArgs(int32_t numArgs_) { numArgs = numArgs_; };
    void SetImtIndex(int32_t imtIndex_) { imtIndex = imtIndex_; }
    void Execute(Frame& frame) override;
private:
    int32_t numArgs;
    int32_t imtIndex;
};

class VmCallInst : public IndexParamInst
{
public:
    VmCallInst();
    Instruction* Clone() const override { return new VmCallInst(*this); }
    void Execute(Frame& frame) override;
};

class SetClassDataInst : public Instruction
{
public:
    SetClassDataInst();
    Instruction* Clone() const override { return new SetClassDataInst(*this); }
    void SetClassName(Constant fullClassName);
    StringPtr GetClassName() const;
    void SetClassData(ClassData* classDataPtr);
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
private:
    Constant classData;
};

class TypeInstruction : public Instruction
{
public:
    TypeInstruction(const std::string& name_);
    void SetTypeName(Constant fullTypeName);
    StringPtr GetTypeName() const;
    void SetType(Type* typePtr);
    Type* GetType();
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Dump(CodeFormatter& formatter) override;
private:
    Constant type;
};

class CreateObjectInst : public TypeInstruction
{
public:
    CreateObjectInst();
    Instruction* Clone() const override { return new CreateObjectInst(*this); }
    void Execute(Frame& frame) override;
};

class CopyObjectInst : public Instruction
{
public:
    CopyObjectInst();
    Instruction* Clone() const override { return new CopyObjectInst(*this); }
    void Execute(Frame& frame) override;
};

class StrLitToStringInst : public Instruction
{
public:
    StrLitToStringInst();
    Instruction* Clone() const override { return new StrLitToStringInst(*this); }
    void Execute(Frame& frame) override;
};

class LengthStringInst : public Instruction
{
public:
    LengthStringInst();
    Instruction* Clone() const override { return new LengthStringInst(*this); }
    void Execute(Frame& frame) override;
};

class DupInst : public Instruction
{
public:
    DupInst();
    Instruction* Clone() const override { return new DupInst(*this); }
    void Execute(Frame& frame) override;
};

class SwapInst : public Instruction
{
public:
    SwapInst();
    Instruction* Clone() const override { return new SwapInst(*this); }
    void Execute(Frame& frame) override;
};

class RotateInst : public Instruction
{
public:
    RotateInst();
    Instruction* Clone() const override { return new RotateInst(*this); }
    void Execute(Frame& frame) override;
};

class UpCastInst : public TypeInstruction
{
public:
    UpCastInst();
    Instruction* Clone() const override { return new UpCastInst(*this); }
    void Execute(Frame& frame) override;
};

class DownCastInst : public TypeInstruction
{
public:
    DownCastInst();
    Instruction* Clone() const override { return new DownCastInst(*this); }
    void Execute(Frame& frame) override;
};

class AllocateArrayElementsInst : public TypeInstruction
{
public:
    AllocateArrayElementsInst();
    Instruction* Clone() const override { return new AllocateArrayElementsInst(*this); }
    void Execute(Frame& frame) override;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_INSTRUCTION_INCLUDED
