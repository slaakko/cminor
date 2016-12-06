/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/pom/Declaration.hpp>
#include <cminor/pom/Visitor.hpp>
#include <cminor/pom/Expression.hpp>
#include <cminor/pom/Type.hpp>

namespace cminor { namespace pom {

AssignInit::AssignInit(CppObject* assignmentExpr_): CppObject("assignInit"), assignmentExpr(assignmentExpr_) 
{
    Own(assignmentExpr);
}

void AssignInit::Add(AssignInit* subInit) 
{
    Own(subInit);
    subInits.push_back(subInit);
}

void AssignInit::Print(CodeFormatter& formatter)
{
    if (assignmentExpr)
    {
        assignmentExpr->Print(formatter);
    }
    else
    {
        formatter.Write("{ ");
        int n = int(subInits.size());
        for (int i = 0; i < n; ++i)
        {
            if (i > 0)
            {
                formatter.Write(", ");
            }
            subInits[i]->Print(formatter);
        }
        formatter.Write(" }");
    }
}

void AssignInit::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    if (assignmentExpr)
    {
        assignmentExpr->Accept(visitor);
    }
    else
    {
        int n = int(subInits.size());
        for (int i = 0; i < n; ++i)
        {
            subInits[i]->Accept(visitor);
        }
    }
}

Initializer::Initializer(AssignInit* assignInit_, const std::vector<CppObject*>& expressionList_): CppObject("initializer"), assignInit(assignInit_), expressionList(expressionList_) 
{
    Own(assignInit);
    for (CppObject* expr : expressionList)
    {
        Own(expr);
    }
}

void Initializer::Print(CodeFormatter& formatter)
{
    if (assignInit)
    {
        formatter.Write(" = ");
        assignInit->Print(formatter);
    }
    else if (!expressionList.empty())
    {
        formatter.Write("(");
        PrintExprs(expressionList, formatter);
        formatter.Write(")");
    }
}

void Initializer::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    if (assignInit)
    {
        assignInit->Accept(visitor);
    }
    else if (!expressionList.empty())
    {
        for (CppObject* expr : expressionList)
        {
            expr->Accept(visitor);
        }
    }
}

InitDeclarator::InitDeclarator(const std::string& declarator_, Initializer* initializer_): CppObject("initDeclarator"), declarator(declarator_), initializer(initializer_) 
{
    Own(initializer);
}

void InitDeclarator::Print(CodeFormatter& formatter)
{
    formatter.Write(declarator);
    if (initializer)
    {
        initializer->Print(formatter);
    }
}

void InitDeclarator::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    if (initializer)
    {
        initializer->Accept(visitor);
    }
}

InitDeclaratorList::InitDeclaratorList(): CppObject("initDeclaratorList") 
{
}

void InitDeclaratorList::Add(InitDeclarator* initDeclarator) 
{ 
    Own(initDeclarator);
    initDeclarators.push_back(initDeclarator);
}

void InitDeclaratorList::Print(CodeFormatter& formatter)
{
    int n = int(initDeclarators.size());
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            formatter.Write(", ");
        }
        initDeclarators[i]->Print(formatter);
    }
}

void InitDeclaratorList::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    int n = int(initDeclarators.size());
    for (int i = 0; i < n; ++i)
    {
        initDeclarators[i]->Accept(visitor);
    }
}

SimpleDeclaration::SimpleDeclaration(): CppObject("simpleDeclaration"), initDeclaratorList(nullptr)
{
}

void SimpleDeclaration::Add(DeclSpecifier* declSpecifier)
{
    Own(declSpecifier);
    declSpecifiers.push_back(declSpecifier);
}

void SimpleDeclaration::SetInitDeclaratorList(InitDeclaratorList* initDeclaratorList_)
{
    initDeclaratorList = initDeclaratorList_; 
    Own(initDeclaratorList);
}

void SimpleDeclaration::Print(CodeFormatter& formatter)
{
    int n = int(declSpecifiers.size());
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            formatter.Write(" ");
        }
        declSpecifiers[i]->Print(formatter);
    }
    if (n > 0)
    {
        formatter.Write(" ");
    }
    if (initDeclaratorList)
    {
        initDeclaratorList->Print(formatter);
    }
}

void SimpleDeclaration::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
    int n = int(declSpecifiers.size());
    for (int i = 0; i < n; ++i)
    {
        declSpecifiers[i]->Accept(visitor);
    }
    if (initDeclaratorList)
    {
        initDeclaratorList->Accept(visitor);
    }
}

UsingObject::UsingObject(const std::string& name_): CppObject(name_) 
{
}

NamespaceAlias::NamespaceAlias(const std::string& aliasName_, const std::string& namespaceName_):
    UsingObject("namespaceAlias"), aliasName(aliasName_), namespaceName(namespaceName_) 
{
}

void NamespaceAlias::Print(CodeFormatter& formatter)
{
    formatter.WriteLine("namespace " + aliasName + " = " + namespaceName + ";");
}

void NamespaceAlias::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

UsingDeclaration::UsingDeclaration(const std::string& usingId_): 
    UsingObject("usingDeclaration"), usingId(usingId_) 
{
}

void UsingDeclaration::Print(CodeFormatter& formatter)
{
    formatter.WriteLine("using " + usingId + ";");
}

void UsingDeclaration::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

UsingDirective::UsingDirective(const std::string& usingNs_): 
    UsingObject("usingDirective"), usingNs(usingNs_) 
{
}

void UsingDirective::Print(CodeFormatter& formatter)
{
    formatter.WriteLine("using namespace " + usingNs + ";");
}

void UsingDirective::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::pom
