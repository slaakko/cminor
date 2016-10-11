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

class JumpInst : public IndexParamInst
{
public:
    JumpInst();
    Instruction* Clone() const override { return new JumpInst(); };
    void Execute(Frame& frame) override;
};

class JumpTrueInst : public IndexParamInst
{
public:
    JumpTrueInst();
    Instruction* Clone() const override { return new JumpTrueInst(); };
    void Execute(Frame& frame) override;
};

class JumpFalseInst : public IndexParamInst
{
public:
    JumpFalseInst();
    Instruction* Clone() const override { return new JumpFalseInst(); };
    void Execute(Frame& frame) override;
};

class CallInst : public Instruction
{
public:
    CallInst();
    Instruction* Clone() const override { return new CallInst(*this); }
    void SetFunctionFullName(Constant functionFullName);
    StringPtr GetFunctionFullName() const;
    void SetFunction(Function* fun);
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
private:
    Constant function;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_INSTRUCTION_INCLUDED
