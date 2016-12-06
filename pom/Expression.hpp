/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CMINOR_POM_EXPRESSION_INCLUDED
#define CMINOR_POM_EXPRESSION_INCLUDED
#include <cminor/pom/Object.hpp>
#include <cminor/pom/Operator.hpp>

namespace cminor { namespace pom {

void PrintExprs(const std::vector<CppObject*>& expressions, CodeFormatter& formatter);

class Expression : public CppObject
{
public:
    Expression(const std::string& name_);
};

class ThisAccess : public Expression
{
public:
    ThisAccess();
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 25; }
};

class IdExpr : public Expression
{
public:
    IdExpr(const std::string& value_);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 26; }
};

class UnaryExpression : public Expression
{
public:
    UnaryExpression(const std::string& name_, CppObject* child_);
    CppObject* Child() const { return child; }
private:
    CppObject* child;
};

class PostfixExpression : public UnaryExpression
{
public:
    PostfixExpression(const std::string& name_, CppObject* child_);
};

class IndexExpr : public PostfixExpression
{
public:
    IndexExpr(CppObject* child_, CppObject* index_);
    CppObject* Index() const { return index; }
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 18; }
private:
    CppObject* index;
};

class InvokeExpr : public PostfixExpression
{
public:
    InvokeExpr(CppObject* child_, const std::vector<CppObject*>& arguments_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 19; }
private:
    std::vector<CppObject*> arguments;
};

class MemberAccessExpr : public PostfixExpression
{
public:
    MemberAccessExpr(CppObject* child_, CppObject* member_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 20; }
private:
    CppObject* member;
};

class PtrMemberAccessExpr : public PostfixExpression
{
public:
    PtrMemberAccessExpr(CppObject* child_, CppObject* member_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 21; }
private:
    CppObject* member;
};

class PostIncrementExpr : public PostfixExpression
{
public:
    PostIncrementExpr(CppObject* child_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 22; }
};

class PostDecrementExpr : public PostfixExpression
{
public:
    PostDecrementExpr(CppObject* child_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 22; }
};

class PostCastExpr: public PostfixExpression
{
public:
    PostCastExpr(const std::string& name_, CppObject* type_, CppObject* child_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 23; }
private:
    CppObject* type;
};

class TypeIdExpr : public PostfixExpression
{
public:
    TypeIdExpr(CppObject* child_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 24; }
};

class PreIncrementExpr : public UnaryExpression
{
public:
    PreIncrementExpr(CppObject* child_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 14; }
};

class PreDecrementExpr : public UnaryExpression
{
public:
    PreDecrementExpr(CppObject* child_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 14; }
};

class UnaryOpExpr : public UnaryExpression
{
public:
    UnaryOpExpr(Operator op_, CppObject* child_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    Operator Op() const { return op; }
    virtual int Rank() const { return 15; }
private:
    Operator op;
};

class SizeOfExpr : public UnaryExpression
{
public:
    SizeOfExpr(CppObject* child_, bool parens_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 16; }
private:
    bool parens;
};

class CastExpr : public Expression
{
public:
    CastExpr(CppObject* typeId_, CppObject* expr_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 13; }
private:
    CppObject* typeId;
    CppObject* expr;
};

class BinaryExpression : public Expression
{
public:
    BinaryExpression(const std::string& name_, CppObject* left_, CppObject* right_);
    CppObject* Left() const { return left; }
    CppObject* Right() const { return right; }
private:
    CppObject* left;
    CppObject* right;
};

class BinaryOpExpr : public BinaryExpression
{
public:
    BinaryOpExpr(CppObject* left_, Operator op_, int rank_, CppObject* right_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    Operator Op() const { return op; }
    virtual int Rank() const { return rank; }
private:
    Operator op;
    int rank;
};

class ConditionalExpr : public Expression
{
public:
    ConditionalExpr(CppObject* condition_, CppObject* thenExpr_, CppObject* elseExpr_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    CppObject* condition;
    CppObject* thenExpr;
    CppObject* elseExpr;
};

class ThrowExpr : public Expression
{
public:
    ThrowExpr(CppObject* exception_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    CppObject* exception;
};

class TypeId;

class NewExpr : public Expression
{
public:
    NewExpr(bool global_, const std::vector<CppObject*>& placement_, TypeId* typeId_, bool parens_, const std::vector<CppObject*>& initializer_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 17; }
private:
    bool global;
    std::vector<CppObject*> placement;
    TypeId* typeId;
    bool parens;
    std::vector<CppObject*> initializer;
};

class DeleteExpr : public Expression
{
public:
    DeleteExpr(bool global_, bool isArray_, CppObject* expr_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    virtual int Rank() const { return 17; }
private:
    bool global;
    bool isArray;
    CppObject* expr;
};

} } // namespace cminor::pom

#endif // CMINOR_POM_EXPRESSION_INCLUDED

