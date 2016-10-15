// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Expression.hpp>
#include <cminor/ast/Visitor.hpp>

namespace cminor { namespace ast {

DisjunctionNode::DisjunctionNode(const Span& span_) : BinaryNode(span_)
{
}

DisjunctionNode::DisjunctionNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* DisjunctionNode::Clone(CloneContext& cloneContext) const
{
    return new DisjunctionNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void DisjunctionNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ConjunctionNode::ConjunctionNode(const Span& span_) : BinaryNode(span_)
{
}

ConjunctionNode::ConjunctionNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* ConjunctionNode::Clone(CloneContext& cloneContext) const
{
    return new ConjunctionNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void ConjunctionNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

BitOrNode::BitOrNode(const Span& span_) : BinaryNode(span_)
{
}

BitOrNode::BitOrNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* BitOrNode::Clone(CloneContext& cloneContext) const
{
    return new BitOrNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void BitOrNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

BitXorNode::BitXorNode(const Span& span_) : BinaryNode(span_)
{
}

BitXorNode::BitXorNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* BitXorNode::Clone(CloneContext& cloneContext) const
{
    return new BitXorNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void BitXorNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

BitAndNode::BitAndNode(const Span& span_) : BinaryNode(span_)
{
}

BitAndNode::BitAndNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* BitAndNode::Clone(CloneContext& cloneContext) const
{
    return new BitAndNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void BitAndNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

EqualNode::EqualNode(const Span& span_) : BinaryNode(span_)
{
}

EqualNode::EqualNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* EqualNode::Clone(CloneContext& cloneContext) const
{
    return new EqualNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void EqualNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

NotEqualNode::NotEqualNode(const Span& span_) : BinaryNode(span_)
{
}

NotEqualNode::NotEqualNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* NotEqualNode::Clone(CloneContext& cloneContext) const
{
    return new NotEqualNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void NotEqualNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

LessNode::LessNode(const Span& span_) : BinaryNode(span_)
{
}

LessNode::LessNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* LessNode::Clone(CloneContext& cloneContext) const
{
    return new LessNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void LessNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

GreaterNode::GreaterNode(const Span& span_) : BinaryNode(span_)
{
}

GreaterNode::GreaterNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* GreaterNode::Clone(CloneContext& cloneContext) const
{
    return new GreaterNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void GreaterNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

LessOrEqualNode::LessOrEqualNode(const Span& span_) : BinaryNode(span_)
{
}

LessOrEqualNode::LessOrEqualNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* LessOrEqualNode::Clone(CloneContext& cloneContext) const
{
    return new LessOrEqualNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void LessOrEqualNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

GreaterOrEqualNode::GreaterOrEqualNode(const Span& span_) : BinaryNode(span_)
{
}

GreaterOrEqualNode::GreaterOrEqualNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* GreaterOrEqualNode::Clone(CloneContext& cloneContext) const
{
    return new GreaterOrEqualNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void GreaterOrEqualNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ShiftLeftNode::ShiftLeftNode(const Span& span_) : BinaryNode(span_)
{
}

ShiftLeftNode::ShiftLeftNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* ShiftLeftNode::Clone(CloneContext& cloneContext) const
{
    return new ShiftLeftNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void ShiftLeftNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ShiftRightNode::ShiftRightNode(const Span& span_) : BinaryNode(span_)
{
}

ShiftRightNode::ShiftRightNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* ShiftRightNode::Clone(CloneContext& cloneContext) const
{
    return new ShiftRightNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void ShiftRightNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

AddNode::AddNode(const Span& span_) : BinaryNode(span_)
{
}

AddNode::AddNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* AddNode::Clone(CloneContext& cloneContext) const
{
    return new AddNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void AddNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

SubNode::SubNode(const Span& span_) : BinaryNode(span_)
{
}

SubNode::SubNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* SubNode::Clone(CloneContext& cloneContext) const
{
    return new SubNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void SubNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

MulNode::MulNode(const Span& span_) : BinaryNode(span_)
{
}

MulNode::MulNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* MulNode::Clone(CloneContext& cloneContext) const
{
    return new MulNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void MulNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

DivNode::DivNode(const Span& span_) : BinaryNode(span_)
{
}

DivNode::DivNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* DivNode::Clone(CloneContext& cloneContext) const
{
    return new DivNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void DivNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

RemNode::RemNode(const Span& span_) : BinaryNode(span_)
{
}

RemNode::RemNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(span_, left_, right_)
{
}

Node* RemNode::Clone(CloneContext& cloneContext) const
{
    return new RemNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void RemNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

NotNode::NotNode(const Span& span_) : UnaryNode(span_)
{
}

NotNode::NotNode(const Span& span_, Node* subject_) : UnaryNode(span_, subject_)
{
}

Node* NotNode::Clone(CloneContext& cloneContext) const
{
    return new NotNode(GetSpan(), Child()->Clone(cloneContext));
}

void NotNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

UnaryPlusNode::UnaryPlusNode(const Span& span_) : UnaryNode(span_)
{
}

UnaryPlusNode::UnaryPlusNode(const Span& span_, Node* subject_) : UnaryNode(span_, subject_)
{
}

Node* UnaryPlusNode::Clone(CloneContext& cloneContext) const
{
    return new UnaryPlusNode(GetSpan(), Child()->Clone(cloneContext));
}

void UnaryPlusNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

UnaryMinusNode::UnaryMinusNode(const Span& span_) : UnaryNode(span_)
{
}

UnaryMinusNode::UnaryMinusNode(const Span& span_, Node* subject_) : UnaryNode(span_, subject_)
{
}

Node* UnaryMinusNode::Clone(CloneContext& cloneContext) const
{
    return new UnaryMinusNode(GetSpan(), Child()->Clone(cloneContext));
}

void UnaryMinusNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ComplementNode::ComplementNode(const Span& span_) : UnaryNode(span_)
{
}

ComplementNode::ComplementNode(const Span& span_, Node* subject_) : UnaryNode(span_, subject_)
{
}

Node* ComplementNode::Clone(CloneContext& cloneContext) const
{
    return new ComplementNode(GetSpan(), Child()->Clone(cloneContext));
}

void ComplementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

IsNode::IsNode(const Span& span_) : Node(span_)
{
}

IsNode::IsNode(const Span& span_, Node* expr_, Node* targetTypeExpr_) : Node(span_), expr(expr_), targetTypeExpr(targetTypeExpr_)
{
}

Node* IsNode::Clone(CloneContext& cloneContext) const
{
    return new IsNode(GetSpan(), expr->Clone(cloneContext), targetTypeExpr->Clone(cloneContext));
}

void IsNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

AsNode::AsNode(const Span& span_) : Node(span_)
{
}

AsNode::AsNode(const Span& span_, Node* expr_, Node* targetTypeExpr_) : Node(span_), expr(expr_), targetTypeExpr(targetTypeExpr_)
{
}

Node* AsNode::Clone(CloneContext& cloneContext) const
{
    return new AsNode(GetSpan(), expr->Clone(cloneContext), targetTypeExpr->Clone(cloneContext));
}

void AsNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

DotNode::DotNode(const Span& span_) : UnaryNode(span_)
{
}

DotNode::DotNode(const Span& span_, Node* subject_, Node* memberId_) : UnaryNode(span_, subject_), memberId(memberId_)
{
}

Node* DotNode::Clone(CloneContext& cloneContext) const
{
    return new DotNode(GetSpan(), Child()->Clone(cloneContext), memberId->Clone(cloneContext));
}

const std::string& DotNode::MemberStr() const
{
    if (IdentifierNode* idNode = dynamic_cast<IdentifierNode*>(memberId.get()))
    {
        return idNode->Str();
    }
    else
    {
        throw std::runtime_error("identifier node expected");
    }
}

void DotNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

InvokeNode::InvokeNode(const Span& span_) : UnaryNode(span_)
{
}

InvokeNode::InvokeNode(const Span& span_, Node* subject_) : UnaryNode(span_, subject_)
{
}

void InvokeNode::AddArgument(Node* argument)
{
    argument->SetParent(this);
    arguments.Add(argument);
}

Node* InvokeNode::Clone(CloneContext& cloneContext) const
{
    InvokeNode* clone = new InvokeNode(GetSpan(), Child()->Clone(cloneContext));
    int n = arguments.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = arguments[i];
        clone->AddArgument(argument->Clone(cloneContext));
    }
    return clone;
}

void InvokeNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::ast
