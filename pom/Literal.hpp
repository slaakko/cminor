// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_POM_LITERAL_INCLUDED
#define CMINOR_POM_LITERAL_INCLUDED
#include <cminor/pom/Object.hpp>

namespace cminor { namespace pom {

class Literal: public CppObject
{
public:
    Literal(const std::string& name_);
    virtual int Rank() const { return 24; }
    virtual void Accept(Visitor& visitor);
};

} } // namespace cminor::pom

#endif // CMINOR_POM_LITERAL_INCLUDED

