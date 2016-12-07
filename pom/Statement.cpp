// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/pom/Statement.hpp>
#include <cminor/pom/Visitor.hpp>
#include <cminor/pom/Type.hpp>

namespace cminor { namespace pom {

Statement::Statement(const std::string& name_): CppObject(name_)
{
}

LabeledStatement::LabeledStatement(const std::string& label_, Statement* statement_): Statement("labeledStatement"), label(label_), statement(statement_) 
{
    Own(statement);
}

void LabeledStatement::Print(CodeFormatter& formatter)
{
    formatter.Write(label);
    formatter.Write(" : ");
    statement->Print(formatter);
}

void LabeledStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    statement->Accept(visitor);
}

CaseStatement::CaseStatement(CppObject* expression_, Statement* statement_): Statement("case"), expression(expression_), statement(statement_) 
{
    Own(expression);
    Own(statement);
}

void CaseStatement::Print(CodeFormatter& formatter)
{
    formatter.Write("case ");
    expression->Print(formatter);
    formatter.Write(": ");
    statement->Print(formatter);
}

void CaseStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    expression->Accept(visitor);
    statement->Accept(visitor);
}

DefaultStatement::DefaultStatement(Statement* statement_): Statement("default"), statement(statement_) 
{
    Own(statement);
}

void DefaultStatement::Print(CodeFormatter& formatter)
{
    formatter.Write("default: ");
    statement->Print(formatter);
}

void DefaultStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    statement->Accept(visitor);
}

EmptyStatement::EmptyStatement(): Statement("empty") 
{
}

void EmptyStatement::Print(CodeFormatter& formatter)
{
    formatter.Write(";");
}

void EmptyStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ExpressionStatement::ExpressionStatement(CppObject* expression_): Statement("expression"), expression(expression_) 
{
    Own(expression);
}

void ExpressionStatement::Print(CodeFormatter& formatter)
{
    expression->Print(formatter);
    formatter.WriteLine(";");
}

void ExpressionStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    expression->Accept(visitor);
}

CompoundStatement::CompoundStatement(): Statement("compound") 
{
}

void CompoundStatement::Add(Statement* statement) 
{ 
    Own(statement);
    statements.push_back(statement); 
}

void CompoundStatement::Print(CodeFormatter& formatter)
{
    formatter.WriteLine("{");
    formatter.IncIndent();
    int n = int(statements.size());
    for (int i = 0; i < n; ++i)
    {
        statements[i]->Print(formatter);
    }
    formatter.DecIndent();
    formatter.WriteLine("}");
}

void CompoundStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    int n = int(statements.size());
    for (int i = 0; i < n; ++i)
    {
        statements[i]->Accept(visitor);
    }
}

SelectionStatement::SelectionStatement(const std::string& name_): Statement(name_) 
{
}

IfStatement::IfStatement(CppObject* condition_, Statement* thenStatement_, Statement* elseStatement_):
    SelectionStatement("if"), condition(condition_), thenStatement(thenStatement_), elseStatement(elseStatement_) 
{
    Own(condition);
    Own(thenStatement);
    Own(elseStatement);
}

void IfStatement::Print(CodeFormatter& formatter)
{
    formatter.Write("if (");
    condition->Print(formatter);
    formatter.Write(")");
    if (thenStatement->IsCompoundStatement())
    {
        formatter.NewLine();
    }
    else
    {
        formatter.Write(" ");
    }
    thenStatement->Print(formatter);
    if (elseStatement)
    {
        formatter.Write("else");
        if (elseStatement->IsCompoundStatement())
        {
            formatter.NewLine();
        }
        else
        {
            formatter.Write(" ");
        }
        elseStatement->Print(formatter);
    }
}

void IfStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    condition->Accept(visitor);
    thenStatement->Accept(visitor);
    if (elseStatement)
    {
        elseStatement->Accept(visitor);
    }
}

SwitchStatement::SwitchStatement(CppObject* condition_, Statement* statement_):
    SelectionStatement("switch"), condition(condition_), statement(statement_) 
{
    Own(condition);
    Own(statement);
}

void SwitchStatement::Print(CodeFormatter& formatter)
{
    formatter.Write("switch (");
    condition->Print(formatter);
    formatter.WriteLine(")");
    statement->Print(formatter);
}

void SwitchStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    condition->Accept(visitor);
    statement->Accept(visitor);
}

IterationStatement::IterationStatement(const std::string& name_): Statement(name_) 
{
}

WhileStatement::WhileStatement(CppObject* condition_, Statement* statement_):
    IterationStatement("while"), condition(condition_), statement(statement_) 
{
    Own(condition);
    Own(statement);
}

void WhileStatement::Print(CodeFormatter& formatter)
{
    formatter.Write("while (");
    condition->Print(formatter);
    formatter.Write(")");
    if (statement->IsCompoundStatement())
    {
        formatter.NewLine();
    }
    else
    {
        formatter.Write(" ");
    }
    statement->Print(formatter);
}

void WhileStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    condition->Accept(visitor);
    statement->Accept(visitor);
}

DoStatement::DoStatement(Statement* statement_, CppObject* condition_):
    IterationStatement("do"), statement(statement_), condition(condition_) 
{
    Own(statement);
    Own(condition);
}

void DoStatement::Print(CodeFormatter& formatter)
{
    formatter.Write("do");
    if (statement->IsCompoundStatement())
    {
        formatter.NewLine();
    }
    else
    {
        formatter.Write(" ");
    }
    statement->Print(formatter);
    formatter.Write("while (");
    condition->Print(formatter);
    formatter.WriteLine(");");
}

void DoStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    statement->Accept(visitor);
    condition->Accept(visitor);
}

ForStatement::ForStatement(CppObject* initialization_, CppObject* condition_, CppObject* iteration_, Statement* statement_):
    IterationStatement("for"), initialization(initialization_), condition(condition_), iteration(iteration_), statement(statement_) 
{
    Own(initialization);
    Own(condition);
    Own(iteration);
    Own(statement);
}

void ForStatement::Print(CodeFormatter& formatter)
{
    formatter.Write("for (");
    if (initialization)
    {
        initialization->Print(formatter);
    }
    formatter.Write("; ");
    if (condition)
    {
        condition->Print(formatter);
    }
    formatter.Write("; ");
    if (iteration)
    {
        iteration->Print(formatter);
    }
    formatter.Write(")");
    if (statement->IsCompoundStatement())
    {
        formatter.NewLine();
    }
    else 
    {
        formatter.Write(" ");
    }
    statement->Print(formatter);
}

void ForStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    if (initialization)
    {
        initialization->Accept(visitor);
    }
    if (condition)
    {
        condition->Accept(visitor);
    }
    if (iteration)
    {
        iteration->Accept(visitor);
    }
    statement->Accept(visitor);
}

JumpStatement::JumpStatement(const std::string& name_): Statement(name_) 
{
}

BreakStatement::BreakStatement(): JumpStatement("break") 
{
}

void BreakStatement::Print(CodeFormatter& formatter)
{
    formatter.WriteLine("break;");
}

void BreakStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ContinueStatement::ContinueStatement(): JumpStatement("continue") 
{
}

void ContinueStatement::Print(CodeFormatter& formatter)
{
    formatter.WriteLine("continue;");
}

void ContinueStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

GotoStatement::GotoStatement(const std::string& target_): JumpStatement("goto"), target(target_) 
{
}

void GotoStatement::Print(CodeFormatter& formatter)
{
    formatter.WriteLine("goto " + target + ";");
}

void GotoStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ReturnStatement::ReturnStatement(CppObject* expression_): JumpStatement("return"), expression(expression_) 
{
    Own(expression);
}

void ReturnStatement::Print(CodeFormatter& formatter)
{
    formatter.Write("return");
    if (expression)
    {
        formatter.Write(" ");
        expression->Print(formatter);
    }
    formatter.WriteLine(";");
}

void ReturnStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    if (expression)
    {
        expression->Accept(visitor);
    }
}

ConditionWithDeclarator::ConditionWithDeclarator(TypeId* type_, const std::string& declarator_, CppObject* expression_):
    CppObject("conditionWithDeclarator"), type(type_), declarator(declarator_), expression(expression_) 
{
    Own(type);
    Own(expression);
}

void ConditionWithDeclarator::Print(CodeFormatter& formatter)
{
    type->Print(formatter);
    formatter.Write(" " + declarator + " = ");
    expression->Print(formatter);
    
}

void ConditionWithDeclarator::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    type->Accept(visitor);
    expression->Accept(visitor);
}

DeclarationStatement::DeclarationStatement(CppObject* declaration_): Statement("declaration"), declaration(declaration_) 
{
    Own(declaration);
}

void DeclarationStatement::Print(CodeFormatter& formatter)
{
    declaration->Print(formatter);
    formatter.WriteLine(";");
}

void DeclarationStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    declaration->Accept(visitor);
}

ExceptionDeclaration::ExceptionDeclaration(): CppObject("exceptionDeclaration"), typeId(new TypeId()), catchAll(false) 
{
    Own(typeId);
}

void ExceptionDeclaration::Print(CodeFormatter& formatter)
{
    if (catchAll)
    {
        formatter.Write("...");
    }
    else
    {
        typeId->Print(formatter);
    }
}

void ExceptionDeclaration::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    typeId->Accept(visitor);
}

Handler::Handler(ExceptionDeclaration* exceptionDeclaration_, CompoundStatement* statement_):
    CppObject("handler"), exceptionDeclaration(exceptionDeclaration_), statement(statement_) 
{
}

void Handler::Print(CodeFormatter& formatter)
{
    formatter.Write("catch (");
    exceptionDeclaration->Print(formatter);
    formatter.WriteLine(")");
    statement->Print(formatter);
}

void Handler::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    exceptionDeclaration->Accept(visitor);
    statement->Accept(visitor);
}

TryStatement::TryStatement(CompoundStatement* statement_): Statement("try"), statement(statement_) 
{
}

void TryStatement::Add(Handler* handler)
{
    Own(handler);
    handlers.push_back(handler);
}

void TryStatement::Print(CodeFormatter& formatter)
{
    formatter.WriteLine("try");
    statement->Print(formatter);
    int n = int(handlers.size());
    for (int i = 0; i < n; ++i)
    {
        handlers[i]->Print(formatter);
    }
}

void TryStatement::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    statement->Accept(visitor);
    int n = int(handlers.size());
    for (int i = 0; i < n; ++i)
    {
        handlers[i]->Accept(visitor);
    }
}

} } // namespace cminor::pom
