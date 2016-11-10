// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_CLASS_TEMPLATE_REPOSITORY_INCLUDED
#define CMINOR_BINDER_CLASS_TEMPLATE_REPOSITORY_INCLUDED
#include <cminor/symbols/Symbol.hpp>

namespace cminor { namespace binder {

using namespace cminor::symbols;

class BoundCompileUnit;

class ClassTemplateRepository
{
public:
    ClassTemplateRepository(BoundCompileUnit& boundCompileUnit_);
    void BindClassTemplateSpecialization(ClassTemplateSpecializationSymbol* classTemplateSpecialization, ContainerScope* containerScope);
    const std::unordered_set<ClassTemplateSpecializationSymbol*>& ClassTemplateSpecializations() const { return classTemplateSpecializations; }
    void Add(ClassTemplateSpecializationSymbol* classTemplateSpecialization);
    void Clear();
private:
    BoundCompileUnit& boundCompileUnit;
    std::unordered_set<ClassTemplateSpecializationSymbol*> classTemplateSpecializations;
};

} } // namespace cminor::binder

#endif // CMINOR_BINDER_CLASS_TEMPLATE_REPOSITORY_INCLUDED
