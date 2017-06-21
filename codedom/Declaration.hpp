// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_CODEDOM_DECLARATION_INCLUDED
#define CMINOR_CODEDOM_DECLARATION_INCLUDED
#include <cminor/codedom/Object.hpp>

namespace cminor { namespace codedom {

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
    InitDeclarator(const std::u32string& declarator_, Initializer* initializer_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::u32string declarator;
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
    UsingObject(const std::u32string& name_);
    const std::u32string& FileAttr() const { return fileAttr; }
    void SetFileAttr(const std::u32string& fileAttr_) { fileAttr = fileAttr_; }
private:
    std::u32string fileAttr;
};

class NamespaceAlias : public UsingObject
{
public:
    NamespaceAlias(const std::u32string& aliasName_, const std::u32string& namespaceName_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::u32string aliasName;
    std::u32string namespaceName;
};

class UsingDeclaration : public UsingObject
{
public:
    UsingDeclaration(const std::u32string& usingId_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::u32string usingId;
};

class UsingDirective : public UsingObject
{
public:
    UsingDirective(const std::u32string& usingNs_);
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::u32string usingNs;
};

inline std::vector<CppObject*> GetEmptyVector() { return std::vector<CppObject*>(); }

} } // namespace cminor::codedom

#endif // CMINOR_CODEDOM_DECLARATION_INCLUDED
