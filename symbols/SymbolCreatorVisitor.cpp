// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/SymbolCreatorVisitor.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/PropertySymbol.hpp>
#include <cminor/ast/CompileUnit.hpp>
#include <cminor/ast/Statement.hpp>
#include <cminor/ast/Class.hpp>
#include <cminor/ast/Interface.hpp>
#include <cminor/ast/Enumeration.hpp>

namespace cminor { namespace symbols {

SymbolCreatorVisitor::SymbolCreatorVisitor(Assembly& assembly_) : assembly(assembly_), symbolTable(assembly.GetSymbolTable()), classInstanceNode(nullptr), classTemplateSpecialization(nullptr)
{
}

void SymbolCreatorVisitor::Visit(CompileUnitNode& compileUnitNode)
{
    compileUnitNode.GlobalNs()->Accept(*this);
}

void SymbolCreatorVisitor::Visit(NamespaceNode& namespaceNode)
{
    symbolTable.BeginNamespace(namespaceNode);
    NodeList<Node>& members = namespaceNode.Members();
    int n = members.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = members[i];
        member->Accept(*this);
    }
    symbolTable.EndNamespace();
}

void SymbolCreatorVisitor::Visit(FunctionNode& functionNode)
{
    symbolTable.BeginFunction(functionNode);
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
    symbolTable.EndFunction();
}

void SymbolCreatorVisitor::Visit(ClassNode& classNode)
{
    if (&classNode == classInstanceNode)
    {
        symbolTable.BeginClassTemplateSpecialization(*classInstanceNode, classTemplateSpecialization);
    }
    else
    {
        symbolTable.BeginClass(classNode);
    }
    int nt = classNode.TemplateParameters().Count();
    for (int i = 0; i < nt; ++i)
    {
        TemplateParameterNode* templateParameterNode = classNode.TemplateParameters()[i];
        symbolTable.AddTemplateParameter(*templateParameterNode);
    }
    if (nt == 0)
    {
        int n = classNode.Members().Count();
        for (int i = 0; i < n; ++i)
        {
            Node* member = classNode.Members()[i];
            member->Accept(*this);
        }
    }
    if (&classNode == classInstanceNode)
    {
        symbolTable.EndClassTemplateSpecialization();
    }
    else
    {
        symbolTable.EndClass();
    }
}

void SymbolCreatorVisitor::Visit(InterfaceNode& interfaceNode)
{
    symbolTable.BeginInterface(interfaceNode);
    int n = interfaceNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = interfaceNode.Members()[i];
        member->Accept(*this);
    }
    symbolTable.EndInterface();
}

void SymbolCreatorVisitor::Visit(StaticConstructorNode& staticConstructorNode)
{
    symbolTable.BeginStaticConstructor(staticConstructorNode);
    if (staticConstructorNode.HasBody())
    {
        staticConstructorNode.Body()->Accept(*this);
    }
    symbolTable.EndStaticConstructor();
}

void SymbolCreatorVisitor::Visit(ConstructorNode& constructorNode)
{
    symbolTable.BeginConstructor(constructorNode);
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
    symbolTable.EndConstructor();
}

void SymbolCreatorVisitor::Visit(MemberFunctionNode& memberFunctionNode)
{
    symbolTable.BeginMemberFunction(memberFunctionNode);
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
    symbolTable.EndMemberFunction();
}

void SymbolCreatorVisitor::Visit(MemberVariableNode& memberVariableNode)
{
    symbolTable.AddMemberVariable(memberVariableNode);
}

void SymbolCreatorVisitor::Visit(PropertyNode& propertyNode)
{
    symbolTable.BeginProperty(propertyNode);
    if (propertyNode.Getter())
    {
        symbolTable.BeginPropertyGetter(propertyNode);
        propertyNode.Getter()->Accept(*this);
        symbolTable.EndPropertyGetter();
    }
    if (propertyNode.Setter())
    {
        symbolTable.BeginPropertySetter(propertyNode);
        propertyNode.Setter()->Accept(*this);
        symbolTable.EndPropertySetter();
    }
    symbolTable.EndProperty();
}

void SymbolCreatorVisitor::Visit(IndexerNode& indexerNode)
{
    symbolTable.BeginIndexer(indexerNode);
    if (indexerNode.Getter())
    {
        symbolTable.BeginIndexerGetter(indexerNode);
        indexerNode.Getter()->Accept(*this);
        symbolTable.EndIndexerGetter();
    }
    if (indexerNode.Setter())
    {
        symbolTable.BeginIndexerSetter(indexerNode);
        indexerNode.Setter()->Accept(*this);
        symbolTable.EndIndexerSetter();
    }
    symbolTable.EndIndexer();
}

void SymbolCreatorVisitor::Visit(ParameterNode& parameterNode)
{
    symbolTable.AddParameter(parameterNode);
}

void SymbolCreatorVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    symbolTable.BeginDeclarationBlock(compoundStatementNode);
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
    symbolTable.EndDeclarationBlock();
}

void SymbolCreatorVisitor::Visit(IfStatementNode& ifStatementNode)
{
    ifStatementNode.ThenS()->Accept(*this);
    if (ifStatementNode.ElseS())
    {
        ifStatementNode.ElseS()->Accept(*this);
    }
}

void SymbolCreatorVisitor::Visit(WhileStatementNode& whileStatementNode)
{
    whileStatementNode.Statement()->Accept(*this);
}

void SymbolCreatorVisitor::Visit(DoStatementNode& doStatementNode)
{
    doStatementNode.Statement()->Accept(*this);
}

void SymbolCreatorVisitor::Visit(ForStatementNode& forStatementNode)
{
    symbolTable.BeginDeclarationBlock(forStatementNode);
    forStatementNode.InitS()->Accept(*this);
    forStatementNode.ActionS()->Accept(*this);
    symbolTable.EndDeclarationBlock();
}

void SymbolCreatorVisitor::Visit(SwitchStatementNode& switchStatementNode)
{
    int n = switchStatementNode.Cases().Count();
    for (int i = 0; i < n; ++i)
    {
        CaseStatementNode* caseStatementNode = switchStatementNode.Cases()[i];
        caseStatementNode->Accept(*this);
    }
    if (switchStatementNode.Default())
    {
        switchStatementNode.Default()->Accept(*this);
    }
}

void SymbolCreatorVisitor::Visit(CaseStatementNode& caseStatementNode)
{
    int n = caseStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = caseStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
}

void SymbolCreatorVisitor::Visit(DefaultStatementNode& defaultStatementNode)
{
    int n = defaultStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = defaultStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
}

void SymbolCreatorVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    symbolTable.AddLocalVariable(constructionStatementNode);
}

void SymbolCreatorVisitor::Visit(TryStatementNode& tryStatementNode)
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

void SymbolCreatorVisitor::Visit(CatchNode& catchNode)
{
    symbolTable.BeginDeclarationBlock(catchNode);
    symbolTable.AddLocalVariable(*catchNode.Id());
    catchNode.CatchBlock()->Accept(*this);
    symbolTable.EndDeclarationBlock();
}

void SymbolCreatorVisitor::Visit(EnumTypeNode& enumTypeNode)
{
    symbolTable.BeginEnumType(enumTypeNode);
    int n = enumTypeNode.Constants().Count();
    for (int i = 0; i < n; ++i)
    {
        EnumConstantNode* enumConstant = enumTypeNode.Constants()[i];
        symbolTable.AddEnumConstant(*enumConstant);
    }
    symbolTable.EndEnumType();
}

void SymbolCreatorVisitor::Visit(ConstantNode& constantNode)
{
    symbolTable.AddConstant(constantNode);
}

void SymbolCreatorVisitor::Visit(DelegateNode& delegateNode)
{
    symbolTable.BeginDelegate(delegateNode);
    int n = delegateNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = delegateNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    symbolTable.EndDelegate();
}

} } // namespace cminor::symbols
