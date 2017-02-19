// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_VARIABLE_REFERENCE_INCLUDED
#define CMINOR_MACHINE_VARIABLE_REFERENCE_INCLUDED
#include <cminor/machine/Object.hpp>

namespace cminor { namespace machine {

class LocalVariableReference;
class MemberVariableReference;
class Frame;

class MACHINE_API VariableReferenceHandler
{
public:
    virtual ~VariableReferenceHandler();
    virtual void Handle(Frame& frame, LocalVariableReference* localVariableReference) = 0;
    virtual void Handle(Frame& frame, MemberVariableReference* memberVariableReference) = 0;
};

class MACHINE_API VariableReference
{
public:
    VariableReference(int32_t id_);
    virtual ~VariableReference();
    int32_t Id() const { return id; }
    virtual void DispatchTo(VariableReferenceHandler* handler, Frame& frame) = 0;
private:
    int32_t id;
};

class MACHINE_API LocalVariableReference : public VariableReference
{
public:
    LocalVariableReference(int32_t id_, int32_t frameId_, int32_t localIndex_);
    int32_t FrameId() const { return frameId; }
    int32_t LocalIndex() const { return localIndex; }
    void DispatchTo(VariableReferenceHandler* handler, Frame& frame) override;
private:
    int32_t frameId;
    int32_t localIndex;
};

class MACHINE_API MemberVariableReference : public VariableReference
{
public:
    MemberVariableReference(int32_t id_, ObjectReference objectReference_, int32_t memberVarIndex_);
    ObjectReference GetObjectReference() const { return objectReference;  }
    int32_t MemberVarIndex() const { return memberVarIndex; }
    void DispatchTo(VariableReferenceHandler* handler, Frame& frame) override;
private:
    ObjectReference objectReference;
    int32_t memberVarIndex;
};

} } // namespace cminor::machine

#endif // CMINOR_MACHINE_VARIABLE_REFERENCE_INCLUDED
