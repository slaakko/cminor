// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/SymbolTable.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/PropertySymbol.hpp>
#include <cminor/symbols/IndexerSymbol.hpp>
#include <cminor/symbols/EnumSymbol.hpp>
#include <cminor/symbols/ConstantSymbol.hpp>
#include <cminor/symbols/DelegateSymbol.hpp>
#include <cminor/symbols/EnumTypeFun.hpp>
#include <cminor/symbols/BasicTypeFun.hpp>
#include <cminor/symbols/ObjectFun.hpp>
#include <cminor/symbols/StringFun.hpp>
#include <cminor/symbols/RefTypeFun.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/util/Unicode.hpp>

namespace cminor { namespace symbols {

using namespace cminor::unicode;

SymbolTable::SymbolTable(Assembly* assembly_) : assembly(assembly_), globalNs(Span(), assembly->GetConstantPool().GetEmptyStringConstant()), container(&globalNs), function(nullptr),
    mainFunction(nullptr), currentClass(nullptr), currentInterface(nullptr), declarationBlockId(0), doNotAddTypes(false), doNotAddClassTemplateSpecializations(false), conversionTable(*assembly), 
    nextSymbolId(firstFreeSymbolId)
{
    globalNs.SetAssembly(assembly);
}

void SymbolTable::BeginContainer(ContainerSymbol* container_)
{
    containerStack.push(container);
    container = container_;
}

void SymbolTable::EndContainer()
{
    container = containerStack.top();
    containerStack.pop();
}

void SymbolTable::BeginNamespace(NamespaceNode& namespaceNode)
{
    std::u32string nsName = namespaceNode.Id()->Str();
    StringPtr namespaceName(nsName.c_str());
    BeginNamespace(namespaceName, namespaceNode.GetSpan());
    MapNode(namespaceNode, container);
}

void SymbolTable::BeginNamespace(StringPtr namespaceName, const Span& span)
{
    if (namespaceName.IsEmpty())
    {
        if (!globalNs.GetSpan().Valid())
        {
            globalNs.SetSpan(span);
        }
        BeginContainer(&globalNs);
    }
    else
    {
        Symbol* symbol = container->GetContainerScope()->Lookup(namespaceName);
        if (symbol)
        {
            if (NamespaceSymbol* ns = dynamic_cast<NamespaceSymbol*>(symbol))
            {
                BeginContainer(ns);
            }
            else
            {
                throw Exception("symbol '" + ToUtf8(symbol->Name().Value()) + "' does not denote a namespace", symbol->GetSpan());
            }
        }
        else
        {
            NamespaceSymbol* ns = container->GetContainerScope()->CreateNamespace(namespaceName, span);
            BeginContainer(ns);
        }
    }
}

void SymbolTable::EndNamespace()
{
    EndContainer();
}

void SymbolTable::BeginFunction(FunctionNode& functionNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = functionNode.Name();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    std::u32string groupName = functionNode.GroupId()->Str();
    Constant groupNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(groupName.c_str())));
    function = new FunctionSymbol(functionNode.GetSpan(), nameConstant);
    function->SetAssembly(assembly);
    function->SetGroupNameConstant(groupNameConstant);
    if (StringPtr(groupNameConstant.Value().AsStringLiteral()) == StringPtr(U"main"))
    {
        if (mainFunction)
        {
            throw Exception("already has main function", functionNode.GetSpan(), mainFunction->GetSpan());
        }
        else
        {
            mainFunction = function;
            mainFunction->SetExported();
        }
    }
    MapNode(functionNode, function);
    ContainerScope* functionScope = function->GetContainerScope();
    functionScope->SetId(function->GetNextContainerScopeId());
    ContainerScope* containerScope = container->GetContainerScope();
    functionScope->SetParent(containerScope);
    BeginContainer(function);
    declarationBlockId = 0;
}

void SymbolTable::EndFunction()
{
    EndContainer();
    function->SetDeclarationBlockId(declarationBlockId);
    container->AddSymbol(std::unique_ptr<Symbol>(function));
    function = nullptr;
}

void SymbolTable::BeginStaticConstructor(StaticConstructorNode& staticConstructorNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(U"@static_constructor")));
    function = new StaticConstructorSymbol(staticConstructorNode.GetSpan(), nameConstant);
    function->SetAssembly(assembly);
    function->SetGroupNameConstant(nameConstant);
    MapNode(staticConstructorNode, function);
    ContainerScope* functionScope = function->GetContainerScope();
    functionScope->SetId(function->GetNextContainerScopeId());
    ContainerScope* containerScope = container->GetContainerScope();
    functionScope->SetParent(containerScope);
    BeginContainer(function);
    declarationBlockId = 0;
}

void SymbolTable::EndStaticConstructor()
{
    EndContainer();
    function->SetDeclarationBlockId(declarationBlockId);
    container->AddSymbol(std::unique_ptr<Symbol>(function));
    function = nullptr;
}

void SymbolTable::BeginConstructor(ConstructorNode& constructorNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(U"@constructor"));
    function = new ConstructorSymbol(constructorNode.GetSpan(), nameConstant);
    function->SetAssembly(assembly);
    function->SetGroupNameConstant(nameConstant);
    MapNode(constructorNode, function);
    ContainerScope* functionScope = function->GetContainerScope();
    functionScope->SetId(function->GetNextContainerScopeId());
    ContainerScope* containerScope = container->GetContainerScope();
    functionScope->SetParent(containerScope);
    BeginContainer(function);
    declarationBlockId = 0;
    Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
    ParameterSymbol* thisParam = new ParameterSymbol(constructorNode.GetSpan(), thisParamName);
    thisParam->SetAssembly(assembly);
    TypeSymbol* thisParamType = currentClass;
    thisParam->SetType(thisParamType);
    thisParam->SetBound();
    function->AddSymbol(std::unique_ptr<Symbol>(thisParam));
}

void SymbolTable::EndConstructor()
{
    EndContainer();
    function->SetDeclarationBlockId(declarationBlockId);
    container->AddSymbol(std::unique_ptr<Symbol>(function));
    function = nullptr;
}

void SymbolTable::BeginMemberFunction(MemberFunctionNode& memberFunctionNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = memberFunctionNode.Name();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    function = new MemberFunctionSymbol(memberFunctionNode.GetSpan(), nameConstant);
    function->SetAssembly(assembly);
    std::u32string groupName = memberFunctionNode.GroupId()->Str();
    Constant groupNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(groupName.c_str())));
    function->SetGroupNameConstant(groupNameConstant);
    MapNode(memberFunctionNode, function);
    ContainerScope* functionScope = function->GetContainerScope();
    functionScope->SetId(function->GetNextContainerScopeId());
    ContainerScope* containerScope = container->GetContainerScope();
    functionScope->SetParent(containerScope);
    BeginContainer(function);
    declarationBlockId = 0;
    if ((memberFunctionNode.GetSpecifiers() & Specifiers::static_) == Specifiers::none)
    {
        Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
        ParameterSymbol* thisParam = new ParameterSymbol(memberFunctionNode.GetSpan(), thisParamName);
        thisParam->SetAssembly(assembly);
        TypeSymbol* thisParamType = nullptr;
        if (currentClass)
        {
            thisParamType = currentClass;
        }
        else if (currentInterface)
        {
            thisParamType = currentInterface;
        }
        else
        {
            Assert(false, "class or interface expected");
        }
        thisParam->SetType(thisParamType);
        thisParam->SetBound();
        function->AddSymbol(std::unique_ptr<Symbol>(thisParam));
    }
}

void SymbolTable::EndMemberFunction()
{
    EndContainer();
    function->SetDeclarationBlockId(declarationBlockId);
    container->AddSymbol(std::unique_ptr<Symbol>(function));
    function = nullptr;
}

void SymbolTable::BeginClass(ClassNode& classNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = classNode.Id()->Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    ClassTypeSymbol* classTypeSymbol = new ClassTypeSymbol(classNode.GetSpan(), nameConstant);
    classTypeSymbol->SetAssembly(assembly);
    classStack.push(currentClass);
    currentClass = classTypeSymbol;
    MapNode(classNode, classTypeSymbol);
    ContainerScope* containerScope = container->GetContainerScope();
    ContainerScope* classScope = classTypeSymbol->GetContainerScope();
    classScope->SetParent(containerScope);
    container->AddSymbol(std::unique_ptr<Symbol>(classTypeSymbol));
    BeginContainer(classTypeSymbol);
}

void SymbolTable::EndClass()
{
    EndContainer();
    currentClass = classStack.top();
    classStack.pop();
}

void SymbolTable::BeginClassTemplateSpecialization(ClassNode& classInstanceNode, ClassTemplateSpecializationSymbol* classTemplateSpecialization)
{
    classStack.push(currentClass);
    currentClass = classTemplateSpecialization;
    MapNode(classInstanceNode, classTemplateSpecialization);
    ContainerScope* containerScope = container->GetContainerScope();
    ContainerScope* classScope = classTemplateSpecialization->GetContainerScope();
    classScope->SetParent(containerScope);
    container->AddSymbol(std::unique_ptr<Symbol>(classTemplateSpecialization));
    BeginContainer(classTemplateSpecialization);
}

void SymbolTable::EndClassTemplateSpecialization()
{
    EndContainer();
    currentClass = classStack.top();
    classStack.pop();
}

void SymbolTable::BeginInterface(InterfaceNode& interfaceNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = interfaceNode.Id()->Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    InterfaceTypeSymbol* interfaceTypeSymbol = new InterfaceTypeSymbol(interfaceNode.GetSpan(), nameConstant);
    interfaceTypeSymbol->SetAssembly(assembly);
    interfaceStack.push(currentInterface);
    currentInterface = interfaceTypeSymbol;
    MapNode(interfaceNode, interfaceTypeSymbol);
    ContainerScope* containerScope = container->GetContainerScope();
    ContainerScope* interfaceScope = interfaceTypeSymbol->GetContainerScope();
    interfaceScope->SetParent(containerScope);
    container->AddSymbol(std::unique_ptr<Symbol>(interfaceTypeSymbol));
    BeginContainer(interfaceTypeSymbol);
}

void SymbolTable::EndInterface()
{
    currentInterface = interfaceStack.top();
    interfaceStack.pop();
    EndContainer();
}

void SymbolTable::AddParameter(ParameterNode& parameterNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = parameterNode.Id()->Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    ParameterSymbol* parameter = new ParameterSymbol(parameterNode.GetSpan(), nameConstant);
    parameter->SetAssembly(assembly);
    container->AddSymbol(std::unique_ptr<Symbol>(parameter));
    MapNode(parameterNode, parameter);
}

void SymbolTable::AddTemplateParameter(TemplateParameterNode& templateParameterNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = templateParameterNode.Id()->Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    TypeParameterSymbol* typeParameter = new TypeParameterSymbol(templateParameterNode.GetSpan(), nameConstant);
    typeParameter->SetAssembly(assembly);
    container->AddSymbol(std::unique_ptr<Symbol>(typeParameter));
    MapNode(templateParameterNode, typeParameter);
}

void SymbolTable::BeginDeclarationBlock(Node& node)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = U"@locals" + ToUtf32(std::to_string(declarationBlockId++));
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    DeclarationBlock* declarationBlock = new DeclarationBlock(node.GetSpan(), nameConstant);
    declarationBlock->SetAssembly(assembly);
    ContainerScope* declarationBlockScope = declarationBlock->GetContainerScope();
    if (!function)
    {
        throw std::runtime_error("function not set");
    }
    declarationBlockScope->SetId(function->GetNextContainerScopeId());
    ContainerScope* containerScope = container->GetContainerScope();
    declarationBlockScope->SetParent(containerScope);
    container->AddSymbol(std::unique_ptr<Symbol>(declarationBlock));
    MapNode(node, declarationBlock);
    BeginContainer(declarationBlock);
}

void SymbolTable::EndDeclarationBlock()
{
    EndContainer();
}

void SymbolTable::AddLocalVariable(ConstructionStatementNode& constructionStatementNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = constructionStatementNode.Id()->Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    LocalVariableSymbol* localVariableSymbol = new LocalVariableSymbol(constructionStatementNode.GetSpan(), nameConstant);
    localVariableSymbol->SetAssembly(assembly);
    container->AddSymbol(std::unique_ptr<Symbol>(localVariableSymbol));
    MapNode(constructionStatementNode, localVariableSymbol);
}

void SymbolTable::AddLocalVariable(IdentifierNode& idNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = idNode.Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    LocalVariableSymbol* localVariableSymbol = new LocalVariableSymbol(idNode.GetSpan(), nameConstant);
    localVariableSymbol->SetAssembly(assembly);
    container->AddSymbol(std::unique_ptr<Symbol>(localVariableSymbol));
    MapNode(idNode, localVariableSymbol);
}

void SymbolTable::AddMemberVariable(MemberVariableNode& memberVariableNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = memberVariableNode.Id()->Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    MemberVariableSymbol* memberVariableSymbol = new MemberVariableSymbol(memberVariableNode.GetSpan(), nameConstant);
    memberVariableSymbol->SetAssembly(assembly);
    if ((memberVariableNode.GetSpecifiers() & Specifiers::static_) != Specifiers::none)
    {
        memberVariableSymbol->SetStatic();
    }
    container->AddSymbol(std::unique_ptr<Symbol>(memberVariableSymbol));
    MapNode(memberVariableNode, memberVariableSymbol);
}

void SymbolTable::BeginProperty(PropertyNode& propertyNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = propertyNode.Id()->Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    PropertySymbol* propertySymbol = new PropertySymbol(propertyNode.GetSpan(), nameConstant);
    propertySymbol->SetAssembly(assembly);
    MapNode(propertyNode, propertySymbol);
    ContainerScope* containerScope = container->GetContainerScope();
    ContainerScope* propertyScope = propertySymbol->GetContainerScope();
    propertyScope->SetParent(containerScope);
    container->AddSymbol(std::unique_ptr<Symbol>(propertySymbol));
    BeginContainer(propertySymbol);
}

void SymbolTable::EndProperty()
{
    EndContainer();
}

void SymbolTable::BeginPropertyGetter(PropertyNode& propertyNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string getterName = U"@get";
    Constant getterNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(getterName.c_str())));
    PropertyGetterFunctionSymbol* getter = new PropertyGetterFunctionSymbol(propertyNode.Getter()->GetSpan(), getterNameConstant);
    function = getter;
    getter->SetAssembly(assembly);
    getter->SetGroupNameConstant(getterNameConstant);
    getter->SetPublic();
    ContainerScope* containerScope = container->GetContainerScope();
    ContainerScope* getterScope = getter->GetContainerScope();
    getterScope->SetId(getter->GetNextContainerScopeId());
    getterScope->SetParent(containerScope);
    container->AddSymbol(std::unique_ptr<Symbol>(getter));
    BeginContainer(getter);
    declarationBlockId = 0;
    if ((propertyNode.GetSpecifiers() & Specifiers::static_) == Specifiers::none)
    {
        Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
        ParameterSymbol* thisParam = new ParameterSymbol(propertyNode.GetSpan(), thisParamName);
        thisParam->SetAssembly(assembly);
        TypeSymbol* thisParamType = currentClass;
        thisParam->SetType(thisParamType);
        thisParam->SetBound();
        getter->AddSymbol(std::unique_ptr<Symbol>(thisParam));
    }
    else
    {
        getter->SetStatic();
    }
}

void SymbolTable::EndPropertyGetter()
{
    EndContainer();
}

void SymbolTable::BeginPropertySetter(PropertyNode& propertyNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string setterName = U"@set";
    Constant setterNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(setterName.c_str())));
    PropertySetterFunctionSymbol* setter = new PropertySetterFunctionSymbol(propertyNode.Setter()->GetSpan(), setterNameConstant);
    function = setter;
    setter->SetAssembly(assembly);
    setter->SetGroupNameConstant(setterNameConstant);
    setter->SetPublic();
    ContainerScope* containerScope = container->GetContainerScope();
    ContainerScope* setterScope = setter->GetContainerScope();
    setterScope->SetId(setter->GetNextContainerScopeId());
    setterScope->SetParent(containerScope);
    container->AddSymbol(std::unique_ptr<Symbol>(setter));
    BeginContainer(setter);
    if ((propertyNode.GetSpecifiers() & Specifiers::static_) == Specifiers::none)
    {
        Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
        ParameterSymbol* thisParam = new ParameterSymbol(propertyNode.GetSpan(), thisParamName);
        thisParam->SetAssembly(assembly);
        TypeSymbol* thisParamType = currentClass;
        thisParam->SetType(thisParamType);
        thisParam->SetBound();
        setter->AddSymbol(std::unique_ptr<Symbol>(thisParam));
    }
    else
    {
        setter->SetStatic();
    }
    declarationBlockId = 0;
    Constant valueConstantName = constantPool.GetConstant(constantPool.Install(U"value"));
    ParameterSymbol* valueParam = new ParameterSymbol(propertyNode.GetSpan(), valueConstantName);
    valueParam->SetAssembly(assembly);
    setter->AddSymbol(std::unique_ptr<Symbol>(valueParam));
}

void SymbolTable::EndPropertySetter()
{
    EndContainer();
}

void SymbolTable::BeginIndexer(IndexerNode& indexerNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = U"this";
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    IndexerSymbol* indexerSymbol = new IndexerSymbol(indexerNode.GetSpan(), nameConstant);
    indexerSymbol->SetAssembly(assembly);
    MapNode(indexerNode, indexerSymbol);
    ContainerScope* containerScope = container->GetContainerScope();
    ContainerScope* indexerScope = indexerSymbol->GetContainerScope();
    indexerScope->SetParent(containerScope);
    container->AddSymbol(std::unique_ptr<Symbol>(indexerSymbol));
    BeginContainer(indexerSymbol);
}

void SymbolTable::EndIndexer()
{
    EndContainer();
}

void SymbolTable::BeginIndexerGetter(IndexerNode& indexerNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string getterName = U"@get";
    Constant getterNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(getterName.c_str())));
    IndexerGetterFunctionSymbol* getter = new IndexerGetterFunctionSymbol(indexerNode.Getter()->GetSpan(), getterNameConstant);
    function = getter;
    getter->SetAssembly(assembly);
    getter->SetGroupNameConstant(getterNameConstant);
    getter->SetPublic();
    ContainerScope* containerScope = container->GetContainerScope();
    ContainerScope* getterScope = getter->GetContainerScope();
    getterScope->SetId(getter->GetNextContainerScopeId());
    getterScope->SetParent(containerScope);
    container->AddSymbol(std::unique_ptr<Symbol>(getter));
    BeginContainer(getter);
    declarationBlockId = 0;
    if ((indexerNode.GetSpecifiers() & Specifiers::static_) == Specifiers::none)
    {
        Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
        ParameterSymbol* thisParam = new ParameterSymbol(indexerNode.GetSpan(), thisParamName);
        thisParam->SetAssembly(assembly);
        TypeSymbol* thisParamType = currentClass;
        thisParam->SetType(thisParamType);
        thisParam->SetBound();
        getter->AddSymbol(std::unique_ptr<Symbol>(thisParam));
    }
    else
    {
        getter->SetStatic();
    }
    std::u32string indexParamName = indexerNode.Id()->Str();
    Constant indexParamNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(indexParamName.c_str())));
    ParameterSymbol* indexParam = new ParameterSymbol(indexerNode.Id()->GetSpan(), indexParamNameConstant);
    indexParam->SetAssembly(assembly);
    getter->AddSymbol(std::unique_ptr<Symbol>(indexParam));
}

void SymbolTable::EndIndexerGetter()
{
    EndContainer();
}

void SymbolTable::BeginIndexerSetter(IndexerNode& indexerNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string setterName = U"@set";
    Constant setterNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(setterName.c_str())));
    IndexerSetterFunctionSymbol* setter = new IndexerSetterFunctionSymbol(indexerNode.Setter()->GetSpan(), setterNameConstant);
    function = setter;
    setter->SetAssembly(assembly);
    setter->SetGroupNameConstant(setterNameConstant);
    setter->SetPublic();
    ContainerScope* containerScope = container->GetContainerScope();
    ContainerScope* setterScope = setter->GetContainerScope();
    setterScope->SetId(setter->GetNextContainerScopeId());
    setterScope->SetParent(containerScope);
    container->AddSymbol(std::unique_ptr<Symbol>(setter));
    BeginContainer(setter);
    if ((indexerNode.GetSpecifiers() & Specifiers::static_) == Specifiers::none)
    {
        Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
        ParameterSymbol* thisParam = new ParameterSymbol(indexerNode.GetSpan(), thisParamName);
        thisParam->SetAssembly(assembly);
        TypeSymbol* thisParamType = currentClass;
        thisParam->SetType(thisParamType);
        thisParam->SetBound();
        setter->AddSymbol(std::unique_ptr<Symbol>(thisParam));
    }
    else
    {
        setter->SetStatic();
    }
    declarationBlockId = 0;
    std::u32string indexParamName = indexerNode.Id()->Str();
    Constant indexParamNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(indexParamName.c_str())));
    ParameterSymbol* indexParam = new ParameterSymbol(indexerNode.Id()->GetSpan(), indexParamNameConstant);
    indexParam->SetAssembly(assembly);
    setter->AddSymbol(std::unique_ptr<Symbol>(indexParam));
    Constant valueConstantName = constantPool.GetConstant(constantPool.Install(U"value"));
    ParameterSymbol* valueParam = new ParameterSymbol(indexerNode.GetSpan(), valueConstantName);
    valueParam->SetAssembly(assembly);
    setter->AddSymbol(std::unique_ptr<Symbol>(valueParam));
}

void SymbolTable::EndIndexerSetter()
{
    EndContainer();
}

void SymbolTable::BeginEnumType(EnumTypeNode& enumTypeNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = enumTypeNode.Id()->Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    EnumTypeSymbol* enumTypeSymbol = new EnumTypeSymbol(enumTypeNode.GetSpan(), nameConstant);
    enumTypeSymbol->SetAssembly(assembly);
    MapNode(enumTypeNode, enumTypeSymbol);
    ContainerScope* containerScope = container->GetContainerScope();
    ContainerScope* enumTypeScope = enumTypeSymbol->GetContainerScope();
    enumTypeScope->SetParent(containerScope);
    container->AddSymbol(std::unique_ptr<Symbol>(enumTypeSymbol));
    BeginContainer(enumTypeSymbol);
}

void SymbolTable::EndEnumType()
{
    EndContainer();
}

void SymbolTable::AddEnumConstant(EnumConstantNode& enumConstantNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = enumConstantNode.Id()->Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    EnumConstantSymbol* enumConstantSymbol = new EnumConstantSymbol(enumConstantNode.GetSpan(), nameConstant);
    enumConstantSymbol->SetAssembly(assembly);
    container->AddSymbol(std::unique_ptr<Symbol>(enumConstantSymbol));
    MapNode(enumConstantNode, enumConstantSymbol);
}

void SymbolTable::AddConstant(ConstantNode& constantNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = constantNode.Id()->Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    ConstantSymbol* constantSymbol = new ConstantSymbol(constantNode.GetSpan(), nameConstant);
    constantSymbol->SetAssembly(assembly);
    container->AddSymbol(std::unique_ptr<Symbol>(constantSymbol));
    MapNode(constantNode, constantSymbol);
}

void SymbolTable::BeginDelegate(DelegateNode& delegateNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = delegateNode.Id()->Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    DelegateTypeSymbol* delegateTypeSymbol = new DelegateTypeSymbol(delegateNode.GetSpan(), nameConstant);
    delegateTypeSymbol->SetAssembly(assembly);
    container->AddSymbol(std::unique_ptr<Symbol>(delegateTypeSymbol));
    MapNode(delegateNode, delegateTypeSymbol);
    ContainerScope* delegateScope = delegateTypeSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    delegateScope->SetParent(containerScope);
    BeginContainer(delegateTypeSymbol);
}

void SymbolTable::EndDelegate()
{
    EndContainer();
}

void SymbolTable::BeginClassDelegate(ClassDelegateNode& classDelegateNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string name = classDelegateNode.Id()->Str();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    ClassDelegateTypeSymbol* classDelegateTypeSymbol = new ClassDelegateTypeSymbol(classDelegateNode.GetSpan(), nameConstant);
    classDelegateTypeSymbol->SetAssembly(assembly);
    container->AddSymbol(std::unique_ptr<Symbol>(classDelegateTypeSymbol));
    MapNode(classDelegateNode, classDelegateTypeSymbol);
    ContainerScope* classDelegateScope = classDelegateTypeSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    classDelegateScope->SetParent(containerScope);
    BeginContainer(classDelegateTypeSymbol);
}

void SymbolTable::EndClassDelegate()
{
    EndContainer();
}

void SymbolTable::Write(SymbolWriter& writer)
{
    globalNs.Write(writer);
    bool hasMainFunction = mainFunction != nullptr;
    writer.AsMachineWriter().Put(hasMainFunction);
}

void SymbolTable::Read(SymbolReader& reader)
{
    NamespaceSymbol ns(Span(), assembly->GetConstantPool().GetEmptyStringConstant());
    ns.SetAssembly(assembly);
    bool prevDoNotAddClassTemplateSpecializations = doNotAddClassTemplateSpecializations;
    bool prevDoNotAddTypes = doNotAddTypes;
    doNotAddTypes = true;
    doNotAddClassTemplateSpecializations = true;
    ns.Read(reader);
    globalNs.Import(&ns, *this);
    doNotAddTypes = prevDoNotAddTypes;
    for (TypeSymbol* type : reader.Types())
    {
        AddType(type);
    }
    reader.ClearTypes();
    doNotAddClassTemplateSpecializations = prevDoNotAddClassTemplateSpecializations;
    bool hasMainFunction = reader.GetBool();
    if (hasMainFunction)
    {
        Symbol* symbol = globalNs.GetContainerScope()->Lookup(StringPtr(U"main"));
        if (FunctionGroupSymbol* mainFunctionGroup = dynamic_cast<FunctionGroupSymbol*>(symbol))
        {
            mainFunction = mainFunctionGroup->GetOverload();
            if (!mainFunction)
            {
                throw std::runtime_error("main function not found from main function group");
            }
        }
        else
        {
            Assert(false, "invalid main function symbol");
        }
    }
    reader.ProcessTypeRequests();
    for (FunctionSymbol* conversion : reader.Conversions())
    {
        AddConversion(conversion);
    }
    reader.ClearConversions();
    for (ClassTemplateSpecializationSymbol* classTemplateSpecialization : reader.ClassTemplateSpecializations())
    {
        if (!classTemplateSpecialization->IsReopened())
        {
            classTemplateSpecializationMap[classTemplateSpecialization->Key()] = classTemplateSpecialization;
        }
    }
    reader.ClearClassTemplateSpecializations();
}

void SymbolTable::Import(SymbolTable& symbolTable)
{
    globalNs.Import(&symbolTable.globalNs, *this);
    conversionTable.ImportConversionMap(symbolTable.conversionTable.ConversionMap());
    symbolTable.Clear();
}

void SymbolTable::Clear()
{
    globalNs.Clear();
}

TypeSymbol* SymbolTable::GetTypeNoThrow(const std::u32string& typeFullName) const
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    ConstantId id = constantPool.GetIdFor(typeFullName);
    if (id != noConstantId)
    {
        Constant c = constantPool.GetConstant(id);
        auto it = typeSymbolMap.find(c);
        if (it != typeSymbolMap.cend())
        {
            return it->second;
        }
    }
    return nullptr;
}

TypeSymbol* SymbolTable::GetType(const std::u32string& typeFullName) const
{
    TypeSymbol* type = GetTypeNoThrow(typeFullName);
    if (type)
    {
        return type;
    }
    else
    {
        throw std::runtime_error("type '" + ToUtf8(typeFullName) + "' not found from symbol table of assembly '" + ToUtf8(assembly->Name().Value()) + "'");
    }
}

void SymbolTable::AddType(TypeSymbol* type)
{
    if (doNotAddTypes) return;
    if (dynamic_cast<TypeParameterSymbol*>(type)) return;
    std::u32string typeFullName = type->FullName();
    ConstantPool& constantPool = assembly->GetConstantPool();
    ConstantId id = constantPool.GetIdFor(typeFullName);
    if (id == noConstantId)
    {
        id = constantPool.Install(StringPtr(typeFullName.c_str()));
    }
    Constant c = constantPool.GetConstant(id);
    auto it = typeSymbolMap.find(c);
    if (it != typeSymbolMap.cend())
    {
        if (!type->IsReopenedClassTemplateSpecialization())
        {
            throw std::runtime_error("name '" + ToUtf8(typeFullName) + "' already found from symbol table of assembly '" + ToUtf8(assembly->Name().Value()) + "'");
        }
    }
    else
    {
        typeSymbolMap[c] = type;
    }
    if (doNotAddClassTemplateSpecializations) return;
    if (ClassTemplateSpecializationSymbol* classTemplateSpecialization = dynamic_cast<ClassTemplateSpecializationSymbol*>(type))
    {
        classTemplateSpecializationMap[classTemplateSpecialization->Key()] = classTemplateSpecialization;
    }
    else if (RefTypeSymbol* refTypeSymbol = dynamic_cast<RefTypeSymbol*>(type))
    {
        refTypeMap[refTypeSymbol->GetBaseType()] = refTypeSymbol;
    }
}

Symbol* SymbolTable::GetSymbol(Node& node) const
{
    auto it = nodeSymbolMap.find(&node);
    if (it != nodeSymbolMap.cend())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("symbol for node not found");
    }
}

Symbol* SymbolTable::GetSymbol(uint32_t symbolId) const
{
    auto it = idSymbolMap.find(symbolId);
    if (it != idSymbolMap.cend())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("symbol for id " + std::to_string(symbolId) + " not found");
    }
}

Symbol* SymbolTable::GetSymbolNothrow(uint32_t symbolId) const
{
    auto it = idSymbolMap.find(symbolId);
    if (it != idSymbolMap.cend())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

void SymbolTable::AddSymbol(Symbol* symbol)
{
    if (symbol->HasId())
    {
        idSymbolMap[symbol->Id()] = symbol;
    }
}

Node* SymbolTable::GetNode(Symbol* symbol) const
{
    auto it = symbolNodeMap.find(symbol);
    if (it != symbolNodeMap.cend())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("node for symbol not found");
    }
}

Node* SymbolTable::GetNodeNothrow(Symbol* symbol) const
{
    auto it = symbolNodeMap.find(symbol);
    if (it != symbolNodeMap.cend())
    {
        return it->second;
    }
    return nullptr;
}

void SymbolTable::MapNodeSetNoIds(Node& node, Symbol* symbol)
{
    symbolNodeMap[symbol] = &node;
}

void SymbolTable::MapNode(Node& node, Symbol* symbol)
{
    nodeSymbolMap[&node] = symbol;
    symbolNodeMap[symbol] = &node;
    uint32_t symbolId = nextSymbolId++;
    idSymbolMap[symbolId] = symbol;
    symbol->SetId(symbolId);
    node.SetSymbolId(symbolId);
}

void SymbolTable::AddConversion(FunctionSymbol* conversionFun)
{
    conversionTable.AddConversion(conversionFun);
}

TypeSymbol* SymbolTable::CreateArrayType(ArrayNode& arrayNode, TypeSymbol* elementType)
{
    std::u32string arrayName = elementType->FullName() + U"[]";
    TypeSymbol* arrayType = GetTypeNoThrow(arrayName);
    if (arrayType) 
    {
        return arrayType;
    }
    else
    {
        ConstantPool& constantPool = assembly->GetConstantPool();
        Constant arrayNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(arrayName.c_str())));
        ArrayTypeSymbol* arrayTypeSymbol = new ArrayTypeSymbol(arrayNode.GetSpan(), arrayNameConstant);
        arrayTypeSymbol->SetAssembly(assembly);
        arrayTypeSymbol->SetPublic();
        globalNs.AddSymbol(std::unique_ptr<Symbol>(arrayTypeSymbol));
        BeginContainer(arrayTypeSymbol);
        TypeSymbol* systemArrayType = GetType(U"System.Array");
        ClassTypeSymbol* systemArrayClassType = dynamic_cast<ClassTypeSymbol*>(systemArrayType);
        Assert(systemArrayClassType, "class type symbol expected");
        arrayTypeSymbol->SetBaseClass(systemArrayClassType);
        TypeSymbol* enumerableType = GetType(U"System.Enumerable");
        InterfaceTypeSymbol* enumerableInterface = dynamic_cast<InterfaceTypeSymbol*>(enumerableType);
        Assert(enumerableInterface, "interface type expected");
        arrayTypeSymbol->AddImplementedInterface(enumerableInterface);
        arrayTypeSymbol->SetElementType(elementType);
        std::u32string fullElementName = elementType->FullName();
        assembly->GetConstantPool().Install(StringPtr(fullElementName.c_str()));
        std::u32string fullName = arrayTypeSymbol->FullName();
        Constant fullNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullName.c_str())));
        arrayTypeSymbol->GetObjectType()->SetNameConstant(fullNameConstant);
        arrayTypeSymbol->GetContainerScope()->SetBase(arrayTypeSymbol->BaseClass()->GetContainerScope());
        ObjectType* baseClassObjectType = arrayTypeSymbol->BaseClass()->GetObjectType();
        arrayTypeSymbol->GetObjectType()->AddFields(baseClassObjectType->Fields());
        arrayTypeSymbol->GetObjectType()->AddField(ValueType::allocationHandle);
        EndContainer();
        CreateBasicTypeObjectFun(*assembly, arrayTypeSymbol);
        CreateArraySizeConstructor(*assembly, arrayTypeSymbol);
        BeginContainer(arrayTypeSymbol);
        TypeSymbol* enumeratorInterface = GetType(U"System.Enumerator");
        Constant getEnumeratorName = constantPool.GetConstant(constantPool.Install(U"GetEnumerator"));
        ArrayGetEnumeratorMemberFunctionSymbol* getEnumerator = new ArrayGetEnumeratorMemberFunctionSymbol(arrayNode.GetSpan(), getEnumeratorName);
        getEnumerator->SetAssembly(assembly);
        getEnumerator->SetPublic();
        getEnumerator->GetContainerScope()->SetId(getEnumerator->GetNextContainerScopeId());
        Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
        ParameterSymbol* thisParam = new ParameterSymbol(arrayNode.GetSpan(), thisParamName);
        thisParam->SetAssembly(assembly);
        thisParam->SetType(arrayTypeSymbol);
        getEnumerator->AddSymbol(std::unique_ptr<Symbol>(thisParam));
        getEnumerator->SetGroupNameConstant(getEnumeratorName);
        getEnumerator->SetReturnType(enumeratorInterface);
        arrayTypeSymbol->AddSymbol(std::unique_ptr<Symbol>(getEnumerator));
        EndContainer();
        createdArrays.push_back(arrayTypeSymbol);
        return arrayTypeSymbol;
    }
}

std::u32string MakeClassTemplateSpecializationName(ClassTypeSymbol* primaryClassTemplate, const std::vector<TypeSymbol*>& typeArguments)
{
    std::u32string name;
    name.append(primaryClassTemplate->Name().Value()).append(1, U'<');
    int n = int(typeArguments.size());
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            name.append(U", ");
        }
        TypeSymbol* typeArgument = typeArguments[i];
        name.append(typeArgument->FullName());
    }
    name.append(1, U'>');
    return name;
}

ClassTemplateSpecializationSymbol* SymbolTable::MakeClassTemplateSpecialization(ClassTypeSymbol* primaryClassTemplate, const std::vector<TypeSymbol*>& typeArguments, const Span& span)
{
    ClassTemplateSpecializationKey classTemplateSpecializationKey(primaryClassTemplate, typeArguments);
    auto it = classTemplateSpecializationMap.find(classTemplateSpecializationKey);
    if (it != classTemplateSpecializationMap.cend())
    {
        return it->second;
    }
    std::u32string name = MakeClassTemplateSpecializationName(primaryClassTemplate, typeArguments);
    ConstantPool& constantPool = assembly->GetConstantPool();
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    ClassTemplateSpecializationSymbol* classTemplateSpecialization = new ClassTemplateSpecializationSymbol(span, nameConstant);
    classTemplateSpecialization->SetAssembly(assembly);
    Constant sfp = primaryClassTemplate->GetSourceFilePathConstant();
    if (sfp.Value().AsStringLiteral())
    {
        Constant ctsc = constantPool.GetConstant(constantPool.Install(sfp.Value().AsStringLiteral()));
        classTemplateSpecialization->SetSourceFilePathConstant(ctsc);
    }
    classTemplateSpecialization->SetPublic();
    classTemplateSpecialization->SetKey(classTemplateSpecializationKey);
    classTemplateSpecialization->SetProject();
    return classTemplateSpecialization;
}

TypeSymbol* SymbolTable::MakeRefType(Node& node, TypeSymbol* baseType)
{
    if (baseType->IsRefType())
    {
        RefTypeSymbol* refBaseType = static_cast<RefTypeSymbol*>(baseType);
        baseType = refBaseType->GetBaseType();
    }
    auto it = refTypeMap.find(baseType);
    if (it != refTypeMap.cend())
    {
        return it->second;
    }
    ConstantPool& constantPool = assembly->GetConstantPool();
    std::u32string s = U"ref " + baseType->FullName();
    Constant name = constantPool.GetConstant(constantPool.Install(StringPtr(s.c_str())));
    RefTypeSymbol* refTypeSymbol = new RefTypeSymbol(node.GetSpan(), name);
    refTypeSymbol->SetAssembly(assembly);
    refTypeSymbol->SetPublic();
    globalNs.AddSymbol(std::unique_ptr<Symbol>(refTypeSymbol));
    refTypeSymbol->SetBaseType(baseType);
    refTypeMap[baseType] = refTypeSymbol;
    CreateRefTypeBasicFun(*assembly, refTypeSymbol);
    return refTypeSymbol;
}

void SymbolTable::MergeClassTemplateSpecializations()
{
    for (auto& p : classTemplateSpecializationMap)
    {
        p.second->MergeOpenedInstances(assembly);
    }
}

void SymbolTable::Dump(CodeFormatter& codeFormatter)
{
    codeFormatter.WriteLine("SYMBOL TABLE");
    codeFormatter.WriteLine();
    globalNs.Dump(codeFormatter, assembly);
    codeFormatter.WriteLine();
}

SymbolCreator::~SymbolCreator()
{
}

template<typename T>
class ConcreteSymbolCreator : public SymbolCreator
{
public:
    Symbol* CreateSymbol(const Span& span, Constant name) override
    {
        return new T(span, name);
    }
};

std::unique_ptr<SymbolFactory> SymbolFactory::instance;

SymbolFactory::SymbolFactory()
{
    creators.resize(int(SymbolType::maxSymbol));
}

void SymbolFactory::Init()
{
    instance = std::unique_ptr<SymbolFactory>(new SymbolFactory());
}

void SymbolFactory::Done()
{
    instance.reset();
}

SymbolFactory& SymbolFactory::Instance()
{
    Assert(instance, "symbol factory not set");
    return *instance;
}

void SymbolFactory::Register(SymbolType symbolType, SymbolCreator* creator)
{
    creators[int(symbolType)] = std::unique_ptr<SymbolCreator>(creator);
}

Symbol* SymbolFactory::CreateSymbol(SymbolType symbolType, const Span& span, Constant name)
{
    const std::unique_ptr<SymbolCreator>& creator = creators[int(symbolType)];
    if (creator)
    {
        Symbol* value = creator->CreateSymbol(span, name);
        if (value)
        {
            return value;
        }
        else
        {
            throw std::runtime_error("could not create symbol");
        }
    }
    else
    {
        throw std::runtime_error("no creator for symbol type '" + SymbolTypeStr(symbolType) + "'");
    }
}

void InitSymbol()
{
    SymbolFactory::Init();
    SymbolFactory::Instance().Register(SymbolType::boolTypeSymbol, new ConcreteSymbolCreator<BoolTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::charTypeSymbol, new ConcreteSymbolCreator<CharTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::voidTypeSymbol, new ConcreteSymbolCreator<VoidTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::sbyteTypeSymbol, new ConcreteSymbolCreator<SByteTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::byteTypeSymbol, new ConcreteSymbolCreator<ByteTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::shortTypeSymbol, new ConcreteSymbolCreator<ShortTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::ushortTypeSymbol, new ConcreteSymbolCreator<UShortTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::intTypeSymbol, new ConcreteSymbolCreator<IntTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::uintTypeSymbol, new ConcreteSymbolCreator<UIntTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::longTypeSymbol, new ConcreteSymbolCreator<LongTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::ulongTypeSymbol, new ConcreteSymbolCreator<ULongTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::floatTypeSymbol, new ConcreteSymbolCreator<FloatTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::doubleTypeSymbol, new ConcreteSymbolCreator<DoubleTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::nullReferenceTypeSymbol, new ConcreteSymbolCreator<NullReferenceTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::classTypeSymbol, new ConcreteSymbolCreator<ClassTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::interfaceTypeSymbol, new ConcreteSymbolCreator<InterfaceTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::arrayTypeSymbol, new ConcreteSymbolCreator<ArrayTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::classTemplateSpecializationSymbol, new ConcreteSymbolCreator<ClassTemplateSpecializationSymbol>());
    SymbolFactory::Instance().Register(SymbolType::functionSymbol, new ConcreteSymbolCreator<FunctionSymbol>());
    SymbolFactory::Instance().Register(SymbolType::memberFunctionSymbol, new ConcreteSymbolCreator<MemberFunctionSymbol>());
    SymbolFactory::Instance().Register(SymbolType::staticConstructorSymbol, new ConcreteSymbolCreator<StaticConstructorSymbol>());
    SymbolFactory::Instance().Register(SymbolType::constructorSymbol, new ConcreteSymbolCreator<ConstructorSymbol>());
    SymbolFactory::Instance().Register(SymbolType::arraySizeConstructorSymbol, new ConcreteSymbolCreator<ArraySizeConstructorSymbol>());
    SymbolFactory::Instance().Register(SymbolType::declarationBlock, new ConcreteSymbolCreator<DeclarationBlock>());
    SymbolFactory::Instance().Register(SymbolType::typeParameterSymbol, new ConcreteSymbolCreator<TypeParameterSymbol>());
    SymbolFactory::Instance().Register(SymbolType::boundTypeParameterSymbol, new ConcreteSymbolCreator<BoundTypeParameterSymbol>());
    SymbolFactory::Instance().Register(SymbolType::parameterSymbol, new ConcreteSymbolCreator<ParameterSymbol>());
    SymbolFactory::Instance().Register(SymbolType::localVariableSymbol, new ConcreteSymbolCreator<LocalVariableSymbol>());
    SymbolFactory::Instance().Register(SymbolType::memberVariableSymbol, new ConcreteSymbolCreator<MemberVariableSymbol>());
    SymbolFactory::Instance().Register(SymbolType::propertySymbol, new ConcreteSymbolCreator<PropertySymbol>());
    SymbolFactory::Instance().Register(SymbolType::propertyGetterSymbol, new ConcreteSymbolCreator<PropertyGetterFunctionSymbol>());
    SymbolFactory::Instance().Register(SymbolType::propertySetterSymbol, new ConcreteSymbolCreator<PropertySetterFunctionSymbol>());
    SymbolFactory::Instance().Register(SymbolType::indexerSymbol, new ConcreteSymbolCreator<IndexerSymbol>());
    SymbolFactory::Instance().Register(SymbolType::indexerGetterSymbol, new ConcreteSymbolCreator<IndexerGetterFunctionSymbol>());
    SymbolFactory::Instance().Register(SymbolType::indexerSetterSymbol, new ConcreteSymbolCreator<IndexerSetterFunctionSymbol>());
    SymbolFactory::Instance().Register(SymbolType::constantSymbol, new ConcreteSymbolCreator<ConstantSymbol>());
    SymbolFactory::Instance().Register(SymbolType::namespaceSymbol, new ConcreteSymbolCreator<NamespaceSymbol>());
    SymbolFactory::Instance().Register(SymbolType::basicTypeDefaultInit, new ConcreteSymbolCreator<BasicTypeDefaultInit>());
    SymbolFactory::Instance().Register(SymbolType::basicTypeCopyInit, new ConcreteSymbolCreator<BasicTypeCopyInit>());
    SymbolFactory::Instance().Register(SymbolType::basicTypeAssignment, new ConcreteSymbolCreator<BasicTypeAssignment>());
    SymbolFactory::Instance().Register(SymbolType::basicTypeReturn, new ConcreteSymbolCreator<BasicTypeReturn>());
    SymbolFactory::Instance().Register(SymbolType::basicTypeConversion, new ConcreteSymbolCreator<BasicTypeConversion>());
    SymbolFactory::Instance().Register(SymbolType::basicTypeUnaryOp, new ConcreteSymbolCreator<BasicTypeUnaryOpFun>());
    SymbolFactory::Instance().Register(SymbolType::basicTypBinaryOp, new ConcreteSymbolCreator<BasicTypeBinaryOpFun>());
    SymbolFactory::Instance().Register(SymbolType::objectDefaultInit, new ConcreteSymbolCreator<ObjectDefaultInit>());
    SymbolFactory::Instance().Register(SymbolType::objectNullEqual, new ConcreteSymbolCreator<ObjectNullEqual>());
    SymbolFactory::Instance().Register(SymbolType::nullObjectEqual, new ConcreteSymbolCreator<NullObjectEqual>());
    SymbolFactory::Instance().Register(SymbolType::objectCopyInit, new ConcreteSymbolCreator<ObjectCopyInit>());
    SymbolFactory::Instance().Register(SymbolType::objectNullInit, new ConcreteSymbolCreator<ObjectNullInit>());
    SymbolFactory::Instance().Register(SymbolType::objectAssignment, new ConcreteSymbolCreator<ObjectAssignment>());
    SymbolFactory::Instance().Register(SymbolType::objectNullAssignment, new ConcreteSymbolCreator<ObjectNullAssignment>());
    SymbolFactory::Instance().Register(SymbolType::enumTypeSymbol, new ConcreteSymbolCreator<EnumTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::enumConstantSymbol, new ConcreteSymbolCreator<EnumConstantSymbol>());
    SymbolFactory::Instance().Register(SymbolType::enumTypeDefaultInit, new ConcreteSymbolCreator<EnumTypeDefaultInit>());
    SymbolFactory::Instance().Register(SymbolType::enumTypeConversion, new ConcreteSymbolCreator<EnumTypeConversionFun>());
    SymbolFactory::Instance().Register(SymbolType::delegateTypeSymbol, new ConcreteSymbolCreator<DelegateTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::classDelegateTypeSymbol, new ConcreteSymbolCreator<ClassDelegateTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::delegateDefaultInit, new ConcreteSymbolCreator<DelegateDefaultInit>());
    SymbolFactory::Instance().Register(SymbolType::refTypeAssignment, new ConcreteSymbolCreator<RefTypeAssignment>());
    SymbolFactory::Instance().Register(SymbolType::refTypeSymbol, new ConcreteSymbolCreator<RefTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::refTypeInit, new ConcreteSymbolCreator<RefTypeInit>());
    SymbolFactory::Instance().Register(SymbolType::delegateNullEqual, new ConcreteSymbolCreator<DelegateNullEqual>());
    SymbolFactory::Instance().Register(SymbolType::nullDelegateEqual, new ConcreteSymbolCreator<NullDelegateEqual>());
    SymbolFactory::Instance().Register(SymbolType::requestGcFunctionSymbol, new ConcreteSymbolCreator<RequestGcFunctionSymbol>());
}

void DoneSymbol()
{
    SymbolFactory::Done();
}

std::unique_ptr<Assembly> CreateSystemCoreAssembly(Machine& machine, const std::string& config)
{
    std::unique_ptr<Assembly> systemCoreAssembly(new Assembly(machine, U"System.Core", CminorSystemCoreAssemblyFilePath(config)));
    systemCoreAssembly->SetCore();
    systemCoreAssembly->GetSymbolTable().BeginNamespace(StringPtr(U"System"), Span());
    BasicTypeSymbol* boolTypeSymbol = new BoolTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"Boolean")));
    boolTypeSymbol->SetAssembly(systemCoreAssembly.get());
    boolTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.Boolean")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(boolTypeSymbol));
    BasicTypeSymbol* charTypeSymbol = new CharTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"Char")));
    charTypeSymbol->SetAssembly(systemCoreAssembly.get());
    charTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.Char")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(charTypeSymbol));
    BasicTypeSymbol* voidTypeSymbol = new VoidTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"Void")));
    voidTypeSymbol->SetAssembly(systemCoreAssembly.get());
    voidTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.Void")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(voidTypeSymbol));
    BasicTypeSymbol* sbyteTypeSymbol = new SByteTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"Int8")));
    sbyteTypeSymbol->SetAssembly(systemCoreAssembly.get());
    sbyteTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.Int8")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(sbyteTypeSymbol));
    BasicTypeSymbol* byteTypeSymbol = new ByteTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"UInt8")));
    byteTypeSymbol->SetAssembly(systemCoreAssembly.get());
    byteTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.UInt8")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(byteTypeSymbol));
    BasicTypeSymbol* shortTypeSymbol = new ShortTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"Int16")));
    shortTypeSymbol->SetAssembly(systemCoreAssembly.get());
    shortTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.Int16")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(shortTypeSymbol));
    BasicTypeSymbol* ushortTypeSymbol = new UShortTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"UInt16")));
    ushortTypeSymbol->SetAssembly(systemCoreAssembly.get());
    ushortTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.UInt16")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(ushortTypeSymbol));
    BasicTypeSymbol* intTypeSymbol = new IntTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"Int32")));
    intTypeSymbol->SetAssembly(systemCoreAssembly.get());
    intTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.Int32")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(intTypeSymbol));
    BasicTypeSymbol* uintTypeSymbol = new UIntTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"UInt32")));
    uintTypeSymbol->SetAssembly(systemCoreAssembly.get());
    uintTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.UInt32")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(uintTypeSymbol));
    BasicTypeSymbol* longTypeSymbol = new LongTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"Int64")));
    longTypeSymbol->SetAssembly(systemCoreAssembly.get());
    longTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.Int64")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(longTypeSymbol));
    BasicTypeSymbol* ulongTypeSymbol = new ULongTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"UInt64")));
    ulongTypeSymbol->SetAssembly(systemCoreAssembly.get());
    ulongTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.UInt64")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(ulongTypeSymbol));
    BasicTypeSymbol* floatTypeSymbol = new FloatTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"Float")));
    floatTypeSymbol->SetAssembly(systemCoreAssembly.get());
    floatTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.Float")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(floatTypeSymbol));
    BasicTypeSymbol* doubleTypeSymbol = new DoubleTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"Double")));
    doubleTypeSymbol->SetAssembly(systemCoreAssembly.get());
    doubleTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.Double")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(doubleTypeSymbol));
    BasicTypeSymbol* nullReferenceTypeSymbol = new NullReferenceTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"@nullref")));
    nullReferenceTypeSymbol->SetAssembly(systemCoreAssembly.get());
    nullReferenceTypeSymbol->GetMachineType()->SetNameConstant(systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"System.@nullref")));
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<TypeSymbol>(nullReferenceTypeSymbol));
    Constant requestGcFunctionName = systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"RequestGc()"));
    FunctionSymbol* requestGcFunctionSymbol = new RequestGcFunctionSymbol(Span(), requestGcFunctionName);
    requestGcFunctionSymbol->SetPublic();
    requestGcFunctionSymbol->SetReturnType(voidTypeSymbol);
    requestGcFunctionSymbol->SetAssembly(systemCoreAssembly.get());
    Constant requestGcGroupName = systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"RequestGc"));
    requestGcFunctionSymbol->SetGroupNameConstant(requestGcGroupName);
    systemCoreAssembly->GetSymbolTable().Container()->AddSymbol(std::unique_ptr<FunctionSymbol>(requestGcFunctionSymbol));
    systemCoreAssembly->GetSymbolTable().EndNamespace();
    InitBasicTypeFun(*systemCoreAssembly);
    return systemCoreAssembly;
}

} } // namespace cminor::symbols
