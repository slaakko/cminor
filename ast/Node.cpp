// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/ast/Node.hpp>
#include <cminor/ast/AstWriter.hpp>
#include <cminor/ast/AstReader.hpp>
#include <cminor/ast/BasicType.hpp>
#include <cminor/ast/Literal.hpp>
#include <cminor/ast/Expression.hpp>
#include <cminor/ast/Function.hpp>
#include <cminor/ast/Namespace.hpp>
#include <cminor/ast/Class.hpp>
#include <cminor/ast/Interface.hpp>
#include <cminor/ast/Statement.hpp>
#include <cminor/ast/CompileUnit.hpp>

namespace cminor { namespace ast {

const char* nodeTypeStr[] =
{
    "boolNode", "sbyteNode", "byteNode", "shortNode", "ushortNode", "intNode", "uintNode", "longNode", "ulongNode", "floatNode", "doubleNode", "charNode", "stringNode", "voidNode", "objectNode",
    "booleanLiteralNode", "sbyteLiteralNode", "byteLiteralNode", "shortLiteralNode", "ushortLiteralNode", "intLiteralNode", "uintLiteralNode", "longLiteralNode", "ulongLiteralNode",
    "floatLiteralNode", "doubleLiteralNode", "charLiteralNode", "stringLiteralNode", "nullLiteralNode",
    "identifierNode", "parameterNode", "functionGroupIdNode", "functionNode", "namespaceNode", "compileUnitNode", "aliasNode", "namespaceImportNode",
    "disjunctionNode", "conjunctionNode", "bitOrNode", "bitXorNode", "bitAndNode", "equalNode", "notEqualNode", "lessNode", "greaterNode", "lessOrEqualNode", "greaterOrEqualNode",
    "shiftLeftNode", "shiftRightNode", "addNode", "subNode", "mulNode", "divNode", "remNode", "notNode", "unaryPlusNode", "unaryMinusNode", "complementNode", "isNode", "asNode", "dotNode", "arrayNode",
    "indexingNode", "invokeNode", "castNode", "classNode", "interfaceNode", "newNode", "memberVariableNode", "propertyNode", "indexerNode", "staticConstructorNode", "constructorNode", "memberFunctionNode",
    "baseInitializerNode", "thisInitializerNode", "labelNode", "thisNode", "baseNode", "templateIdNode", "templateParameterNode",
    "compoundStatementNode", "returnStatementNode", "ifStatementNode", "whileStatementNode", "doStatementNode", "forStatementNode", "breakStatementNode", "continueStatementNode",
    "constructionStatementNode", "assignmentStatementNode", "expressionStatementNode", "emptyStatementNode", "incrementStatementNode", "decrementStatementNode"
};

std::string NodeTypeStr(NodeType nodeType)
{
    return nodeTypeStr[int(nodeType)];
}

const uint32_t noSymbolId = 0;

Node::Node() : span(), parent(nullptr), symbolId(noSymbolId)
{
}

Node::Node(const Span& span_) : span(span_), parent(nullptr), symbolId(noSymbolId)
{
}

Node::~Node()
{
}

void Node::Write(AstWriter& writer)
{
    writer.AsMachineWriter().PutEncodedUInt(symbolId);
}

void Node::Read(AstReader& reader)
{
    symbolId = reader.GetEncodedUInt();
}

void Node::Accept(Visitor& visitor)
{
    throw std::runtime_error("accept missing");
}

void Node::AddArgument(Node* argument)
{
    throw std::runtime_error("add argument missing");
}

void Node::AddParameter(ParameterNode* parameter)
{
    throw std::runtime_error("add parameter missing");
}

void Node::AddTemplateParameter(TemplateParameterNode* templateParameter)
{
    throw std::runtime_error("add template parameter missing");
}

void Node::SetGetter(CompoundStatementNode* getter)
{
    throw std::runtime_error("set getter missing");
}

void Node::SetSetter(CompoundStatementNode* setter)
{
    throw std::runtime_error("set setter missing");
}


UnaryNode::UnaryNode(const Span& span_) : Node(span_)
{
}

UnaryNode::UnaryNode(const Span& span_, Node* child_) : Node(span_), child(child_)
{
    child->SetParent(this);
}

void UnaryNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(child.get());
}

void UnaryNode::Read(AstReader& reader)
{
    Node::Read(reader);
    child.reset(reader.GetNode());
    child->SetParent(this);
}

void UnaryNode::Accept(Visitor& visitor)
{
    child->Accept(visitor);
}

BinaryNode::BinaryNode(const Span& span_) : Node(span_)
{
}

BinaryNode::BinaryNode(const Span& span_, Node* left_, Node* right_) : Node(span_), left(left_), right(right_)
{
    left->SetParent(this);
    right->SetParent(this);
}

void BinaryNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Put(left.get());
    writer.Put(right.get());
}

void BinaryNode::Read(AstReader& reader)
{
    Node::Read(reader);
    left.reset(reader.GetNode());
    left->SetParent(this);
    right.reset(reader.GetNode());
    right->SetParent(this);
}

void BinaryNode::Accept(Visitor& visitor)
{
    left->Accept(visitor);
    right->Accept(visitor);
}

NodeCreator::~NodeCreator()
{
}

template<typename T>
class ConcreteNodeCreator : public NodeCreator
{
public:
    Node* CreateNode(const Span& span) override
    {
        return new T(span);
    }
};

void NodeFactory::Init()
{
    instance.reset(new NodeFactory());
}

void NodeFactory::Done()
{
    instance.reset();
}

std::unique_ptr<NodeFactory> NodeFactory::instance;

NodeFactory& NodeFactory::Instance()
{
    Assert(instance, "node factory instance not set");
    return *instance;
}

NodeFactory::NodeFactory()
{
    creators.resize(int(NodeType::maxNode));
}

void NodeFactory::Register(NodeType nodeType, NodeCreator* creator)
{
    creators[int(nodeType)] = std::unique_ptr<NodeCreator>(creator);
}

Node* NodeFactory::CreateNode(NodeType nodeType, const Span& span)
{
    const std::unique_ptr<NodeCreator>& creator = creators[int(nodeType)];
    if (creator)
    {
        Node* value = creator->CreateNode(span);
        if (value)
        {
            return value;
        }
        else
        {
            throw std::runtime_error("could not create node");
        }
    }
    else
    {
        throw std::runtime_error("no creator for node type '" + NodeTypeStr(nodeType) + "'");
    }
}

void NodeInit()
{
    NodeFactory::Init();
    NodeFactory::Instance().Register(NodeType::boolNode, new ConcreteNodeCreator<BoolNode>());
    NodeFactory::Instance().Register(NodeType::sbyteNode, new ConcreteNodeCreator<SByteNode>());
    NodeFactory::Instance().Register(NodeType::byteNode, new ConcreteNodeCreator<ByteNode>());
    NodeFactory::Instance().Register(NodeType::shortNode, new ConcreteNodeCreator<ShortNode>());
    NodeFactory::Instance().Register(NodeType::ushortNode, new ConcreteNodeCreator<UShortNode>());
    NodeFactory::Instance().Register(NodeType::intNode, new ConcreteNodeCreator<IntNode>());
    NodeFactory::Instance().Register(NodeType::uintNode, new ConcreteNodeCreator<UIntNode>());
    NodeFactory::Instance().Register(NodeType::longNode, new ConcreteNodeCreator<LongNode>());
    NodeFactory::Instance().Register(NodeType::ulongNode, new ConcreteNodeCreator<ULongNode>());
    NodeFactory::Instance().Register(NodeType::floatNode, new ConcreteNodeCreator<FloatNode>());
    NodeFactory::Instance().Register(NodeType::doubleNode, new ConcreteNodeCreator<DoubleNode>());
    NodeFactory::Instance().Register(NodeType::charNode, new ConcreteNodeCreator<CharNode>());
    NodeFactory::Instance().Register(NodeType::stringNode, new ConcreteNodeCreator<StringNode>());
    NodeFactory::Instance().Register(NodeType::voidNode, new ConcreteNodeCreator<VoidNode>());
    NodeFactory::Instance().Register(NodeType::objectNode, new ConcreteNodeCreator<ObjectNode>());
    NodeFactory::Instance().Register(NodeType::booleanLiteralNode, new ConcreteNodeCreator<BooleanLiteralNode>());
    NodeFactory::Instance().Register(NodeType::sbyteLiteralNode, new ConcreteNodeCreator<SByteLiteralNode>());
    NodeFactory::Instance().Register(NodeType::byteLiteralNode, new ConcreteNodeCreator<ByteLiteralNode>());
    NodeFactory::Instance().Register(NodeType::shortLiteralNode, new ConcreteNodeCreator<ShortLiteralNode>());
    NodeFactory::Instance().Register(NodeType::ushortLiteralNode, new ConcreteNodeCreator<UShortLiteralNode>());
    NodeFactory::Instance().Register(NodeType::intLiteralNode, new ConcreteNodeCreator<IntLiteralNode>());
    NodeFactory::Instance().Register(NodeType::uintLiteralNode, new ConcreteNodeCreator<UIntLiteralNode>());
    NodeFactory::Instance().Register(NodeType::longLiteralNode, new ConcreteNodeCreator<LongLiteralNode>());
    NodeFactory::Instance().Register(NodeType::ulongLiteralNode, new ConcreteNodeCreator<ULongLiteralNode>());
    NodeFactory::Instance().Register(NodeType::floatLiteralNode, new ConcreteNodeCreator<FloatLiteralNode>());
    NodeFactory::Instance().Register(NodeType::doubleLiteralNode, new ConcreteNodeCreator<DoubleLiteralNode>());
    NodeFactory::Instance().Register(NodeType::charLiteralNode, new ConcreteNodeCreator<CharLiteralNode>());
    NodeFactory::Instance().Register(NodeType::stringLiteralNode, new ConcreteNodeCreator<StringLiteralNode>());
    NodeFactory::Instance().Register(NodeType::nullLiteralNode, new ConcreteNodeCreator<NullLiteralNode>());
    NodeFactory::Instance().Register(NodeType::identifierNode, new ConcreteNodeCreator<IdentifierNode>());
    NodeFactory::Instance().Register(NodeType::parameterNode, new ConcreteNodeCreator<ParameterNode>());
    NodeFactory::Instance().Register(NodeType::functionGroupIdNode, new ConcreteNodeCreator<FunctionGroupIdNode>());
    NodeFactory::Instance().Register(NodeType::functionNode, new ConcreteNodeCreator<FunctionNode>());
    NodeFactory::Instance().Register(NodeType::namespaceNode, new ConcreteNodeCreator<NamespaceNode>());
    NodeFactory::Instance().Register(NodeType::compileUnitNode, new ConcreteNodeCreator<CompileUnitNode>());
    NodeFactory::Instance().Register(NodeType::aliasNode, new ConcreteNodeCreator<AliasNode>());
    NodeFactory::Instance().Register(NodeType::namespaceImportNode, new ConcreteNodeCreator<NamespaceImportNode>());
    NodeFactory::Instance().Register(NodeType::disjunctionNode, new ConcreteNodeCreator<DisjunctionNode>());
    NodeFactory::Instance().Register(NodeType::conjunctionNode, new ConcreteNodeCreator<ConjunctionNode>());
    NodeFactory::Instance().Register(NodeType::bitOrNode, new ConcreteNodeCreator<BitOrNode>());
    NodeFactory::Instance().Register(NodeType::bitXorNode, new ConcreteNodeCreator<BitXorNode>());
    NodeFactory::Instance().Register(NodeType::bitAndNode, new ConcreteNodeCreator<BitAndNode>());
    NodeFactory::Instance().Register(NodeType::equalNode, new ConcreteNodeCreator<EqualNode>());
    NodeFactory::Instance().Register(NodeType::notEqualNode, new ConcreteNodeCreator<NotEqualNode>());
    NodeFactory::Instance().Register(NodeType::lessNode, new ConcreteNodeCreator<LessNode>());
    NodeFactory::Instance().Register(NodeType::greaterNode, new ConcreteNodeCreator<GreaterNode>());
    NodeFactory::Instance().Register(NodeType::lessOrEqualNode, new ConcreteNodeCreator<LessOrEqualNode>());
    NodeFactory::Instance().Register(NodeType::greaterOrEqualNode, new ConcreteNodeCreator<GreaterOrEqualNode>());
    NodeFactory::Instance().Register(NodeType::shiftLeftNode, new ConcreteNodeCreator<ShiftLeftNode>());
    NodeFactory::Instance().Register(NodeType::shiftRightNode, new ConcreteNodeCreator<ShiftRightNode>());
    NodeFactory::Instance().Register(NodeType::addNode, new ConcreteNodeCreator<AddNode>());
    NodeFactory::Instance().Register(NodeType::subNode, new ConcreteNodeCreator<SubNode>());
    NodeFactory::Instance().Register(NodeType::mulNode, new ConcreteNodeCreator<MulNode>());
    NodeFactory::Instance().Register(NodeType::divNode, new ConcreteNodeCreator<DivNode>());
    NodeFactory::Instance().Register(NodeType::remNode, new ConcreteNodeCreator<RemNode>());
    NodeFactory::Instance().Register(NodeType::notNode, new ConcreteNodeCreator<NotNode>());
    NodeFactory::Instance().Register(NodeType::unaryPlusNode, new ConcreteNodeCreator<UnaryPlusNode>());
    NodeFactory::Instance().Register(NodeType::unaryMinusNode, new ConcreteNodeCreator<UnaryMinusNode>());
    NodeFactory::Instance().Register(NodeType::complementNode, new ConcreteNodeCreator<ComplementNode>());
    NodeFactory::Instance().Register(NodeType::isNode, new ConcreteNodeCreator<IsNode>());
    NodeFactory::Instance().Register(NodeType::asNode, new ConcreteNodeCreator<AsNode>());
    NodeFactory::Instance().Register(NodeType::dotNode, new ConcreteNodeCreator<DotNode>());
    NodeFactory::Instance().Register(NodeType::arrayNode, new ConcreteNodeCreator<ArrayNode>());
    NodeFactory::Instance().Register(NodeType::indexingNode, new ConcreteNodeCreator<IndexingNode>());
    NodeFactory::Instance().Register(NodeType::invokeNode, new ConcreteNodeCreator<InvokeNode>());
    NodeFactory::Instance().Register(NodeType::castNode, new ConcreteNodeCreator<CastNode>());
    NodeFactory::Instance().Register(NodeType::classNode, new ConcreteNodeCreator<ClassNode>());
    NodeFactory::Instance().Register(NodeType::interfaceNode, new ConcreteNodeCreator<InterfaceNode>());
    NodeFactory::Instance().Register(NodeType::newNode, new ConcreteNodeCreator<NewNode>());
    NodeFactory::Instance().Register(NodeType::memberVariableNode, new ConcreteNodeCreator<MemberVariableNode>());
    NodeFactory::Instance().Register(NodeType::propertyNode, new ConcreteNodeCreator<PropertyNode>());
    NodeFactory::Instance().Register(NodeType::indexerNode, new ConcreteNodeCreator<IndexerNode>());
    NodeFactory::Instance().Register(NodeType::staticConstructorNode, new ConcreteNodeCreator<StaticConstructorNode>());
    NodeFactory::Instance().Register(NodeType::constructorNode, new ConcreteNodeCreator<ConstructorNode>());
    NodeFactory::Instance().Register(NodeType::memberFunctionNode, new ConcreteNodeCreator<MemberFunctionNode>());
    NodeFactory::Instance().Register(NodeType::baseInitializerNode, new ConcreteNodeCreator<BaseInitializerNode>());
    NodeFactory::Instance().Register(NodeType::thisInitializerNode, new ConcreteNodeCreator<ThisInitializerNode>());
    NodeFactory::Instance().Register(NodeType::labelNode, new ConcreteNodeCreator<LabelNode>());
    NodeFactory::Instance().Register(NodeType::thisNode, new ConcreteNodeCreator<ThisNode>());
    NodeFactory::Instance().Register(NodeType::baseNode, new ConcreteNodeCreator<BaseNode>());
    NodeFactory::Instance().Register(NodeType::templateIdNode, new ConcreteNodeCreator<TemplateIdNode>());
    NodeFactory::Instance().Register(NodeType::templateParameterNode, new ConcreteNodeCreator<TemplateParameterNode>());
    NodeFactory::Instance().Register(NodeType::compoundStatementNode, new ConcreteNodeCreator<CompoundStatementNode>());
    NodeFactory::Instance().Register(NodeType::returnStatementNode, new ConcreteNodeCreator<ReturnStatementNode>());
    NodeFactory::Instance().Register(NodeType::ifStatementNode, new ConcreteNodeCreator<IfStatementNode>());
    NodeFactory::Instance().Register(NodeType::whileStatementNode, new ConcreteNodeCreator<WhileStatementNode>());
    NodeFactory::Instance().Register(NodeType::doStatementNode, new ConcreteNodeCreator<DoStatementNode>());
    NodeFactory::Instance().Register(NodeType::forStatementNode, new ConcreteNodeCreator<ForStatementNode>());
    NodeFactory::Instance().Register(NodeType::breakStatementNode, new ConcreteNodeCreator<BreakStatementNode>());
    NodeFactory::Instance().Register(NodeType::continueStatementNode, new ConcreteNodeCreator<ContinueStatementNode>());
    NodeFactory::Instance().Register(NodeType::constructionStatementNode, new ConcreteNodeCreator<ConstructionStatementNode>());
    NodeFactory::Instance().Register(NodeType::assignmentStatementNode, new ConcreteNodeCreator<AssignmentStatementNode>());
    NodeFactory::Instance().Register(NodeType::expressionStatementNode, new ConcreteNodeCreator<ExpressionStatementNode>());
    NodeFactory::Instance().Register(NodeType::emptyStatementNode, new ConcreteNodeCreator<EmptyStatementNode>());
    NodeFactory::Instance().Register(NodeType::incrementStatementNode, new ConcreteNodeCreator<IncrementStatementNode>());
    NodeFactory::Instance().Register(NodeType::decrementStatementNode, new ConcreteNodeCreator<DecrementStatementNode>());
}

void NodeDone()
{
    NodeFactory::Done();
}

} } // namespace cminor::ast
