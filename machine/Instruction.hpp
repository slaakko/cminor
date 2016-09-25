// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_INSTRUCTION_INCLUDED
#define CMINOR_MACHINE_INSTRUCTION_INCLUDED
#include <cminor/machine/Frame.hpp>
#include <string>
#include <memory>

namespace cminor { namespace machine {

class Machine;

class Instruction
{
public:
    Instruction(const std::string& name_);
    virtual ~Instruction();
    void SetOpCode(uint8_t opCode_) { opCode = opCode_; }
    uint8_t OpCode() const { return opCode; }
    const std::string& Name() const { return name; }
    virtual void Execute(Frame& frame);
private:
    uint8_t opCode;
    std::string name;
};

class InvalidInst : public Instruction
{
public:
    InvalidInst();
    void Execute(Frame& frame);
};

class GroupInst : public Instruction
{
public:
    GroupInst(Machine& machine_, const std::string& name_);
    void SetInst(uint8_t opCode, Instruction* inst);
private:
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
    void Execute(Frame& frame) override;
};

template<typename SourceT, typename TargetT>
class ConversionInst : public Instruction
{
public:
    ConversionInst(const std::string& name_) : Instruction(name_)
    {
    }
    void Execute(Frame& frame) override
    {
        SourceT source = static_cast<SourceT>(frame.OpStack().Pop());
        TargetT target = static_cast<TargetT>(source);
        frame.OpStack().Push(static_cast<uint64_t>(target));
    }
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_INSTRUCTION_INCLUDED
