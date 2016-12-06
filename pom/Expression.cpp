/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/pom/Expression.hpp>
#include <cminor/pom/Visitor.hpp>
#include <cminor/pom/Type.hpp>

namespace cminor { namespace pom {

void PrintExprs(const std::vector<CppObject*>& expressions, CodeFormatter& formatter)
{
    bool first = true;
    for (CppObject* expr : expressions)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            formatter.Write(", ");
        }
        expr->Print(formatter);
    }
}

Expression::Expression(const std::string& name_): CppObject(name_) 
{
}

ThisAccess::ThisAccess(): Expression("this") 
{
}

void ThisAccess::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

IdExpr::IdExpr(const std::string& value_): Expression(value_) 
{
}

void IdExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

UnaryExpression::UnaryExpression(const std::string& name_, CppObject* child_): Expression(name_), child(child_) 
{
    Own(child);
}

PostfixExpression::PostfixExpression(const std::string& name_, CppObject* child_): UnaryExpression(name_, child_) 
{
}

IndexExpr::IndexExpr(CppObject* child_, CppObject* index_): PostfixExpression("[]", child_), index(index_) 
{
    Own(index);
}

void IndexExpr::Print(CodeFormatter& formatter)
{
    Child()->Print(formatter);
    formatter.Write("[");
    index->Print(formatter);
    formatter.Write("]");
}

void IndexExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Child()->Accept(visitor);
}

InvokeExpr::InvokeExpr(CppObject* child_, const std::vector<CppObject*>& arguments_): PostfixExpression("()", child_), arguments(arguments_) 
{
    for (CppObject* arg : arguments)
    {
        Own(arg);
    }
}

void InvokeExpr::Print(CodeFormatter& formatter)
{
    Child()->Print(formatter);
    formatter.Write("(");
    PrintExprs(arguments, formatter);
    formatter.Write(")");
}

void InvokeExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Child()->Accept(visitor);
    for (CppObject* arg : arguments)
    {
        arg->Accept(visitor);
    }
}

MemberAccessExpr::MemberAccessExpr(CppObject* child_, CppObject* member_): PostfixExpression(".", child_), member(member_) 
{
    Own(member);
}

void MemberAccessExpr::Print(CodeFormatter& formatter)
{
    Child()->Print(formatter);
    formatter.Write(".");
    member->Print(formatter);
}

void MemberAccessExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Child()->Accept(visitor);
}

PtrMemberAccessExpr::PtrMemberAccessExpr(CppObject* child_, CppObject* member_): PostfixExpression("->", child_), member(member_) 
{
    Own(member);
}

void PtrMemberAccessExpr::Print(CodeFormatter& formatter)
{
    Child()->Print(formatter);
    formatter.Write("->");
    member->Print(formatter);
}

void PtrMemberAccessExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Child()->Accept(visitor);
}

PostIncrementExpr::PostIncrementExpr(CppObject* child_): PostfixExpression("++", child_) 
{
}

void PostIncrementExpr::Print(CodeFormatter& formatter)
{
    Child()->Print(formatter);
    formatter.Write("++");
}

void PostIncrementExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Child()->Accept(visitor);
}

PostDecrementExpr::PostDecrementExpr(CppObject* child_): PostfixExpression("--", child_) 
{
}

void PostDecrementExpr::Print(CodeFormatter& formatter)
{
    Child()->Print(formatter);
    formatter.Write("--");
}

void PostDecrementExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Child()->Accept(visitor);
}

PostCastExpr::PostCastExpr(const std::string& name_, CppObject* type_, CppObject* child_): PostfixExpression(name_, child_), type(type_) 
{
    Own(type);
}

void PostCastExpr::Print(CodeFormatter& formatter)
{
    formatter.Write(Name() + "<");
    type->Print(formatter);
    formatter.Write(">(");
    Child()->Print(formatter);
    formatter.Write(")");
}

void PostCastExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Child()->Accept(visitor);
    type->Accept(visitor);
}

TypeIdExpr::TypeIdExpr(CppObject* child_): PostfixExpression("typeid", child_) 
{
}

void TypeIdExpr::Print(CodeFormatter& formatter)
{
    formatter.Write("typeid(");
    Child()->Print(formatter);
    formatter.Write(")");
}

void TypeIdExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Child()->Accept(visitor);
}

PreIncrementExpr::PreIncrementExpr(CppObject* child_): UnaryExpression("++", child_) 
{
}

void PreIncrementExpr::Print(CodeFormatter& formatter)
{
    formatter.Write("++");
    Child()->Print(formatter);
}

void PreIncrementExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Child()->Accept(visitor);
}

PreDecrementExpr::PreDecrementExpr(CppObject* child_): UnaryExpression("--", child_) 
{
}

void PreDecrementExpr::Print(CodeFormatter& formatter)
{
    formatter.Write("--");
    Child()->Print(formatter);
}

void PreDecrementExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Child()->Accept(visitor);
}

UnaryOpExpr::UnaryOpExpr(Operator op_, CppObject* child_): UnaryExpression(GetOperatorStr(op_), child_), op(op_) 
{
}

void UnaryOpExpr::Print(CodeFormatter& formatter)
{
    formatter.Write(Name());
    if (Child()->Rank() < Rank())
    {
        formatter.Write("(");
    }
    Child()->Print(formatter);
    if (Child()->Rank() < Rank())
    {
        formatter.Write(")");
    }
}

void UnaryOpExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Child()->Accept(visitor);
}

SizeOfExpr::SizeOfExpr(CppObject* child_, bool parens_): UnaryExpression("sizeof", child_), parens(parens_) 
{
}

void SizeOfExpr::Print(CodeFormatter& formatter)
{
    formatter.Write("sizeof");
    if (parens)
    {
        formatter.Write("(");
    }
    else 
    {
        formatter.Write(" ");
    }
    Child()->Print(formatter);
    if (parens)
    {
        formatter.Write(")");
    }
}

void SizeOfExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Child()->Accept(visitor);
}

CastExpr::CastExpr(CppObject* typeId_, CppObject* expr_): Expression("cast"), typeId(typeId_), expr(expr_) 
{
    Own(typeId);
    Own(expr);
}

void CastExpr::Print(CodeFormatter& formatter)
{
    formatter.Write("(");
    typeId->Print(formatter);
    formatter.Write(")");
    if (expr->Rank() < Rank())
    {
        formatter.Write("(");
    }
    expr->Print(formatter);
    if (expr->Rank() < Rank())
    {
        formatter.Write(")");
    }
}

void CastExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    typeId->Accept(visitor);
    expr->Accept(visitor);
}

BinaryExpression::BinaryExpression(const std::string& name_, CppObject* left_, CppObject* right_): Expression(name_), left(left_), right(right_) 
{
    Own(left);
    Own(right);
}

BinaryOpExpr::BinaryOpExpr(CppObject* left_, Operator op_, int rank_, CppObject* right_): BinaryExpression(GetOperatorStr(op_), left_, right_), op(op_), rank(rank_) 
{
}

void BinaryOpExpr::Print(CodeFormatter& formatter)
{
    if (Left()->Rank() < Rank())
    {
        formatter.Write("(");
    }
    Left()->Print(formatter);
    if (Left()->Rank() < Rank())
    {
        formatter.Write(")");
    }
    formatter.Write(" ");
    formatter.Write(Name());
    formatter.Write(" ");
    if (Right()->Rank() < Rank())
    {
        formatter.Write("(");
    }
    Right()->Print(formatter);
    if (Right()->Rank() < Rank())
    {
        formatter.Write(")");
    }
}

void BinaryOpExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    Left()->Accept(visitor);
    Right()->Accept(visitor);
}

ConditionalExpr::ConditionalExpr(CppObject* condition_, CppObject* thenExpr_, CppObject* elseExpr_): 
    Expression("?:"), condition(condition_), thenExpr(thenExpr_), elseExpr(elseExpr_) 
{
    Own(condition);
    Own(thenExpr);
    Own(elseExpr);
}

void ConditionalExpr::Print(CodeFormatter& formatter)
{
    condition->Print(formatter);
    formatter.Write(" ? ");
    thenExpr->Print(formatter);
    formatter.Write(" : ");
    elseExpr->Print(formatter);
}

void ConditionalExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    condition->Accept(visitor);
    thenExpr->Accept(visitor);
    elseExpr->Accept(visitor);
}

ThrowExpr::ThrowExpr(CppObject* exception_): Expression("throw"), exception(exception_) 
{
    Own(exception);
}

void ThrowExpr::Print(CodeFormatter& formatter)
{
    formatter.Write("throw");
    if (exception)
    {
        formatter.Write(" ");
        exception->Print(formatter);
    }
}

void ThrowExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    if (exception)
    {
        exception->Accept(visitor);
    }
}

NewExpr::NewExpr(bool global_, const std::vector<CppObject*>& placement_, TypeId* typeId_, bool parens_, const std::vector<CppObject*>& initializer_):
    Expression("new"), global(global_), placement(placement_), typeId(typeId_), parens(parens_), initializer(initializer_) 
{
    for (CppObject* p : placement)
    {
        Own(p);
    }
    Own(typeId);
    for (CppObject* i : initializer)
    {
        Own(i);
    }
}

void NewExpr::Print(CodeFormatter& formatter)
{
    if (global)
    {
        formatter.Write("::");
    }
    formatter.Write("new");
    if (!placement.empty())
    {
        formatter.Write("(");
        PrintExprs(placement, formatter);
        formatter.Write(")");
    }
    if (parens)
    {
        formatter.Write("(");
    }
    formatter.Write(" ");
    typeId->Print(formatter);
    if (parens)
    {
        formatter.Write(")");
    }
    if (!initializer.empty())
    {
        formatter.Write("(");
        PrintExprs(initializer, formatter);
        formatter.Write(")");
    }
}

void NewExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    for (CppObject* p : placement)
    {
        p->Accept(visitor);
    }
    if (typeId)
    {
        typeId->Accept(visitor);
    }
    for (CppObject* i : initializer)
    {
        i->Accept(visitor);
    }
}

DeleteExpr::DeleteExpr(bool global_, bool isArray_, CppObject* expr_): Expression("delete"), global(global_), isArray(isArray_), expr(expr_) 
{
    Own(expr);
}

void DeleteExpr::Print(CodeFormatter& formatter)
{
    if (global)
    {
        formatter.Write("::");
    }
    formatter.Write("delete");
    if (isArray)
    {
        formatter.Write("[]");
    }
    formatter.Write(" ");
    expr->Print(formatter);
}

void DeleteExpr::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    expr->Accept(visitor);
}

} } // namespace cminor::pom
