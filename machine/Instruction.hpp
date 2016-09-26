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
#include <string>
#include <memory>

namespace cminor { namespace machine {

class Machine;

class Instruction
{
public:
    Instruction(const std::string& name_);
    Instruction(const Instruction&) = default;
    virtual ~Instruction();
    virtual Instruction* Clone() const = 0;
    virtual void Encode(Writer& writer);
    virtual Instruction* Decode(Reader& reader);
    void SetParent(Instruction* parent_) { parent = parent_; }
    void SetOpCode(uint8_t opCode_) { opCode = opCode_; }
    uint8_t OpCode() const { return opCode; }
    const std::string& Name() const { return name; }
    virtual void Execute(Frame& frame);
    virtual void Dump(CodeFormatter& formatter);
private:
    uint8_t opCode;
    std::string name;
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

class GroupInst : public Instruction
{
public:
    GroupInst(Machine& machine_, const std::string& name_, bool root_);
    void SetInst(uint8_t opCode, Instruction* inst);
    void Encode(Writer& writer) override;
    Instruction* Decode(Reader& reader) override;
    Instruction* Clone() const override { assert(false, "not cloneable"); return nullptr; }
    bool IsRoot() const { return root; }
private:
    bool root;
    Machine& machine;
    std::vector<std::unique_ptr<Instruction>> childInsts;
};

template<typename OperandT, typename UnaryOpT>
class UnaryOpInst : public Instruction
{
public:
    UnaryOpInst(const std::string& name_) : Instruction(name_)
    {
    }
    Instruction* Clone() const override { return new UnaryOpInst<OperandT, UnaryOpT>(*this); }
    void Execute(Frame& frame) override
    {
        OperandT operand = static_cast<OperandT>(frame.OpStack().Pop());
        frame.OpStack().Push(static_cast<uint64_t>(UnaryOpT()(operand)));
    }
};

template<typename OperandT, typename BinaryOpT>
class BinaryOpInst : public Instruction
{
public:
    BinaryOpInst(const std::string& name_) : Instruction(name_)
    {
    }
    Instruction* Clone() const override { return new BinaryOpInst<OperandT, BinaryOpT>(*this); }
    void Execute(Frame& frame) override
    {
        OperandT right = static_cast<OperandT>(frame.OpStack().Pop());
        OperandT left = static_cast<OperandT>(frame.OpStack().Pop());
        frame.OpStack().Push(static_cast<uint64_t>(BinaryOpT()(left, right)));
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

template<typename OperandT, typename RelationT>
class BinaryPredInst : public Instruction
{
public:
    BinaryPredInst(const std::string& name_) : Instruction(name_)
    {
    }
    Instruction* Clone() const override { return new BinaryPredInst<OperandT, RelationT>(*this); }
    void Execute(Frame& frame) override
    {
        OperandT right = static_cast<OperandT>(frame.OpStack().Pop());
        OperandT left = static_cast<OperandT>(frame.OpStack().Pop());
        bool result = RelationT()(left, right);
        frame.OpStack().Push(static_cast<uint64_t>(result));
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

template<typename SourceT, typename TargetT>
class ConversionInst : public Instruction
{
public:
    ConversionInst(const std::string& name_) : Instruction(name_)
    {
    }
    Instruction* Clone() const override { return new ConversionInst<SourceT, TargetT>(*this); }
    void Execute(Frame& frame) override
    {
        SourceT source = static_cast<SourceT>(frame.OpStack().Pop());
        TargetT target = static_cast<TargetT>(source);
        frame.OpStack().Push(static_cast<uint64_t>(target));
    }
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_INSTRUCTION_INCLUDED
