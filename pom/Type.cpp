// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/pom/Type.hpp>
#include <cminor/pom/Visitor.hpp>

namespace cminor { namespace pom {

DeclSpecifier::DeclSpecifier(const std::string& name_): CppObject(name_) 
{
}

Typedef::Typedef(): DeclSpecifier("typedef") 
{
}

void Typedef::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

TypeSpecifier::TypeSpecifier(const std::string& name_): DeclSpecifier(name_) 
{
}

void TypeSpecifier::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

Const::Const(): TypeSpecifier("const") 
{
}

Volatile::Volatile(): TypeSpecifier("volatile") 
{
}

TypeName::TypeName(const std::string& name_): TypeSpecifier(name_), isTemplate(false) 
{
}

void TypeName::AddTemplateArgument(CppObject* templateArgument) 
{ 
    Own(templateArgument);
    templateArguments.push_back(templateArgument); 
}

std::string TypeName::ToString() const
{
    std::string s(Name());
    if (isTemplate)
    {
        s.append("<");
        int n = int(templateArguments.size());
        for (int i = 0; i < n; ++i)
        {
            if (i > 0)
            {
                s.append(", ");
            }
            s.append(templateArguments[i]->ToString());
        }
        s.append(">");
    }
    return s;
}

void TypeName::Print(CodeFormatter& formatter)
{
    formatter.Write(ToString());
}

void TypeName::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

Type::Type(): CppObject("type") 
{
}

void Type::Add(TypeSpecifier* typeSpecifier) 
{
    Own(typeSpecifier);
    typeSpecifiers.push_back(typeSpecifier);
}

std::string Type::ToString() const
{
    std::string s;
    int n = int(typeSpecifiers.size());
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            s.append(" ");
        }
        s.append(typeSpecifiers[i]->ToString());
    }
    return s;
}

void Type::Print(CodeFormatter& formatter)
{
    formatter.Write(ToString());
}

void Type::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

TypeId::TypeId(): CppObject("typeId") 
{
}

void TypeId::Add(TypeSpecifier* typeSpecifier) 
{
    Own(typeSpecifier);
    typeSpecifiers.push_back(typeSpecifier);
}

std::string TypeId::ToString() const
{
    std::string v;
    int n = int(typeSpecifiers.size());
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            v.append(" ");
        }
        v.append(typeSpecifiers[i]->ToString());
    }
    if (!declarator.empty())
    {
        v.append(declarator);
    }
    return v;
}

void TypeId::Print(CodeFormatter& formatter)
{
    int n = int(typeSpecifiers.size());
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            formatter.Write(" ");
        }
        typeSpecifiers[i]->Print(formatter);
    }
    if (!declarator.empty())
    {
        formatter.Write(declarator);
    }
}

void TypeId::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void StorageClassSpecifier::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::pom
