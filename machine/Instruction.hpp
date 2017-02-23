// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_INSTRUCTION_INCLUDED
#define CMINOR_MACHINE_INSTRUCTION_INCLUDED
#include <cminor/machine/MachineApi.hpp>
#include <cminor/util/CodeFormatter.hpp>
#include <cminor/machine/Reader.hpp>
#include <cminor/machine/Writer.hpp>
#include <cminor/machine/Error.hpp>
#include <cminor/machine/Frame.hpp>
#include <cminor/machine/Constant.hpp>
#include <cminor/machine/Type.hpp>
#include <cminor/machine/Class.hpp>
#include <unordered_map>
#include <string>
#include <memory>

namespace cminor { namespace machine {

class Machine;
class Function;

MACHINE_API void ThrowSystemException(const SystemException& ex, Frame& frame);
MACHINE_API void ThrowNullReferenceException(const NullReferenceException& ex, Frame& frame);
MACHINE_API void ThrowIndexOutOfRangeException(const IndexOutOfRangeException& ex, Frame& frame);
MACHINE_API void ThrowArgumentOutOfRangeException(const ArgumentOutOfRangeException& ex, Frame& frame);
MACHINE_API void ThrowInvalidCastException(const InvalidCastException& ex, Frame& frame);
MACHINE_API void ThrowFileSystemException(const FileSystemError& ex, Frame& frame);
MACHINE_API void ThrowStackOverflowException(const StackOverflowException& ex, Frame& frame);

MACHINE_API Machine& GetMachine();
void SetMachine(Machine* machine_);
MACHINE_API ManagedMemoryPool& GetManagedMemoryPool();
void SetManagedMemoryPool(ManagedMemoryPool* managedMemoryPool_);

class MachineFunctionVisitor;

class MACHINE_API Instruction
{
public:
    Instruction(const std::string& name_);
    Instruction(const std::string& name_, const std::string& groupName_, const std::string& typeName_);
    Instruction(const Instruction&) = default;
    virtual void Clear();
    virtual ~Instruction();
    virtual Instruction* Clone() const = 0;
    virtual void Encode(Writer& writer);
    virtual Instruction* Decode(Reader& reader);
    virtual void SetIndex(int32_t index_);
    virtual void SetIndex(uint8_t index_);
    virtual void SetIndex(uint16_t index_);
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
    virtual bool IsJumpingInst() const { return false; }
    virtual bool EndsBasicBlock() const { return false; }
    virtual bool IsJump() const { return false; }
    virtual bool IsThrow() const { return false; }
    virtual bool IsEndEhInst() const { return false; }
    virtual bool IsContinuousSwitchInst() const { return false; }
    virtual bool IsBinarySearchSwitchInst() const { return false; }
    virtual bool DontRemove() const { return false; }
    virtual void DispatchTo(InstAdder& adder);
    virtual void Accept(MachineFunctionVisitor& visitor);
private:
    uint8_t opCode;
    std::string name;
    std::string groupName;
    std::string typeName;
    Instruction* parent;
};

class MACHINE_API InvalidInst : public Instruction
{
public:
    InvalidInst();
    Instruction* Clone() const override { return new InvalidInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame);
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API NopInst : public Instruction
{
public:
    NopInst();
    Instruction* Clone() const override { return new NopInst(*this); }
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API ContainerInst : public Instruction
{
public:
    ContainerInst(Machine& machine_, const std::string& name_, bool root_);
    ContainerInst(const ContainerInst&) = delete;
    ContainerInst& operator=(const ContainerInst&) = delete;
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

class MACHINE_API InstructionTypeGroup
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

class MACHINE_API LoadDefaultValueBaseInst : public Instruction
{
public:
    LoadDefaultValueBaseInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_);
    void Accept(MachineFunctionVisitor& visitor) override;
    virtual ValueType GetValueType() const = 0;
};

template<ValueType type>
class MACHINE_API LoadDefaultValueInst : public LoadDefaultValueBaseInst
{
public:
    LoadDefaultValueInst(const std::string& name_, const std::string& typeName_) : LoadDefaultValueBaseInst(name_, "def", typeName_)
    {
    }
    Instruction* Clone() const override { return new LoadDefaultValueInst<type>(*this); }
    ValueType GetValueType() const override { return type; }
    void Execute(Frame& frame) override
    {
        frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(0), type));
    }
};

class MACHINE_API UnaryOpBaseInst : public Instruction
{
public:
    UnaryOpBaseInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_);
    void Accept(MachineFunctionVisitor& visitor) override;
    virtual ValueType GetValueType() const = 0;
};

template<typename OperandT, typename UnaryOpT, ValueType type>
class MACHINE_API UnaryOpInst : public UnaryOpBaseInst
{
public:
    UnaryOpInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_) : UnaryOpBaseInst(name_, groupName_, typeName_)
    {
    }
    Instruction* Clone() const override { return new UnaryOpInst<OperandT, UnaryOpT, type>(*this); }
    ValueType GetValueType() const override { return type; }
    void Execute(Frame& frame) override
    {
        IntegralValue operand = frame.OpStack().Pop();
        Assert(operand.GetType() == type, ValueTypeStr(type) + " operand expected");
        OperandT value = static_cast<OperandT>(operand.Value());
        frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(UnaryOpT()(value)), type));
    }
};

class MACHINE_API BinaryOpBaseInst : public Instruction
{
public:
    BinaryOpBaseInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_);
    void Accept(MachineFunctionVisitor& visitor) override;
};

template<typename OperandT, typename BinaryOpT, ValueType type>
class MACHINE_API BinaryOpInst : public BinaryOpBaseInst
{
public:
    BinaryOpInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_) : BinaryOpBaseInst(name_, groupName_, typeName_)
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
struct MACHINE_API ShiftLeft
{
    OperandT operator()(const OperandT& left, const OperandT& right) const
    {
        return left << right;
    }
};

template<typename OperandT>
struct MACHINE_API ShiftRight
{
    OperandT operator()(const OperandT& left, const OperandT& right) const
    {
        return left >> right;
    }
};

class MACHINE_API BinaryPredBaseInst : public Instruction
{
public:
    BinaryPredBaseInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_);
    void Accept(MachineFunctionVisitor& visitor) override;
};

template<typename OperandT, typename RelationT, ValueType type>
class MACHINE_API BinaryPredInst : public BinaryPredBaseInst
{
public:
    BinaryPredInst(const std::string& name_, const std::string& groupName_, const std::string& typeName_) : BinaryPredBaseInst(name_, groupName_, typeName_)
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

class MACHINE_API LogicalNotInst : public Instruction
{
public:
    LogicalNotInst();
    Instruction* Clone() const override { return new LogicalNotInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API IndexParamInst : public Instruction
{
public:
    IndexParamInst(const std::string& name_);
    IndexParamInst(const IndexParamInst&) = default;
    void Clear() override;
    void SetIndex(int32_t index_) override { index = index_; }
    int32_t Index() const { return index; }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Dump(CodeFormatter& formatter) override;
private:
    int32_t index;
};

class MACHINE_API ByteParamInst : public Instruction
{
public:
    ByteParamInst(const std::string& name_);
    ByteParamInst(const ByteParamInst&) = default;
    void Clear() override;
    void SetIndex(uint8_t index_) override { index = index_; }
    uint8_t Index() const { return index; }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Dump(CodeFormatter& formatter) override;
private:
    uint8_t index;
};

class MACHINE_API UShortParamInst : public Instruction
{
public:
    UShortParamInst(const std::string& name_);
    UShortParamInst(const UShortParamInst&) = default;
    void Clear() override;
    void SetIndex(uint16_t index_) override { index = index_; }
    uint16_t Index() const { return index; }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Dump(CodeFormatter& formatter) override;
private:
    uint16_t index;
};

class MACHINE_API LoadLocalInst : public IndexParamInst
{
public:
    LoadLocalInst();
    Instruction* Clone() const override { return new LoadLocalInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API LoadLocal0Inst : public Instruction
{
public:
    LoadLocal0Inst();
    Instruction* Clone() const override { return new LoadLocal0Inst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API LoadLocal1Inst : public Instruction
{
public:
    LoadLocal1Inst();
    Instruction* Clone() const override { return new LoadLocal1Inst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API LoadLocal2Inst : public Instruction
{
public:
    LoadLocal2Inst();
    Instruction* Clone() const override { return new LoadLocal2Inst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API LoadLocal3Inst : public Instruction
{
public:
    LoadLocal3Inst();
    Instruction* Clone() const override { return new LoadLocal3Inst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API LoadLocalBInst : public ByteParamInst
{
public:
    LoadLocalBInst();
    Instruction* Clone() const override { return new LoadLocalBInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API LoadLocalSInst : public UShortParamInst
{
public:
    LoadLocalSInst();
    Instruction* Clone() const override { return new LoadLocalSInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API StoreLocalInst : public IndexParamInst
{
public:
    StoreLocalInst();
    Instruction* Clone() const override { return new StoreLocalInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API StoreLocal0Inst : public Instruction
{
public:
    StoreLocal0Inst();
    Instruction* Clone() const override { return new StoreLocal0Inst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API StoreLocal1Inst : public Instruction
{
public:
    StoreLocal1Inst();
    Instruction* Clone() const override { return new StoreLocal1Inst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API StoreLocal2Inst : public Instruction
{
public:
    StoreLocal2Inst();
    Instruction* Clone() const override { return new StoreLocal2Inst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API StoreLocal3Inst : public Instruction
{
public:
    StoreLocal3Inst();
    Instruction* Clone() const override { return new StoreLocal3Inst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API StoreLocalBInst : public ByteParamInst
{
public:
    StoreLocalBInst();
    Instruction* Clone() const override { return new StoreLocalBInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API StoreLocalSInst : public UShortParamInst
{
public:
    StoreLocalSInst();
    Instruction* Clone() const override { return new StoreLocalSInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API LoadFieldInst : public IndexParamInst
{
public:
    LoadFieldInst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new LoadFieldInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API LoadField0Inst : public Instruction
{
public:
    LoadField0Inst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new LoadField0Inst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API LoadField1Inst : public Instruction
{
public:
    LoadField1Inst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new LoadField1Inst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API LoadField2Inst : public Instruction
{
public:
    LoadField2Inst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new LoadField2Inst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API LoadField3Inst : public Instruction
{
public:
    LoadField3Inst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new LoadField3Inst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API LoadFieldBInst : public ByteParamInst
{
public:
    LoadFieldBInst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new LoadFieldBInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API LoadFieldSInst : public UShortParamInst
{
public:
    LoadFieldSInst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new LoadFieldSInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API StoreFieldInst : public IndexParamInst
{
public:
    StoreFieldInst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new StoreFieldInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API StoreField0Inst : public Instruction
{
public:
    StoreField0Inst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new StoreField0Inst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API StoreField1Inst : public Instruction
{
public:
    StoreField1Inst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new StoreField1Inst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API StoreField2Inst : public Instruction
{
public:
    StoreField2Inst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new StoreField2Inst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API StoreField3Inst : public Instruction
{
public:
    StoreField3Inst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new StoreField3Inst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API StoreFieldBInst : public ByteParamInst
{
public:
    StoreFieldBInst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new StoreFieldBInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API StoreFieldSInst : public UShortParamInst
{
public:
    StoreFieldSInst();
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    Instruction* Clone() const override { return new StoreFieldSInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType fieldType;
};

class MACHINE_API LoadElemInst : public Instruction
{
public:
    LoadElemInst();
    void SetElemType(ValueType elemType_) { elemType = elemType_; }
    ValueType GetElemType() const { return elemType; }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    Instruction* Clone() const override { return new LoadElemInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType elemType;
};

class MACHINE_API StoreElemInst : public Instruction
{
public:
    StoreElemInst();
    void SetElemType(ValueType elemType_) { elemType = elemType_; }
    ValueType GetElemType() const { return elemType; }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    Instruction* Clone() const override { return new StoreElemInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType elemType;
};

class MACHINE_API LoadConstantInst : public IndexParamInst
{
public:
    LoadConstantInst();
    Instruction* Clone() const override { return new LoadConstantInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API LoadConstantBInst : public ByteParamInst
{
public:
    LoadConstantBInst();
    Instruction* Clone() const override { return new LoadConstantBInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API LoadConstantSInst : public UShortParamInst
{
public:
    LoadConstantSInst();
    Instruction* Clone() const override { return new LoadConstantSInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API ReceiveInst : public Instruction
{
public:
    ReceiveInst();
    Instruction* Clone() const override { return new ReceiveInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API ConversionBaseInst : public Instruction
{
public:
    ConversionBaseInst(const std::string& name_);
    void Accept(MachineFunctionVisitor& visitor) override;
    virtual ValueType GetTargetType() const = 0;
};

template<typename SourceT, typename TargetT, ValueType type>
class MACHINE_API ConversionInst : public ConversionBaseInst
{
public:
    ConversionInst(const std::string& name_) : ConversionBaseInst(name_)
    {
    }
    Instruction* Clone() const override { return new ConversionInst<SourceT, TargetT, type>(*this); }
    ValueType GetTargetType() const override { return type; }
    void Execute(Frame& frame) override
    {
        SourceT source = static_cast<SourceT>(frame.OpStack().Pop().Value());
        TargetT target = static_cast<TargetT>(source);
        frame.OpStack().Push(IntegralValue(static_cast<uint64_t>(target), type));
    }
};

const int32_t endOfFunction = -1;

class MACHINE_API JumpInst : public IndexParamInst
{
public:
    JumpInst();
    Instruction* Clone() const override { return new JumpInst(*this); };
    void SetTarget(int32_t target) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    bool IsJumpingInst() const override { return true; }
    bool IsJump() const override { return true; } 
    bool EndsBasicBlock() const override { return true; }
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API JumpTrueInst : public IndexParamInst
{
public:
    JumpTrueInst();
    Instruction* Clone() const override { return new JumpTrueInst(*this); };
    void SetTarget(int32_t target) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    bool IsJumpingInst() const override { return true; }
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API JumpFalseInst : public IndexParamInst
{
public:
    JumpFalseInst();
    Instruction* Clone() const override { return new JumpFalseInst(*this); };
    void SetTarget(int32_t target) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    bool IsJumpingInst() const override { return true; }
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API EnterBlockInst : public Instruction
{
public:
    EnterBlockInst();
    Instruction* Clone() const override { return new EnterBlockInst(*this); };
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API ExitBlockInst : public Instruction
{
public:
    ExitBlockInst();
    Instruction* Clone() const override { return new ExitBlockInst(*this); };
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API ContinuousSwitchInst : public Instruction
{
public:
    ContinuousSwitchInst();
    ContinuousSwitchInst(const ContinuousSwitchInst& that) = default;
    void SetCondType(ValueType condType_) { condType = condType_; }
    ValueType CondType() const { return condType; }
    void Clear() override;
    Instruction* Clone() const override { return new ContinuousSwitchInst(*this); };
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void SetBegin(IntegralValue begin_) { begin = begin_; }
    void SetEnd(IntegralValue end_) { end = end_; }
    void AddTarget(int32_t target);
    void SetTarget(int index, int32_t target);
    void SetDefaultTarget(int32_t defaultTarget_) { defaultTarget = defaultTarget_; }
    void AddNextTargetIndex(int nextTargetIndex);
    void SetTarget(int32_t target) override;
    IntegralValue Begin() const { return begin; }
    IntegralValue End() const { return end; }
    const std::vector<int32_t>& Targets() const { return targets; }
    std::vector<int32_t>& Targets() { return targets; }
    int32_t DefaultTarget() const { return defaultTarget; }
    bool EndsBasicBlock() const override { return true; }
    bool IsContinuousSwitchInst() const override { return true; }
    void Dump(CodeFormatter& formatter) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType condType;
    IntegralValue begin;
    IntegralValue end;
    std::vector<int32_t> targets;
    int32_t defaultTarget;
    std::vector<int> nextTargetIndices;
};

struct MACHINE_API IntegralValueLess
{
    bool operator()(const std::pair<IntegralValue, int32_t>& left, const std::pair<IntegralValue, int32_t>& right) const
    {
        return left.first.Value() < right.first.Value();
    }
};

class MACHINE_API BinarySearchSwitchInst : public Instruction
{
public:
    BinarySearchSwitchInst();
    BinarySearchSwitchInst(const BinarySearchSwitchInst& that) = default;
    void SetCondType(ValueType condType_) { condType = condType_; }
    ValueType CondType() const { return condType; }
    void Clear() override;
    Instruction* Clone() const override { return new BinarySearchSwitchInst(*this); };
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void SetTargets(const std::vector<std::pair<IntegralValue, int32_t>>& targets_);
    void SetTarget(int32_t target) override;
    void SetDefaultTarget(int32_t defaultTarget_) { defaultTarget = defaultTarget_; }
    const std::vector<std::pair<IntegralValue, int32_t>>& Targets() const { return targets; }
    std::vector<std::pair<IntegralValue, int32_t>>& Targets() { return targets; }
    int32_t DefaultTarget() const { return defaultTarget; }
    bool EndsBasicBlock() const override { return true; }
    bool IsBinarySearchSwitchInst() const override { return true; }
    void Dump(CodeFormatter& formatter) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType condType;
    std::vector<std::pair<IntegralValue, int32_t>> targets;
    int32_t defaultTarget;
};

class MACHINE_API CallInst : public Instruction
{
public:
    CallInst();
    CallInst(const CallInst& that) = default;
    void Clear() override;
    Instruction* Clone() const override { return new CallInst(*this); }
    void SetFunctionCallName(Constant functionCallName);
    StringPtr GetFunctionCallName() const;
    void SetFunction(Function* fun);
    Function* GetFunction() const;
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    void DispatchTo(InstAdder& adder) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    Constant function;
};

class MACHINE_API VirtualCallInst : public Instruction
{
public:
    VirtualCallInst();
    VirtualCallInst(const VirtualCallInst& that) = default;
    void Clear() override;
    void SetNumArgs(uint32_t numArgs_) { numArgs = numArgs_; }
    void SetVmtIndex(uint32_t vmtIndex_) { vmtIndex = vmtIndex_; }
    uint32_t VmtIndex() const { return vmtIndex; }
    void SetFunctionType(const FunctionType& functionType_);
    const FunctionType& GetFunctionType() const { return functionType; }
    Instruction* Clone() const override { return new VirtualCallInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    uint32_t numArgs;
    uint32_t vmtIndex;
    FunctionType functionType;
};

class MACHINE_API InterfaceCallInst : public Instruction
{
public:
    InterfaceCallInst();
    InterfaceCallInst(const InterfaceCallInst& that) = default;
    void Clear() override;
    Instruction* Clone() const override { return new InterfaceCallInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void SetNumArgs(uint32_t numArgs_) { numArgs = numArgs_; };
    void SetImtIndex(uint32_t imtIndex_) { imtIndex = imtIndex_; }
    uint32_t ImtIndex() const { return imtIndex; }
    void SetFunctionType(const FunctionType& functionType_);
    const FunctionType& GetFunctionType() const { return functionType; }
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    uint32_t numArgs;
    uint32_t imtIndex;
    FunctionType functionType;
};

class MACHINE_API VmCallInst : public IndexParamInst
{
public:
    VmCallInst();
    Instruction* Clone() const override { return new VmCallInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API DelegateCallInst : public Instruction
{
public:
    DelegateCallInst();
    void SetFunctionType(const FunctionType& functionType_);
    const FunctionType& GetFunctionType() const { return functionType; }
    Instruction* Clone() const override { return new DelegateCallInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    FunctionType functionType;
};

class MACHINE_API ClassDelegateCallInst : public Instruction
{
public:
    ClassDelegateCallInst();
    void SetFunctionType(const FunctionType& functionType_);
    const FunctionType& GetFunctionType() const { return functionType; }
    Instruction* Clone() const override { return new ClassDelegateCallInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    FunctionType functionType;
};

class MACHINE_API SetClassDataInst : public Instruction
{
public:
    SetClassDataInst();
    SetClassDataInst(const SetClassDataInst& that) = default;
    void Clear() override;
    Instruction* Clone() const override { return new SetClassDataInst(*this); }
    void SetClassName(Constant fullClassName);
    StringPtr GetClassName() const;
    void SetClassData(ClassData* classDataPtr);
    ClassData* GetClassData() const { return classData.Value().AsClassDataPtr(); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    void DispatchTo(InstAdder& adder) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    Constant classData;
};

class MACHINE_API TypeInstruction : public Instruction
{
public:
    TypeInstruction(const std::string& name_);
    TypeInstruction(const TypeInstruction& that) = default;
    void Clear() override;
    void SetTypeName(Constant fullTypeName);
    StringPtr GetTypeName() const;
    void SetType(Type* typePtr);
    Type* GetType();
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Dump(CodeFormatter& formatter) override;
    void DispatchTo(InstAdder& adder) override;
private:
    Constant type;
};

class MACHINE_API CreateObjectInst : public TypeInstruction
{
public:
    CreateObjectInst();
    Instruction* Clone() const override { return new CreateObjectInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API CopyObjectInst : public Instruction
{
public:
    CopyObjectInst();
    Instruction* Clone() const override { return new CopyObjectInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API StrLitToStringInst : public Instruction
{
public:
    StrLitToStringInst();
    Instruction* Clone() const override { return new StrLitToStringInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API LoadStringCharInst : public Instruction
{
public:
    LoadStringCharInst();
    Instruction* Clone() const override { return new LoadStringCharInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API DupInst : public Instruction
{
public:
    DupInst();
    Instruction* Clone() const override { return new DupInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API SwapInst : public Instruction
{
public:
    SwapInst();
    Instruction* Clone() const override { return new SwapInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API RotateInst : public Instruction
{
public:
    RotateInst();
    Instruction* Clone() const override { return new RotateInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API PopInst : public Instruction
{
public:
    PopInst();
    Instruction* Clone() const override { return new PopInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API DownCastInst : public TypeInstruction
{
public:
    DownCastInst();
    Instruction* Clone() const override { return new DownCastInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API ThrowInst : public Instruction
{
public:
    ThrowInst();
    Instruction* Clone() const override { return new ThrowInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
    bool EndsBasicBlock() const override { return true; }
    bool IsThrow() const override { return true; }
};

class MACHINE_API RethrowInst : public Instruction
{
public:
    RethrowInst();
    Instruction* Clone() const override { return new RethrowInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
    bool EndsBasicBlock() const override { return true; }
};

class MACHINE_API BeginTryInst : public IndexParamInst
{
public:
    BeginTryInst();
    Instruction* Clone() const override { return new BeginTryInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
    bool DontRemove() const override { return true; }
};

class MACHINE_API EndTryInst : public IndexParamInst
{
public:
    EndTryInst();
    Instruction* Clone() const override { return new EndTryInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
    bool DontRemove() const override { return true; }
    bool IsEndEhInst() const override { return true; }
};

class MACHINE_API BeginCatchSectionInst : public IndexParamInst
{
public:
    BeginCatchSectionInst();
    Instruction* Clone() const override { return new BeginCatchSectionInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
    bool DontRemove() const override { return true; }
};

class MACHINE_API EndCatchSectionInst : public IndexParamInst
{
public:
    EndCatchSectionInst();
    Instruction* Clone() const override { return new EndCatchSectionInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
    bool DontRemove() const override { return true; }
    bool IsEndEhInst() const override { return true; }
};

class MACHINE_API BeginCatchInst : public IndexParamInst
{
public:
    BeginCatchInst();
    Instruction* Clone() const override { return new BeginCatchInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
    bool DontRemove() const override { return true; }
};

class MACHINE_API EndCatchInst : public IndexParamInst
{
public:
    EndCatchInst();
    Instruction* Clone() const override { return new EndCatchInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
    bool DontRemove() const override { return true; }
    bool IsEndEhInst() const override { return true; }
};

class MACHINE_API BeginFinallyInst : public IndexParamInst
{
public:
    BeginFinallyInst();
    Instruction* Clone() const override { return new BeginFinallyInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
    bool DontRemove() const override { return true; }
};

class MACHINE_API EndFinallyInst : public IndexParamInst
{
public:
    EndFinallyInst();
    Instruction* Clone() const override { return new EndFinallyInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
    bool DontRemove() const override { return true; }
    bool IsEndEhInst() const override { return true; }
};

class ExceptionBlock;

class MACHINE_API NextInst : public Instruction
{
public:
    NextInst();
    void Clear() override;
    Instruction* Clone() const override { return new NextInst(*this); }
    void SetTarget(int32_t target) override;
    void SetExceptionBlock(ExceptionBlock* exceptionBlock_) { exceptionBlock = exceptionBlock_; }
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ExceptionBlock* exceptionBlock;
};

class MACHINE_API StaticInitInst : public TypeInstruction
{
public:
    StaticInitInst();
    Instruction* Clone() const override { return new StaticInitInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API DoneStaticInitInst : public TypeInstruction
{
public:
    DoneStaticInitInst();
    Instruction* Clone() const override { return new DoneStaticInitInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API LoadStaticFieldInst : public TypeInstruction
{
public:
    LoadStaticFieldInst();
    LoadStaticFieldInst(const LoadStaticFieldInst& that) = default;
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    ValueType GetFieldType() const { return fieldType; }
    void Clear() override;
    Instruction* Clone() const override { return new LoadStaticFieldInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    void SetIndex(int32_t index_) override;
    int32_t Index() const { return index; }
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    int32_t index;
    ValueType fieldType;
};

class MACHINE_API StoreStaticFieldInst : public TypeInstruction
{
public:
    StoreStaticFieldInst();
    StoreStaticFieldInst(const StoreStaticFieldInst& that) = default;
    void SetFieldType(ValueType fieldType_) { fieldType = fieldType_; }
    ValueType GetFieldType() const { return fieldType; }
    void Clear() override;
    Instruction* Clone() const override { return new StoreStaticFieldInst(*this); }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    void SetIndex(int32_t index_) override;
    int32_t Index() const { return index; }
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    int32_t index;
    ValueType fieldType;
};

class MACHINE_API EqualObjectNullInst : public Instruction
{
public:
    EqualObjectNullInst();
    Instruction* Clone() const override { return new EqualObjectNullInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API EqualNullObjectInst : public Instruction
{
public:
    EqualNullObjectInst();
    Instruction* Clone() const override { return new EqualNullObjectInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API BoxBaseInst : public Instruction
{
public:
    BoxBaseInst(const std::string& name_);
    virtual ValueType GetValueType() const = 0;
    void Accept(MachineFunctionVisitor& visitor) override;
};

template<ValueType valueType>
class MACHINE_API BoxInst : public BoxBaseInst
{
public:
    BoxInst(const std::string& name_) : BoxBaseInst(name_) {}
    Instruction* Clone() const override { return new BoxInst<valueType>(*this); }
    ValueType GetValueType() const override { return valueType; }
    void Execute(Frame& frame)
    {
        try
        {
            ObjectType* objectType = GetBoxedType(valueType);
            ObjectReference objectReference = GetManagedMemoryPool().CreateObject(frame.GetThread(), objectType);
            ClassData* classData = GetClassDataForBoxedType(valueType);
            IntegralValue classDataValue(classData);
            GetManagedMemoryPool().SetField(objectReference, 0, classDataValue);
            IntegralValue value = frame.OpStack().Pop();
            Assert(value.GetType() == valueType, "value type mismatch");
            GetManagedMemoryPool().SetField(objectReference, 1, value);
            frame.OpStack().Push(objectReference);
        }
        catch (const NullReferenceException& ex)
        {
            ThrowNullReferenceException(ex, frame);
        }
        catch (const SystemException& ex)
        {
            ThrowSystemException(ex, frame);
        }
    }
};

class MACHINE_API UnboxBaseInst : public Instruction
{
public:
    UnboxBaseInst(const std::string& name_);
    virtual ValueType GetValueType() const = 0;
    void Accept(MachineFunctionVisitor& visitor) override;
};

template<ValueType valueType>
class MACHINE_API UnboxInst : public UnboxBaseInst
{
public:
    UnboxInst(const std::string& name_) : UnboxBaseInst(name_) {}
    Instruction* Clone() const override { return new UnboxInst<valueType>(*this); }
    ValueType GetValueType() const override { return valueType; }
    void Execute(Frame& frame)
    {
        try
        {
            IntegralValue value = frame.OpStack().Pop();
            Assert(value.GetType() == ValueType::objectReference, "object reference operand expected");
            ObjectReference objectReference(value.Value());
            if (objectReference.IsNull())
            {
                throw NullReferenceException("tried to unbox null object reference");
            }
            else
            {
                IntegralValue value = GetManagedMemoryPool().GetField(objectReference, 1);
                if (value.GetType() != valueType)
                {
                    throw SystemException("invalid unbox operation: value type does not match");
                }
                frame.OpStack().Push(value);
            }
        }
        catch (const NullReferenceException& ex)
        {
            ThrowNullReferenceException(ex, frame);
        }
        catch (const SystemException& ex)
        {
            ThrowSystemException(ex, frame);
        }
    }
};

class MACHINE_API AllocateArrayElementsInst : public TypeInstruction
{
public:
    AllocateArrayElementsInst();
    Instruction* Clone() const override { return new AllocateArrayElementsInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API IsInst : public TypeInstruction
{
public:
    IsInst();
    Instruction* Clone() const override { return new IsInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API AsInst : public TypeInstruction
{
public:
    AsInst();
    Instruction* Clone() const override { return new AsInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

class MACHINE_API Fun2DlgInst : public Instruction
{
public:
    Fun2DlgInst();
    Fun2DlgInst(const Fun2DlgInst& that) = default;
    void Clear() override;
    Instruction* Clone() const override { return new Fun2DlgInst(*this); }
    void SetFunctionName(Constant functionName);
    StringPtr GetFunctionName() const;
    void SetFunction(Function* fun);
    Function* GetFunction();
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    void DispatchTo(InstAdder& adder) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    Constant function;
};

class MACHINE_API MemFun2ClassDlgInst : public Instruction
{
public:
    MemFun2ClassDlgInst();
    MemFun2ClassDlgInst(const MemFun2ClassDlgInst& that) = default;
    void Clear() override;
    Instruction* Clone() const override { return new MemFun2ClassDlgInst(*this); }
    void SetFunctionName(Constant functionName);
    StringPtr GetFunctionName() const;
    void SetFunction(Function* fun);
    Function* GetFunction() const;
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    void DispatchTo(InstAdder& adder) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    Constant function;
};

class MACHINE_API CreateLocalVariableReferenceInst : public Instruction
{
public:
    CreateLocalVariableReferenceInst();
    CreateLocalVariableReferenceInst(const CreateLocalVariableReferenceInst& that) = default;
    void Clear() override;
    Instruction* Clone() const override { return new CreateLocalVariableReferenceInst(*this); }
    void SetLocalIndex(int32_t localIndex_) { localIndex = localIndex_; }
    int32_t LocalIndex() const { return localIndex; }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    int32_t localIndex;
};

class MACHINE_API CreateMemberVariableReferenceInst : public Instruction
{
public:
    CreateMemberVariableReferenceInst();
    CreateMemberVariableReferenceInst(const CreateMemberVariableReferenceInst& that) = default;
    void Clear() override;
    Instruction* Clone() const override { return new CreateMemberVariableReferenceInst(*this); }
    void SetMemberVarIndex(int32_t memberVarIndex_) { memberVarIndex = memberVarIndex_; }
    int32_t MemberVarIndex() const { return memberVarIndex; }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Dump(CodeFormatter& formatter) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    int32_t memberVarIndex;
};

class MACHINE_API LoadVariableReferenceInst : public IndexParamInst, public VariableReferenceHandler
{
public:
    LoadVariableReferenceInst();
    Instruction* Clone() const override { return new LoadVariableReferenceInst(*this); }
    void SetType(ValueType type_) { type = type_; }
    ValueType GetType() const { return type; }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Handle(Frame& frame, LocalVariableReference* localVariableReference) override;
    void Handle(Frame& frame, MemberVariableReference* memberVariableReference) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType type;
};

class MACHINE_API StoreVariableReferenceInst : public IndexParamInst, public VariableReferenceHandler
{
public:
    StoreVariableReferenceInst();
    Instruction* Clone() const override { return new StoreVariableReferenceInst(*this); }
    void SetType(ValueType type_) { type = type_; }
    ValueType GetType() const { return type; }
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    void Execute(Frame& frame) override;
    void Handle(Frame& frame, LocalVariableReference* localVariableReference) override;
    void Handle(Frame& frame, MemberVariableReference* memberVariableRefeence) override;
    void Accept(MachineFunctionVisitor& visitor) override;
private:
    ValueType type;
};

class MACHINE_API GcPointInst : public Instruction
{
public:
    GcPointInst();
    Instruction* Clone() const override { return new GcPointInst(*this); }
    void Execute(Frame& frame) override;
    void Accept(MachineFunctionVisitor& visitor) override;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_INSTRUCTION_INCLUDED
