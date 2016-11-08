// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/AstReader.hpp>
#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/Statement.hpp>
#include <cminor/ast/Function.hpp>

namespace cminor { namespace ast {

AstReader::AstReader(const std::string& fileName_) : Reader(fileName_)
{
}

NodeType AstReader::GetNodeType()
{
    uint8_t x = GetByte();
    return static_cast<NodeType>(x);
}

Node* AstReader::GetNode()
{
    NodeType nodeType = GetNodeType();
    Span span = GetSpan();
    Node* node = NodeFactory::Instance().CreateNode(nodeType, span);
    node->Read(*this);
    return node;
}

IdentifierNode* AstReader::GetIdentifierNode()
{
    Node* node = GetNode();
    IdentifierNode* idNode = dynamic_cast<IdentifierNode*>(node);
    Assert(idNode, "identifier node expected");
    return idNode;
}

CompoundStatementNode* AstReader::GetCompoundStatementNode()
{
    Node* node = GetNode();
    CompoundStatementNode* compoundStatementNode = dynamic_cast<CompoundStatementNode*>(node);
    Assert(compoundStatementNode, "compound statement node expected");
    return compoundStatementNode;
}

FunctionGroupIdNode* AstReader::GetFunctionGroupIdNode()
{
    Node* node = GetNode();
    FunctionGroupIdNode* functionGroupIdNode = dynamic_cast<FunctionGroupIdNode*>(node);
    Assert(functionGroupIdNode, "function group id node expected");
    return functionGroupIdNode;
}

StatementNode* AstReader::GetStatementNode()
{
    Node* node = GetNode();
    StatementNode* statementNode = dynamic_cast<StatementNode*>(node);
    Assert(statementNode, "statement node expected");
    return statementNode;
}

Specifiers AstReader::GetSpecifiers()
{
    uint16_t s = GetUShort();
    return static_cast<Specifiers>(s);
}

} } // namespace cminor::ast
