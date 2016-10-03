// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/CompileUnit.hpp>
#include <cminor/ast/Visitor.hpp>

namespace cminor { namespace ast {

CompileUnitNode::CompileUnitNode(const Span& span_) : Node(span_)
{
}

CompileUnitNode::CompileUnitNode(const Span& span_, const std::string& filePath_) : Node(span_), filePath(filePath_)
{
}

Node* CompileUnitNode::Clone(CloneContext& cloneContext) const
{
    CompileUnitNode* clone = new CompileUnitNode(GetSpan(), filePath);
    clone->globalNs.reset(static_cast<NamespaceNode*>(globalNs->Clone(cloneContext)));
    return clone;
}

void CompileUnitNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
