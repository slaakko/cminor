// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/ConstantPoolInstallerVisitor.hpp>
#include <cminor/ast/Identifier.hpp>
#include <cminor/ast/Literal.hpp>
#include <cminor/ast/Parameter.hpp>
#include <cminor/ast/Function.hpp>
#include <cminor/ast/Statement.hpp>
#include <cminor/ast/Namespace.hpp>
#include <cminor/ast/Class.hpp>
#include <cminor/ast/Constant.hpp>
#include <cminor/ast/Interface.hpp>
#include <cminor/ast/Expression.hpp>
#include <cminor/ast/Enumeration.hpp>
#include <cminor/ast/Delegate.hpp>

namespace cminor { namespace binder {

ConstantPoolInstallerVisitor::ConstantPoolInstallerVisitor(ConstantPool& constantPool_) : constantPool(constantPool_)
{
}

void ConstantPoolInstallerVisitor::Visit(IdentifierNode& identifierNode)
{
    utf32_string s(ToUtf32(identifierNode.Str()));
    constantPool.Install(StringPtr(s.c_str()));
}

void ConstantPoolInstallerVisitor::Visit(StringLiteralNode& stringLiteralNode)
{
    constantPool.Install(StringPtr(stringLiteralNode.Value().c_str()));
}

void ConstantPoolInstallerVisitor::Visit(ParameterNode& parameterNode)
{
    parameterNode.Id()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(FunctionGroupIdNode& functionGroupIdNode)
{
    utf32_string s(ToUtf32(functionGroupIdNode.Str()));
    constantPool.Install(StringPtr(s.c_str()));
}

void ConstantPoolInstallerVisitor::Visit(AttributeMap& attributes)
{
    for (const std::pair<std::string, std::string>& p : attributes.NameValuePairs())
    {
        const std::string& name = p.first;
        const std::string& value = p.second;
        utf32_string n(ToUtf32(name));
        constantPool.Install(StringPtr(n.c_str()));
        utf32_string v(ToUtf32(value));
        constantPool.Install(StringPtr(v.c_str()));
    }
}

void  ConstantPoolInstallerVisitor::Visit(FunctionNode& functionNode)
{
    if (functionNode.ReturnTypeExpr())
    {
        functionNode.ReturnTypeExpr()->Accept(*this);
    }
    functionNode.GroupId()->Accept(*this);
    int n = functionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = functionNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    if (functionNode.HasBody())
    {
        functionNode.Body()->Accept(*this);
    }
    if (functionNode.BodySource())
    {
        functionNode.BodySource()->Accept(*this);
    }
    functionNode.Attributes().Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(NamespaceNode& namespaceNode)
{
    int n = namespaceNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = namespaceNode.Members()[i];
        member->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(AliasNode& aliasNode)
{
    aliasNode.Id()->Accept(*this);
    aliasNode.Qid()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(NamespaceImportNode& namespaceImportNode)
{
    namespaceImportNode.Ns()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(ClassNode& classNode)
{
    classNode.Id()->Accept(*this);
    int nb = classNode.BaseClassOrInterfaces().Count();
    for (int i = 0; i < nb; ++i)
    {
        Node* node = classNode.BaseClassOrInterfaces()[i];
        node->Accept(*this);
    }
    int nm = classNode.Members().Count();
    for (int i = 0; i < nm; ++i)
    {
        Node* node = classNode.Members()[i];
        node->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(InterfaceNode& interfaceNode)
{
    interfaceNode.Id()->Accept(*this);
    int n = interfaceNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = interfaceNode.Members()[i];
        member->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(TemplateIdNode& templateIdNode)
{
    templateIdNode.Primary()->Accept(*this);
    int n = templateIdNode.TemplateArguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = templateIdNode.TemplateArguments()[i];
        argument->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(TemplateParameterNode& templateParameterNode)
{
    templateParameterNode.Id()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(StaticConstructorNode& staticConstructorNode)
{
    staticConstructorNode.GroupId()->Accept(*this);
    if (staticConstructorNode.HasBody())
    {
        staticConstructorNode.Body()->Accept(*this);
    }
    if (staticConstructorNode.BodySource())
    {
        staticConstructorNode.BodySource()->Accept(*this);
    }
    staticConstructorNode.Attributes().Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(BaseInitializerNode& baseInitializerNode)
{
    int n = baseInitializerNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = baseInitializerNode.Arguments()[i];
        argument->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(ThisInitializerNode& thisInitializerNode)
{
    int n = thisInitializerNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = thisInitializerNode.Arguments()[i];
        argument->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(ConstructorNode& constructorNode)
{
    constructorNode.GroupId()->Accept(*this);
    int n = constructorNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameter = constructorNode.Parameters()[i];
        parameter->Accept(*this);
    }
    if (constructorNode.HasBody())
    {
        constructorNode.Body()->Accept(*this);
    }
    if (constructorNode.BodySource())
    {
        constructorNode.BodySource()->Accept(*this);
    }
    constructorNode.Attributes().Accept(*this);
    if (constructorNode.Initializer())
    {
        constructorNode.Initializer()->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(MemberFunctionNode& memberFunctionNode)
{
    if (memberFunctionNode.ReturnTypeExpr())
    {
        memberFunctionNode.ReturnTypeExpr()->Accept(*this);
    }
    memberFunctionNode.GroupId()->Accept(*this);
    int n = memberFunctionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameter = memberFunctionNode.Parameters()[i];
        parameter->Accept(*this);
    }
    if (memberFunctionNode.HasBody())
    {
        memberFunctionNode.Body()->Accept(*this);
    }
    if (memberFunctionNode.BodySource())
    {
        memberFunctionNode.BodySource()->Accept(*this);
    }
    memberFunctionNode.Attributes().Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(MemberVariableNode& memberVariableNode)
{
    memberVariableNode.TypeExpr()->Accept(*this);
    memberVariableNode.Id()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(PropertyNode& propertyNode)
{
    propertyNode.TypeExpr()->Accept(*this);
    propertyNode.Id()->Accept(*this);
    if (propertyNode.Getter())
    {
        propertyNode.Getter()->Accept(*this);
    }
    if (propertyNode.Setter())
    {
        propertyNode.Setter()->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(IndexerNode& indexerNode)
{
    indexerNode.ValueTypeExpr()->Accept(*this);
    indexerNode.IndexTypeExpr()->Accept(*this);
    indexerNode.Id()->Accept(*this);
    if (indexerNode.Getter())
    {
        indexerNode.Getter()->Accept(*this);
    }
    if (indexerNode.Setter())
    {
        indexerNode.Setter()->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(DisjunctionNode& disjunctionNode)
{
    disjunctionNode.Left()->Accept(*this);
    disjunctionNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(ConjunctionNode& conjunctionNode)
{
    conjunctionNode.Left()->Accept(*this);
    conjunctionNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(BitOrNode& bitOrNode)
{
    bitOrNode.Left()->Accept(*this);
    bitOrNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(BitXorNode& bitXorNode)
{
    bitXorNode.Left()->Accept(*this);
    bitXorNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(BitAndNode& bitAndNode)
{
    bitAndNode.Left()->Accept(*this);
    bitAndNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(EqualNode& equalNode)
{
    equalNode.Left()->Accept(*this);
    equalNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(NotEqualNode& notEqualNode)
{
    notEqualNode.Left()->Accept(*this);
    notEqualNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(LessNode& lessNode)
{
    lessNode.Left()->Accept(*this);
    lessNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(GreaterNode& greaterNode)
{
    greaterNode.Left()->Accept(*this);
    greaterNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(LessOrEqualNode& lessOrEqualNode)
{
    lessOrEqualNode.Left()->Accept(*this);
    lessOrEqualNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(GreaterOrEqualNode& greaterOrEqualNode)
{
    greaterOrEqualNode.Left()->Accept(*this);
    greaterOrEqualNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(ShiftLeftNode& shiftLeftNode)
{
    shiftLeftNode.Left()->Accept(*this);
    shiftLeftNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(ShiftRightNode& shiftRightNode)
{
    shiftRightNode.Left()->Accept(*this);
    shiftRightNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(AddNode& addNode)
{
    addNode.Left()->Accept(*this);
    addNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(SubNode& subNode)
{
    subNode.Left()->Accept(*this);
    subNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(MulNode& mulNode)
{
    mulNode.Left()->Accept(*this);
    mulNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(DivNode& divNode)
{
    divNode.Left()->Accept(*this);
    divNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(RemNode& remNode)
{
    remNode.Left()->Accept(*this);
    remNode.Right()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(NotNode& notNode)
{
    notNode.Child()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(UnaryPlusNode& unaryPlusNode)
{
    unaryPlusNode.Child()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(UnaryMinusNode& unaryMinusNode)
{
    unaryMinusNode.Child()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(ComplementNode& complementNode)
{
    complementNode.Child()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(IsNode& isNode)
{
    isNode.Expr()->Accept(*this);
    isNode.TargetTypeExpr()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(AsNode& asNode)
{
    asNode.Expr()->Accept(*this);
    asNode.TargetTypeExpr()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(DotNode& dotNode)
{
    dotNode.Child()->Accept(*this);
    dotNode.MemberId()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(ArrayNode& arrayNode)
{
    arrayNode.Child()->Accept(*this);
    if (arrayNode.Size())
    {
        arrayNode.Size()->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(IndexingNode& indexingNode)
{
    indexingNode.Child()->Accept(*this);
    indexingNode.Index()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(InvokeNode& invokeNode)
{
    invokeNode.Child()->Accept(*this);
    int n = invokeNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = invokeNode.Arguments()[i];
        argument->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(CastNode& castNode)
{
    castNode.TargetTypeExpr()->Accept(*this);
    castNode.SourceExpr()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(NewNode& newNode)
{
    newNode.TypeExpr()->Accept(*this);
    int n = newNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = newNode.Arguments()[i];
        argument->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statement = compoundStatementNode.Statements()[i];
        statement->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(ReturnStatementNode& returnStatementNode)
{
    if (returnStatementNode.Expression())
    {
        returnStatementNode.Expression()->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(IfStatementNode& ifStatementNode)
{
    ifStatementNode.Condition()->Accept(*this);
    ifStatementNode.ThenS()->Accept(*this);
    if (ifStatementNode.ElseS())
    {
        ifStatementNode.ElseS()->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(WhileStatementNode& whileStatementNode)
{
    whileStatementNode.Condition()->Accept(*this);
    whileStatementNode.Statement()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(DoStatementNode& doStatementNode)
{
    doStatementNode.Statement()->Accept(*this);
    doStatementNode.Condition()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(ForStatementNode& forStatementNode)
{
    forStatementNode.InitS()->Accept(*this);
    if (forStatementNode.Condition())
    {
        forStatementNode.Condition()->Accept(*this);
    }
    forStatementNode.LoopS()->Accept(*this);
    forStatementNode.ActionS()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    constructionStatementNode.TypeExpr()->Accept(*this);
    constructionStatementNode.Id()->Accept(*this);
    int n = constructionStatementNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = constructionStatementNode.Arguments()[i];
        argument->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(AssignmentStatementNode& assignmentStatementNode)
{
    assignmentStatementNode.TargetExpr()->Accept(*this);
    assignmentStatementNode.SourceExpr()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(ExpressionStatementNode& expressionStatementNode)
{
    expressionStatementNode.Expression()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(IncrementStatementNode& incrementStatementNode)
{
    incrementStatementNode.Expression()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(DecrementStatementNode& decrementStatementNode)
{
    decrementStatementNode.Expression()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(ForEachStatementNode& forEachStatementNode)
{
    forEachStatementNode.TypeExpr()->Accept(*this);
    forEachStatementNode.Id()->Accept(*this);
    forEachStatementNode.Container()->Accept(*this);
    forEachStatementNode.Action()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(ThrowStatementNode& throwStatementNode)
{
    if (throwStatementNode.Expression())
    {
        throwStatementNode.Expression()->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(CatchNode& catchNode)
{
    if (catchNode.TypeExpr())
    {
        catchNode.TypeExpr()->Accept(*this);
    }
    if (catchNode.Id())
    {
        catchNode.Id()->Accept(*this);
    }
    catchNode.CatchBlock()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(TryStatementNode& tryStatementNode)
{
    tryStatementNode.TryBlock()->Accept(*this);
    int n = tryStatementNode.Catches().Count();
    for (int i = 0; i < n; ++i)
    {
        CatchNode* catchNode = tryStatementNode.Catches()[i];
        catchNode->Accept(*this);
    }
    if (tryStatementNode.FinallyBlock())
    {
        tryStatementNode.FinallyBlock()->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(UsingStatementNode& usingStatementNode)
{
    usingStatementNode.ConstructionStatement()->Accept(*this);
    usingStatementNode.Statement()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(EnumTypeNode& enumTypeNode)
{
    enumTypeNode.Id()->Accept(*this);
    if (enumTypeNode.GetUnderlyingType())
    {
        enumTypeNode.GetUnderlyingType()->Accept(*this);
    }
    int n = enumTypeNode.Constants().Count();
    for (int i = 0; i < n; ++i)
    {
        EnumConstantNode* enumConstant = enumTypeNode.Constants()[i];
        enumConstant->Accept(*this);
    }
}

void ConstantPoolInstallerVisitor::Visit(EnumConstantNode& enumConstantNode)
{
    enumConstantNode.Id()->Accept(*this);
    enumConstantNode.GetValue()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(ConstantNode& constantNode) 
{
    constantNode.TypeExpr()->Accept(*this);
    constantNode.Id()->Accept(*this);
    constantNode.Value()->Accept(*this);
}

void ConstantPoolInstallerVisitor::Visit(DelegateNode& delegateNode)
{
    delegateNode.ReturnTypeExpr()->Accept(*this);
    delegateNode.Id()->Accept(*this);
    int n = delegateNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameter = delegateNode.Parameters()[i];
        parameter->Accept(*this);
    }
}

} } // namespace cminor::binder
