// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_AST_WRITER_INCLUDED
#define CMINOR_AST_AST_WRITER_INCLUDED
#include <cminor/machine/Writer.hpp>
#include <cminor/ast/Node.hpp>
#include <cminor/ast/Specifier.hpp>

namespace cminor { namespace ast {

using namespace cminor::machine;

class IdentifierNode;

class AstWriter : public Writer
{
public:
    AstWriter(const std::string& fileName_);
    Writer& AsMachineWriter() { return *this; }
    void Put(NodeType nodeType);
    void Put(Node* node);
    void Put(Specifiers specifiers);
};

} } // namespace cminor::ast

#endif // CMINOR_AST_AST_WRITER_INCLUDED
