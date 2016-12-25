// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/VariableReference.hpp>

namespace cminor { namespace machine {

VariableReferenceHandler::~VariableReferenceHandler()
{
}

VariableReference::VariableReference(int32_t id_) : id(id_)
{
}

VariableReference::~VariableReference()
{
}

LocalVariableReference::LocalVariableReference(int32_t id_, int32_t frameId_, int32_t localIndex_) : VariableReference(id_), frameId(frameId_), localIndex(localIndex_)
{
}

void LocalVariableReference::DispatchTo(VariableReferenceHandler* handler, Frame& frame)
{
    handler->Handle(frame, this);
}

MemberVariableReference::MemberVariableReference(int32_t id_, ObjectReference objectReference_, int32_t memberVarIndex_) :
    VariableReference(id_), objectReference(objectReference_), memberVarIndex(memberVarIndex_)
{
}

void MemberVariableReference::DispatchTo(VariableReferenceHandler* handler, Frame& frame)
{
    handler->Handle(frame, this);
}

} } // namespace cminor::machine