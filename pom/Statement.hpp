/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CMINOR_POM_STATEMENT_INCLUDED
#define CMINOR_POM_STATEMENT_INCLUDED
#include <cminor/pom/Object.hpp>

namespace cminor { namespace pom {

class Statement: public CppObject
{
public:
    Statement(const std::string& name_);
    virtual bool IsCompoundStatement() const { return false; }
};

class LabeledStatement: public Statement
{
public:
    LabeledStatement(const std::string& label_, Statement* statement_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::string label;
    Statement* statement;
};

class CaseStatement: public Statement
{
public:
    CaseStatement(CppObject* expression_, Statement* statement_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    CppObject* expression;
    Statement* statement;
};

class DefaultStatement : public Statement
{
public:
    DefaultStatement(Statement* statement_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    Statement* statement;
};

class EmptyStatement : public Statement
{
public:
    EmptyStatement();
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
};

class ExpressionStatement : public Statement
{
public:
    ExpressionStatement(CppObject* expression_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    CppObject* expression;
};

class CompoundStatement : public Statement
{
public:
    CompoundStatement();
    void Add(Statement* statement);
    virtual bool IsCompoundStatement() const { return true; }
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::vector<Statement*> statements;
};

typedef std::shared_ptr<CompoundStatement> CompoundStatementPtr;

class SelectionStatement : public Statement
{
public:
    SelectionStatement(const std::string& name_);
};

class IfStatement : public SelectionStatement
{
public:
    IfStatement(CppObject* condition_, Statement* thenStatement_, Statement* elseStatement_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    CppObject* condition;
    Statement* thenStatement;
    Statement* elseStatement;
};

class SwitchStatement : public SelectionStatement
{
public:
    SwitchStatement(CppObject* condition_, Statement* statement_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    CppObject* condition;
    Statement* statement;
};

class IterationStatement : public Statement
{
public:
    IterationStatement(const std::string& name_);
};

class WhileStatement : public IterationStatement
{
public:
    WhileStatement(CppObject* condition_, Statement* statement_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    CppObject* condition;
    Statement* statement;
};

class DoStatement : public IterationStatement
{
public:
    DoStatement(Statement* statement_, CppObject* condition_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    Statement* statement;
    CppObject* condition;
};

class ForStatement : public IterationStatement
{
public:
    ForStatement(CppObject* initialization_, CppObject* condition_, CppObject* iteration_, Statement* statement_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    CppObject* initialization;
    CppObject* condition;
    CppObject* iteration;
    Statement* statement;
};

class JumpStatement : public Statement
{
public:
    JumpStatement(const std::string& name_);
};

class BreakStatement : public JumpStatement
{
public:
    BreakStatement();
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
};

class ContinueStatement : public JumpStatement
{
public:
    ContinueStatement();
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
};

class GotoStatement : public JumpStatement
{
public:
    GotoStatement(const std::string& target_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::string target;
};

class ReturnStatement : public JumpStatement
{
public:
    ReturnStatement(CppObject* expression_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    CppObject* expression;
};

class TypeId;

class ConditionWithDeclarator : public CppObject
{
public:
    ConditionWithDeclarator(TypeId* type_, const std::string& declarator_, CppObject* expression_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    TypeId* Type() const { return type; }
    const std::string& Declarator() const { return declarator; }
    CppObject* Expression() const { return expression; }
private:
    TypeId* type;
    std::string declarator;
    CppObject* expression;
};

class DeclarationStatement : public Statement
{
public:
    DeclarationStatement(CppObject* declaration_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    CppObject* declaration;
};

class ExceptionDeclaration : public CppObject
{
public:
    ExceptionDeclaration();
    TypeId* GetTypeId() const { return typeId; }
    bool CatchAll() const { return catchAll; }
    bool& CatchAll() { return catchAll; }
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    TypeId* typeId;
    bool catchAll;
};

typedef std::shared_ptr<ExceptionDeclaration> ExceptionDeclarationPtr;

class Handler : public CppObject
{
public:
    Handler(ExceptionDeclaration* exceptionDeclaration_, CompoundStatement* statement_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    ExceptionDeclaration* exceptionDeclaration;
    CompoundStatement* statement;
};

class TryStatement : public Statement
{
public:
    TryStatement(CompoundStatement* statement_);
    void Add(Handler* handler);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
    CompoundStatement* GetStatement() const { return statement; }
private:
    CompoundStatement* statement;
    std::vector<Handler*> handlers;
};

} } // namespace cminor::pom

#endif // CMINOR_POM_STATEMENT_INCLUDED
