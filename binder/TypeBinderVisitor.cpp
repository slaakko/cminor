// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/TypeBinderVisitor.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/TypeResolver.hpp>
#include <cminor/ast/CompileUnit.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/ObjectFun.hpp>
#include <cminor/machine/Type.hpp>

namespace cminor { namespace binder {

TypeBinderVisitor::TypeBinderVisitor(BoundCompileUnit& boundCompileUnit_) : boundCompileUnit(boundCompileUnit_), containerScope(nullptr)
{
}

void TypeBinderVisitor::Visit(CompileUnitNode& compileUnitNode)
{
    boundCompileUnit.AddFileScope(std::unique_ptr<FileScope>(new FileScope()));
    compileUnitNode.GlobalNs()->Accept(*this);
}

void TypeBinderVisitor::Visit(NamespaceNode& namespaceNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(namespaceNode);
    containerScope = symbol->GetContainerScope();
    int n = namespaceNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = namespaceNode.Members()[i];
        member->Accept(*this);
    }
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(NamespaceImportNode& namespaceImportNode)
{
    boundCompileUnit.FirstFileScope()->InstallNamespaceImport(containerScope, &namespaceImportNode);
}

void TypeBinderVisitor::Visit(AliasNode& aliasNode)
{
    boundCompileUnit.FirstFileScope()->InstallAlias(containerScope, &aliasNode);
}

void TypeBinderVisitor::Visit(FunctionNode& functionNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(functionNode);
    FunctionSymbol* functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
    Assert(functionSymbol, "function symbol expected");
    containerScope = functionSymbol->GetContainerScope();
    Specifiers specifiers = functionNode.GetSpecifiers();
    functionSymbol->SetSpecifiers(specifiers);
    if (functionSymbol->IsExternal())
    {
        if (functionNode.HasBody())
        {
            throw Exception("external function cannot have a body", functionNode.GetSpan());
        }
        utf32_string vmFunctionName = ToUtf32(functionNode.Attributes().GetAttribute("vm_function_name"));
        if (vmFunctionName.empty())
        {
            throw Exception("virtual machine function name attribute (vm_function_name) not set for external function", functionNode.GetSpan());
        }
        functionSymbol->SetVmFunctionName(StringPtr(vmFunctionName.c_str()));
    }
    int n = functionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = functionNode.Parameters()[i];
        TypeSymbol* parameterType = ResolveType(boundCompileUnit, containerScope, parameterNode->TypeExpr());
        Symbol* ps = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(*parameterNode);
        ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(ps);
        Assert(parameterSymbol, "parameter symbol expected");
        parameterSymbol->SetType(parameterType);
    }
    TypeSymbol* returnType = ResolveType(boundCompileUnit, containerScope, functionNode.ReturnTypeExpr());
    functionSymbol->SetReturnType(returnType);
    functionSymbol->ComputeName();
    if (functionNode.HasBody())
    {
        functionNode.Body()->Accept(*this);
    }
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::BindClass(ClassTypeSymbol* classTypeSymbol, ClassNode& classNode)
{
    if (classTypeSymbol->IsBound()) return;
    classTypeSymbol->SetBound();
    classTypeSymbol->SetSpecifiers(classNode.GetSpecifiers());
    utf32_string fullName = classTypeSymbol->FullName();
    ConstantPool& constantPool = boundCompileUnit.GetAssembly().GetConstantPool();
    Constant fullNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullName.c_str())));
    classTypeSymbol->GetObjectType()->SetNameConstant(fullNameConstant);
    ContainerScope* prevContainerScope = containerScope;
    containerScope = classTypeSymbol->GetContainerScope();
    int nb = classNode.BaseClassOrInterfaces().Count();
    for (int i = 0; i < nb; ++i)
    {
        Node* baseOrInterfaceNode = classNode.BaseClassOrInterfaces()[i];
        TypeSymbol* baseOrInterfaceSymbol = ResolveType(boundCompileUnit, containerScope, baseOrInterfaceNode);
        ClassTypeSymbol* baseClassSymbol = dynamic_cast<ClassTypeSymbol*>(baseOrInterfaceSymbol);
        if (baseClassSymbol)
        {
            Node* node = boundCompileUnit.GetAssembly().GetSymbolTable().GetNode(baseClassSymbol);
            ClassNode* baseClassNode = dynamic_cast<ClassNode*>(node);
            Assert(baseClassNode, "class node expected");
            BindClass(baseClassSymbol, *baseClassNode);
            if (classTypeSymbol->BaseClass())
            {
                throw Exception("class type can have at most one base class", classTypeSymbol->GetSpan(), baseClassSymbol->GetSpan());
            }
            else if (baseClassSymbol == classTypeSymbol)
            {
                throw Exception("class cannot derive from itself", classTypeSymbol->GetSpan());
            }
            else
            {
                classTypeSymbol->SetBaseClass(baseClassSymbol);
            }
        }
        else
        {
            throw Exception("symbol '" + ToUtf8(baseOrInterfaceSymbol->FullName()) + "' is not a class or interface type symbol", baseOrInterfaceNode->GetSpan(), baseOrInterfaceSymbol->GetSpan());
        }
    }
    int nm = classNode.Members().Count();
    for (int i = 0; i < nm; ++i)
    {
        Node* member = classNode.Members()[i];
        member->Accept(*this);
    }
    if (!classTypeSymbol->BaseClass())
    {
        TypeSymbol* type = boundCompileUnit.GetAssembly().GetSymbolTable().GetType(U"System.Object");
        ClassTypeSymbol* objectType = dynamic_cast<ClassTypeSymbol*>(type);
        if (objectType != classTypeSymbol)
        {
            Assert(objectType, "class type symbol expected");
            classTypeSymbol->SetBaseClass(objectType);
        }
    }
    if (classTypeSymbol->BaseClass())
    {
        classTypeSymbol->GetContainerScope()->SetBase(classTypeSymbol->BaseClass()->GetContainerScope());
        ObjectType* baseClassObjectType = classTypeSymbol->BaseClass()->GetObjectType();
        classTypeSymbol->GetObjectType()->AddFields(baseClassObjectType->Fields());
    }
    if (!classTypeSymbol->DefaultConstructorSymbol())
    {
        CreateDefaultConstructor(boundCompileUnit.GetAssembly(), classTypeSymbol);
    }
    CreateBasicTypeObjectFun(boundCompileUnit.GetAssembly(), classTypeSymbol);
    int nmf = int(classTypeSymbol->MemberFunctions().size());
    for (int i = 0; i < nmf; ++i)
    {
        MemberFunctionSymbol* memberFunctionSymbol = classTypeSymbol->MemberFunctions()[i];
        if (memberFunctionSymbol->IsVirtualAbstractOrOverride())
        {
            classTypeSymbol->AddVirtualFunction(memberFunctionSymbol);
        }
    }
    classTypeSymbol->InitVmt();
    int nmv = int(classTypeSymbol->MemberVariables().size());
    for (int i = 0; i < nmv; ++i)
    {
        MemberVariableSymbol* memberVariableSymbol = classTypeSymbol->MemberVariables()[i];
        TypeSymbol* memberVariableType = memberVariableSymbol->GetType();
        Assert(memberVariableType, "got no type");
        ValueType memberVariableValueType = memberVariableType->GetValueType();
        classTypeSymbol->GetObjectType()->AddField(memberVariableValueType);
    }
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(ClassNode& classNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(classNode);
    ClassTypeSymbol* classTypeSymbol = dynamic_cast<ClassTypeSymbol*>(symbol);
    Assert(classTypeSymbol, "class type symbol expected");
    BindClass(classTypeSymbol, classNode);
}

void TypeBinderVisitor::Visit(StaticConstructorNode& staticConstructorNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(staticConstructorNode);
    StaticConstructorSymbol* staticConstructorSymbol = dynamic_cast<StaticConstructorSymbol*>(symbol);
    Assert(staticConstructorSymbol, "static constructor function symbol expected");
    staticConstructorSymbol->SetSpecifiers(staticConstructorNode.GetSpecifiers());
    ContainerScope* prevContainerScope = containerScope;
    containerScope = staticConstructorSymbol->GetContainerScope();
    staticConstructorSymbol->ComputeName();
    staticConstructorNode.Body()->Accept(*this);
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(ConstructorNode& constructorNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(constructorNode);
    ConstructorSymbol* constructorSymbol = dynamic_cast<ConstructorSymbol*>(symbol);
    Assert(constructorSymbol, "constructor function symbol expected");
    constructorSymbol->SetSpecifiers(constructorNode.GetSpecifiers());
    ContainerScope* prevContainerScope = containerScope;
    containerScope = constructorSymbol->GetContainerScope();
    int n = constructorNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = constructorNode.Parameters()[i];
        TypeSymbol* parameterType = ResolveType(boundCompileUnit, containerScope, parameterNode->TypeExpr());
        Symbol* ps = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(*parameterNode);
        ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(ps);
        Assert(parameterSymbol, "parameter symbol expected");
        parameterSymbol->SetType(parameterType);
    }
    constructorSymbol->ComputeName();
    if (constructorSymbol->IsDefaultConstructorSymbol())
    {
        constructorSymbol->SetBound();
    }
    constructorNode.Body()->Accept(*this);
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(MemberFunctionNode& memberFunctionNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(memberFunctionNode);
    MemberFunctionSymbol* memberFunctionSymbol = dynamic_cast<MemberFunctionSymbol*>(symbol);
    Assert(memberFunctionSymbol, "member function symbol expected");
    memberFunctionSymbol->SetSpecifiers(memberFunctionNode.GetSpecifiers());
    if (memberFunctionSymbol->IsExternal())
    {
        if (memberFunctionNode.HasBody())
        {
            throw Exception("external function cannot have a body", memberFunctionNode.GetSpan());
        }
        utf32_string vmFunctionName = ToUtf32(memberFunctionNode.Attributes().GetAttribute("vm_function_name"));
        if (vmFunctionName.empty())
        {
            throw Exception("virtual machine function name attribute (vm_function_name) not set for external function", memberFunctionNode.GetSpan());
        }
        memberFunctionSymbol->SetVmFunctionName(StringPtr(vmFunctionName.c_str()));
    }
    ContainerScope* prevContainerScope = containerScope;
    containerScope = memberFunctionSymbol->GetContainerScope();
    int n = memberFunctionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = memberFunctionNode.Parameters()[i];
        TypeSymbol* parameterType = ResolveType(boundCompileUnit, containerScope, parameterNode->TypeExpr());
        Symbol* ps = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(*parameterNode);
        ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(ps);
        Assert(parameterSymbol, "parameter symbol expected");
        parameterSymbol->SetType(parameterType);
    }
    TypeSymbol* returnType = ResolveType(boundCompileUnit, containerScope, memberFunctionNode.ReturnTypeExpr());
    memberFunctionSymbol->SetReturnType(returnType);
    memberFunctionSymbol->ComputeName();
    if (memberFunctionNode.HasBody())
    {
        memberFunctionNode.Body()->Accept(*this);
    }
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(MemberVariableNode& memberVariableNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(memberVariableNode);
    MemberVariableSymbol* memberVariableSymbol = dynamic_cast<MemberVariableSymbol*>(symbol);
    Assert(memberVariableSymbol, "member variable symbol expected");
    memberVariableSymbol->SetSpecifiers(memberVariableNode.GetSpecifiers());
    TypeSymbol* memberVariableType = ResolveType(boundCompileUnit, containerScope, memberVariableNode.TypeExpr());
    memberVariableSymbol->SetType(memberVariableType);
}

void TypeBinderVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    ContainerScope* prevContainerScope = containerScope;
    containerScope = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(compoundStatementNode)->GetContainerScope();
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(ForStatementNode& forStatementNode)
{
    ContainerScope* prevContainerScope = containerScope;
    containerScope = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(forStatementNode)->GetContainerScope();
    forStatementNode.InitS()->Accept(*this);
    containerScope = prevContainerScope;
}

void TypeBinderVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    Symbol* symbol = boundCompileUnit.GetAssembly().GetSymbolTable().GetSymbol(constructionStatementNode);
    LocalVariableSymbol* localVariableSymbol = dynamic_cast<LocalVariableSymbol*>(symbol);
    Assert(localVariableSymbol, "local variable symbol expected");
    TypeSymbol* type = ResolveType(boundCompileUnit, containerScope, constructionStatementNode.TypeExpr());
    localVariableSymbol->SetType(type);
}

} } // namespace cminor::binder
