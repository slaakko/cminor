// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_POM_DECLARATION_INCLUDED
#define CMINOR_POM_DECLARATION_INCLUDED
#include <cminor/pom/Object.hpp>

namespace cminor { namespace pom {

class AssignInit : public CppObject
{
public:
    AssignInit(CppObject* assignmentExpr_);
    void Add(AssignInit* subInit);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    CppObject* assignmentExpr;
    std::vector<AssignInit*> subInits;
};

class Initializer : public CppObject
{
public:
    Initializer(AssignInit* assignInit_, const std::vector<CppObject*>& expressionList_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    AssignInit* assignInit;
    std::vector<CppObject*> expressionList;
};

class InitDeclarator : public CppObject
{
public:
    InitDeclarator(const std::string& declarator_, Initializer* initializer_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::string declarator;
    Initializer* initializer;
};

class InitDeclaratorList : public CppObject
{
public:
    InitDeclaratorList();
    void Add(InitDeclarator* initDeclarator);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::vector<InitDeclarator*> initDeclarators;
};

class DeclSpecifier;

class SimpleDeclaration : public CppObject
{
public:
    SimpleDeclaration();
    void Add(DeclSpecifier* declSpecifier);
    void SetInitDeclaratorList(InitDeclaratorList* initDeclaratorList_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::vector<DeclSpecifier*> declSpecifiers;
    InitDeclaratorList* initDeclaratorList;
};

class UsingObject : public CppObject
{
public:
    UsingObject(const std::string& name_);
    const std::string& FileAttr() const { return fileAttr; }
    void SetFileAttr(const std::string& fileAttr_) { fileAttr = fileAttr_; }
private:
    std::string fileAttr;
};

class NamespaceAlias : public UsingObject
{
public:
    NamespaceAlias(const std::string& aliasName_, const std::string& namespaceName_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::string aliasName;
    std::string namespaceName;
};

class UsingDeclaration : public UsingObject
{
public:
    UsingDeclaration(const std::string& usingId_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::string usingId;
};

class UsingDirective : public UsingObject
{
public:
    UsingDirective(const std::string& usingNs_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::string usingNs;
};

inline std::vector<CppObject*> GetEmptyVector() { return std::vector<CppObject*>(); }

} } // namespace cminor::pom

#endif // CMINOR_POM_DECLARATION_INCLUDED

