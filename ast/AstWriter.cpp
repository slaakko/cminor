// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/AstWriter.hpp>
#include <cminor/ast/Identifier.hpp>

namespace cminor { namespace ast {

AstWriter::AstWriter(const std::string& fileName_) : Writer(fileName_)
{
}

void AstWriter::Put(NodeType nodeType)
{
    AsMachineWriter().Put(static_cast<uint8_t>(nodeType));
}

void AstWriter::Put(Node* node)
{
    Put(node->GetNodeType());
    AsMachineWriter().Put(node->GetSpan());
    node->Write(*this);
}

void AstWriter::Put(Specifiers specifiers)
{
    AsMachineWriter().Put(static_cast<uint16_t>(specifiers));
}

} } // namespace cminor::ast
