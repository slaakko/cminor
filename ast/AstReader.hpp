// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_AST_AST_READER_INCLUDED
#define CMINOR_AST_AST_READER_INCLUDED
#include <cminor/machine/Reader.hpp>
#include <cminor/ast/Node.hpp>
#include <cminor/ast/Specifier.hpp>

namespace cminor { namespace ast {

using namespace cminor::machine;

class IdentifierNode;
class CompoundStatementNode;
class ConstructionStatementNode;
class FunctionGroupIdNode;
class StatementNode;

class AstReader : public Reader
{
public:
    AstReader(const std::string& fileName_);
    NodeType GetNodeType();
    Node* GetNode();
    IdentifierNode* GetIdentifierNode();
    CompoundStatementNode* GetCompoundStatementNode();
    ConstructionStatementNode* GetConstructionStatementNode();
    FunctionGroupIdNode* GetFunctionGroupIdNode();
    StatementNode* GetStatementNode();
    Specifiers GetSpecifiers();
};

} } // namespace cminor::ast

#endif // CMINOR_AST_AST_READER_INCLUDED
