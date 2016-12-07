// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/pom/Literal.hpp>
#include <cminor/pom/Visitor.hpp>

namespace cminor { namespace pom {

Literal::Literal(const std::string& name_): CppObject(name_)
{
}

void Literal::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cminor::pom
