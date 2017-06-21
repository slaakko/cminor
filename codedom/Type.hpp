// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_CODEDOM_TYPE_INCLUDED
#define CMINOR_CODEDOM_TYPE_INCLUDED
#include <cminor/codedom/Object.hpp>

namespace cminor { namespace codedom {

class DeclSpecifier : public CppObject
{
public:
    DeclSpecifier(const std::u32string& name_);
};

class Typedef : public DeclSpecifier
{
public:
    Typedef();
    virtual void Accept(Visitor& visitor);
};

class TypeSpecifier : public DeclSpecifier
{
public:
    TypeSpecifier(const std::u32string& name_);
    virtual void Accept(Visitor& visitor);
};

class Const : public TypeSpecifier
{
public:
    Const();
};

class Volatile: public TypeSpecifier
{
public:
    Volatile();
};

class TypeName : public TypeSpecifier
{
public:
    TypeName(const std::u32string& name_);
    void AddTemplateArgument(CppObject* templateArgument);
    bool IsTemplate() const { return isTemplate; }
    bool& IsTemplate() { return isTemplate; }
    virtual std::u32string ToString() const;
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    bool isTemplate;
    std::vector<CppObject*> templateArguments;
};

class Type : public CppObject
{
public:
    Type();
    void Add(TypeSpecifier* typeSpecifier);
    virtual std::u32string ToString() const;
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    std::vector<TypeSpecifier*> typeSpecifiers;
};

typedef std::shared_ptr<Type> TypePtr;

class TypeId : public CppObject
{
public:
    typedef std::vector<TypeSpecifier*> TypeSpecifierVec;
    TypeId();
    void Add(TypeSpecifier* typeSpecifier);
    const TypeSpecifierVec& TypeSpecifiers() const { return typeSpecifiers; }
    TypeSpecifierVec& TypeSpecifiers() { return typeSpecifiers; }
    const std::u32string& Declarator() const { return declarator; }
    std::u32string& Declarator() { return declarator; }
    virtual std::u32string ToString() const;
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor);
private:
    TypeSpecifierVec typeSpecifiers;
    std::u32string declarator;
};

class StorageClassSpecifier : public DeclSpecifier
{
public:
    StorageClassSpecifier(const std::u32string& name_): DeclSpecifier(name_) {}
    virtual void Accept(Visitor& visitor);
};

} } // namespace cminor::codedom

#endif // CMINOR_CODEDOM_TYPE_INCLUDED
