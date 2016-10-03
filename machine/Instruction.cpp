// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/Instruction.hpp>
#include <cminor/machine/Error.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Util.hpp>
#include <cminor/machine/String.hpp>
#include <cminor/machine/Constant.hpp>

namespace cminor { namespace machine {

Instruction::Instruction(const std::string& name_) : opCode(255), name(name_), groupName(""), typeName(""), parent(nullptr)
{
}

Instruction::Instruction(const std::string& name_, const std::string& groupName_, const std::string& typeName_) : opCode(255), name(name_), groupName(groupName_), typeName(typeName_), parent(nullptr)
{
}

Instruction::~Instruction()
{
}

void Instruction::Encode(Writer& writer)
{
    if (parent)
    {
        parent->Encode(writer);
    }
    writer.Put(opCode);
}

Instruction* Instruction::Decode(Reader& reader)
{
    return this;
}

void Instruction::Execute(Frame& frame)
{
}

void Instruction::Dump(CodeFormatter& formatter)
{
    formatter.Write(ToHexString(opCode) + " " + name);
}

InvalidInst::InvalidInst() : Instruction("<invalid_instruction>", "", "")
{
}

void InvalidInst::Encode(Writer& writer)
{
    throw std::runtime_error("tried to encode invalid instruction " + std::to_string(OpCode()));
}

Instruction* InvalidInst::Decode(Reader& reader)
{
    throw std::runtime_error("tried to decode invalid instruction " + std::to_string(OpCode()));
}

void InvalidInst::Execute(Frame& frame)
{
    throw std::runtime_error("invalid instruction " + std::to_string(OpCode()));
}

NopInst::NopInst() : Instruction("nop", "", "")
{
}

ContainerInst::ContainerInst(Machine& machine_, const std::string& name_, bool root_) : Instruction(name_, "", ""), machine(machine_), root(root_)
{
    childInsts.resize(256);
    for (int i = 0; i < 256; ++i)
    {
        SetInst(i, new InvalidInst());
    }
}

void ContainerInst::SetInst(uint8_t opCode, Instruction* inst)
{
    inst->SetOpCode(opCode);
    childInsts[opCode].reset(inst);
    if (dynamic_cast<InvalidInst*>(inst) == nullptr)
    {
        inst->SetParent(this);
        machine.AddInst(inst);
    }
}

void ContainerInst::Encode(Writer& writer)
{
    if (!root)
    {
        Instruction::Encode(writer);
    }
}

Instruction* ContainerInst::Decode(Reader& reader)
{
    uint8_t opCode = reader.GetByte();
    Instruction* inst = childInsts[opCode].get();
    return inst->Decode(reader);
}

InstructionTypeGroup::InstructionTypeGroup() : instGroupName("")
{
}

InstructionTypeGroup::InstructionTypeGroup(const std::string& instGroupName_) : instGroupName(instGroupName_)
{
}

void InstructionTypeGroup::AddInst(Instruction* inst)
{
    instructionMap[inst->TypeName()] = inst;
}

std::unique_ptr<Instruction> InstructionTypeGroup::CreateInst(const std::string& typeName) const
{
    auto it = instructionMap.find(typeName);
    if (it != instructionMap.cend())
    {
        Instruction* inst = it->second;
        return std::unique_ptr<Instruction>(inst->Clone());
    }
    throw std::runtime_error("instruction for type '" + typeName + "' not found in instruction group '" + instGroupName + "'");
}

LogicalNotInst::LogicalNotInst() : Instruction("not", "", "")
{
}

void LogicalNotInst::Execute(Frame& frame)
{
    IntegralValue value = frame.OpStack().Pop();
    Assert(value.GetType() == ValueType::boolType, "bool operand expected");
    bool operand = value.AsBool();
    frame.OpStack().Push(IntegralValue(!operand, ValueType::boolType));
}

IndexParamInst::IndexParamInst(const std::string& name_) : Instruction(name_, "", ""), index(-1)
{
}

void IndexParamInst::Encode(Writer& writer)
{
    Instruction::Encode(writer);
    writer.Put(index);
}

Instruction* IndexParamInst::Decode(Reader& reader)
{
    Instruction::Decode(reader);
    index = reader.GetInt();
    return this;
}

void IndexParamInst::Dump(CodeFormatter& formatter)
{
    Instruction::Dump(formatter);
    formatter.Write(" " + std::to_string(index));
}

LoadLocalInst::LoadLocalInst() : IndexParamInst("loadlocal")
{
}

void LoadLocalInst::Execute(Frame& frame)
{
    frame.OpStack().Push(frame.Local(Index()).GetValue());
}

StoreLocalInst::StoreLocalInst() : IndexParamInst("storelocal")
{
}

void StoreLocalInst::Execute(Frame& frame)
{
    frame.Local(Index()).SetValue(frame.OpStack().Pop());
}

LoadFieldInst::LoadFieldInst() : IndexParamInst("loadfield")
{
}

void LoadFieldInst::Execute(Frame& frame)
{
    IntegralValue operand = frame.OpStack().Pop();
    Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference reference = ObjectReference(operand.Value());
    IntegralValue fieldValue = frame.GetObjectPool().GetField(reference, Index());
    frame.OpStack().Push(fieldValue);
}

StoreFieldInst::StoreFieldInst() : IndexParamInst("storefield")
{
}

void StoreFieldInst::Execute(Frame& frame)
{
    IntegralValue fieldValue = frame.OpStack().Pop();
    IntegralValue operand = frame.OpStack().Pop();
    Assert(operand.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference reference = ObjectReference(operand.Value());
    frame.GetObjectPool().SetField(reference, Index(), fieldValue);
}

LoadNullReferenceInst::LoadNullReferenceInst() : Instruction("loadnull")
{
}

void LoadNullReferenceInst::Execute(Frame& frame)
{
    frame.OpStack().Push(ObjectReference());
}

LoadConstantInst::LoadConstantInst() : IndexParamInst("loadconstant")
{
}

void LoadConstantInst::Execute(Frame& frame)
{
    ConstantId constantId(Index());
    frame.OpStack().Push(frame.GetConstantPool().GetConstant(constantId).Value());
}

InitStringFromConstInst::InitStringFromConstInst() : IndexParamInst("initstc")
{
}

void InitStringFromConstInst::Execute(Frame& frame)
{
    ConstantId constantId(Index());
    Constant constant = frame.GetConstantPool().GetConstant(constantId);
    IntegralValue constantValue = constant.Value();
    Assert(constantValue.GetType() == ValueType::stringLiteral, "string constant expected");
    ObjectReference reference = frame.GetObjectPool().CreateString(frame.GetThread(), ArenaId::notGCMem, constantValue);
    frame.OpStack().Push(reference);
}

StringEqualInst::StringEqualInst() : Instruction("equalst", "equal", "string")
{
}

void StringEqualInst::Execute(Frame& frame) 
{
    IntegralValue rightOperand = frame.OpStack().Pop();
    Assert(rightOperand.GetType() == ValueType::objectReference, "object reference operand expected");
    IntegralValue leftOperand = frame.OpStack().Pop();
    Assert(leftOperand.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference left = ObjectReference(leftOperand.Value());
    ObjectReference right = ObjectReference(rightOperand.Value());
    const char32_t* leftChars = frame.GetConstantPool().GetEmptyStringConstant().Value().AsStringLiteral();
    const char32_t* rightChars = leftChars;
    if (!left.IsNull())
    {
        ObjectMemPtr leftMemPtr = frame.GetObjectPool().GetObjectMemPtr(left);
        leftChars = reinterpret_cast<const char32_t*>(leftMemPtr.Value());
    }
    if (!right.IsNull())
    {
        ObjectMemPtr rightMemPtr = frame.GetObjectPool().GetObjectMemPtr(right);
        rightChars = reinterpret_cast<const char32_t*>(rightMemPtr.Value());
    }
    bool result = StringPtr(leftChars) == StringPtr(rightChars);
    frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
}

StringLessInst::StringLessInst() : Instruction("lessst", "less", "string")
{
}

void StringLessInst::Execute(Frame& frame)
{
    IntegralValue rightOperand = frame.OpStack().Pop();
    Assert(rightOperand.GetType() == ValueType::objectReference, "object reference operand expected");
    IntegralValue leftOperand = frame.OpStack().Pop();
    Assert(leftOperand.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference left = ObjectReference(leftOperand.Value());
    ObjectReference right = ObjectReference(rightOperand.Value());
    const char32_t* leftChars = frame.GetConstantPool().GetEmptyStringConstant().Value().AsStringLiteral();
    const char32_t* rightChars = leftChars;
    if (!left.IsNull())
    {
        ObjectMemPtr leftMemPtr = frame.GetObjectPool().GetObjectMemPtr(left);
        leftChars = reinterpret_cast<const char32_t*>(leftMemPtr.Value());
    }
    if (!right.IsNull())
    {
        ObjectMemPtr rightMemPtr = frame.GetObjectPool().GetObjectMemPtr(right);
        rightChars = reinterpret_cast<const char32_t*>(rightMemPtr.Value());
    }
    bool result = StringPtr(leftChars) < StringPtr(rightChars);
    frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
}

StringEqStrLitInst::StringEqStrLitInst() : Instruction("equalstringstrlit")
{
}

void StringEqStrLitInst::Execute(Frame& frame)
{
    IntegralValue rightOperand = frame.OpStack().Pop();
    Assert(rightOperand.GetType() == ValueType::stringLiteral, "string literal operand expected");
    IntegralValue leftOperand = frame.OpStack().Pop();
    Assert(leftOperand.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference left = ObjectReference(leftOperand.Value());
    const char32_t* leftChars = frame.GetConstantPool().GetEmptyStringConstant().Value().AsStringLiteral();
    if (!left.IsNull())
    {
        ObjectMemPtr leftMemPtr = frame.GetObjectPool().GetObjectMemPtr(left);
        leftChars = reinterpret_cast<const char32_t*>(leftMemPtr.Value());
    }
    const char32_t* rightChars = rightOperand.AsStringLiteral();
    bool result = StringPtr(leftChars) == StringPtr(rightChars);
    frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
}

StrLitEqStringInst::StrLitEqStringInst() : Instruction("equalstrlitstring")
{
}

void StrLitEqStringInst::Execute(Frame& frame)
{
    IntegralValue rightOperand = frame.OpStack().Pop();
    Assert(rightOperand.GetType() == ValueType::objectReference, "object reference operand expected");
    IntegralValue leftOperand = frame.OpStack().Pop();
    Assert(leftOperand.GetType() == ValueType::stringLiteral, "string literal operand expected");
    const char32_t* leftChars = leftOperand.AsStringLiteral();
    ObjectReference right = ObjectReference(rightOperand.Value());
    const char32_t* rightChars = frame.GetConstantPool().GetEmptyStringConstant().Value().AsStringLiteral();
    if (!right.IsNull())
    {
        ObjectMemPtr rightMemPtr = frame.GetObjectPool().GetObjectMemPtr(right);
        rightChars = reinterpret_cast<const char32_t*>(rightMemPtr.Value());
    }
    bool result = StringPtr(leftChars) == StringPtr(rightChars);
    frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
}

StringLessStrLitInst::StringLessStrLitInst() : Instruction("lessstringstrlit")
{
}

void StringLessStrLitInst::Execute(Frame& frame)
{
    IntegralValue rightOperand = frame.OpStack().Pop();
    Assert(rightOperand.GetType() == ValueType::stringLiteral, "string literal operand expected");
    IntegralValue leftOperand = frame.OpStack().Pop();
    Assert(leftOperand.GetType() == ValueType::objectReference, "object reference operand expected");
    ObjectReference left = ObjectReference(leftOperand.Value());
    const char32_t* leftChars = frame.GetConstantPool().GetEmptyStringConstant().Value().AsStringLiteral();
    if (!left.IsNull())
    {
        ObjectMemPtr leftMemPtr = frame.GetObjectPool().GetObjectMemPtr(left);
        leftChars = reinterpret_cast<const char32_t*>(leftMemPtr.Value());
    }
    const char32_t* rightChars = rightOperand.AsStringLiteral();
    bool result = StringPtr(leftChars) < StringPtr(rightChars);
    frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
}

StrLitLessStringInst::StrLitLessStringInst() : Instruction("lessstrlitstring")
{
}

void StrLitLessStringInst::Execute(Frame& frame)
{
    IntegralValue rightOperand = frame.OpStack().Pop();
    Assert(rightOperand.GetType() == ValueType::objectReference, "object reference operand expected");
    IntegralValue leftOperand = frame.OpStack().Pop();
    Assert(leftOperand.GetType() == ValueType::stringLiteral, "string literal operand expected");
    const char32_t* leftChars = leftOperand.AsStringLiteral();
    ObjectReference right = ObjectReference(rightOperand.Value());
    const char32_t* rightChars = frame.GetConstantPool().GetEmptyStringConstant().Value().AsStringLiteral();
    if (!right.IsNull())
    {
        ObjectMemPtr rightMemPtr = frame.GetObjectPool().GetObjectMemPtr(right);
        rightChars = reinterpret_cast<const char32_t*>(rightMemPtr.Value());
    }
    bool result = StringPtr(leftChars) < StringPtr(rightChars);
    frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
}

} } // namespace cminor::machine
