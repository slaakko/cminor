// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/SymbolTable.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/ConstantSymbol.hpp>
#include <cminor/symbols/BasicTypeFun.hpp>
#include <cminor/symbols/ObjectFun.hpp>
#include <cminor/symbols/StringFun.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/machine/Type.hpp>

namespace cminor { namespace symbols {

SymbolTable::SymbolTable(Assembly* assembly_) : assembly(assembly_), globalNs(Span(), assembly->GetConstantPool().GetEmptyStringConstant()), container(&globalNs), function(nullptr),
mainFunction(nullptr), currentClass(nullptr), declarationBlockId(0), doNotAddTypes(false), conversionTable(*assembly)
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
    utf32_string nsName = ToUtf32(namespaceNode.Id()->Str());
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
    utf32_string name = ToUtf32(functionNode.Name());
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    utf32_string groupName = ToUtf32(functionNode.GroupId()->Str());
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
        }
    }
    MapNode(functionNode, function);
    ContainerScope* functionScope = function->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    functionScope->SetParent(containerScope);
    BeginContainer(function);
    declarationBlockId = 0;
}

void SymbolTable::EndFunction()
{
    EndContainer();
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
    ContainerScope* containerScope = container->GetContainerScope();
    functionScope->SetParent(containerScope);
    BeginContainer(function);
    declarationBlockId = 0;
}

void SymbolTable::EndStaticConstructor()
{
    EndContainer();
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
    container->AddSymbol(std::unique_ptr<Symbol>(function));
    function = nullptr;
}

void SymbolTable::BeginMemberFunction(MemberFunctionNode& memberFunctionNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    utf32_string name = ToUtf32(memberFunctionNode.Name());
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    function = new MemberFunctionSymbol(memberFunctionNode.GetSpan(), nameConstant);
    function->SetAssembly(assembly);
    utf32_string groupName = ToUtf32(memberFunctionNode.GroupId()->Str());
    Constant groupNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(groupName.c_str())));
    function->SetGroupNameConstant(groupNameConstant);
    MapNode(memberFunctionNode, function);
    ContainerScope* functionScope = function->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    functionScope->SetParent(containerScope);
    BeginContainer(function);
    declarationBlockId = 0;
    Constant thisParamName = constantPool.GetConstant(constantPool.Install(U"this"));
    ParameterSymbol* thisParam = new ParameterSymbol(memberFunctionNode.GetSpan(), thisParamName);
    thisParam->SetAssembly(assembly);
    TypeSymbol* thisParamType = currentClass;
    thisParam->SetType(thisParamType);
    thisParam->SetBound();
    function->AddSymbol(std::unique_ptr<Symbol>(thisParam));
}

void SymbolTable::EndMemberFunction()
{
    EndContainer();
    container->AddSymbol(std::unique_ptr<Symbol>(function));
    function = nullptr;
}

void SymbolTable::BeginClass(ClassNode& classNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    utf32_string name = ToUtf32(classNode.Id()->Str());
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

void SymbolTable::AddParameter(ParameterNode& parameterNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    utf32_string name = ToUtf32(parameterNode.Id()->Str());
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    ParameterSymbol* parameter = new ParameterSymbol(parameterNode.GetSpan(), nameConstant);
    parameter->SetAssembly(assembly);
    container->AddSymbol(std::unique_ptr<Symbol>(parameter));
    MapNode(parameterNode, parameter);
}

void SymbolTable::BeginDeclarationBlock(StatementNode& statementNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    utf32_string name = U"@locals" + ToUtf32(std::to_string(declarationBlockId++));
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    DeclarationBlock* declarationBlock = new DeclarationBlock(statementNode.GetSpan(), nameConstant);
    declarationBlock->SetAssembly(assembly);
    ContainerScope* declarationBlockScope = declarationBlock->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    declarationBlockScope->SetParent(containerScope);
    container->AddSymbol(std::unique_ptr<Symbol>(declarationBlock));
    MapNode(statementNode, declarationBlock);
    BeginContainer(declarationBlock);
}

void SymbolTable::EndDeclarationBlock()
{
    EndContainer();
}

void SymbolTable::AddLocalVariable(ConstructionStatementNode& constructionStatementNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    utf32_string name = ToUtf32(constructionStatementNode.Id()->Str());
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(name.c_str())));
    LocalVariableSymbol* localVariableSymbol = new LocalVariableSymbol(constructionStatementNode.GetSpan(), nameConstant);
    localVariableSymbol->SetAssembly(assembly);
    container->AddSymbol(std::unique_ptr<Symbol>(localVariableSymbol));
    MapNode(constructionStatementNode, localVariableSymbol);
}

void SymbolTable::AddMemberVariable(MemberVariableNode& memberVariableNode)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    utf32_string name = ToUtf32(memberVariableNode.Id()->Str());
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

void SymbolTable::Write(SymbolWriter& writer)
{
    globalNs.Write(writer);
    bool hasMainFunction = mainFunction != nullptr;
    writer.AsMachineWriter().Put(hasMainFunction);
    if (hasMainFunction)
    {
        utf32_string mainFunctionFullName = mainFunction->FullName();
        ConstantId mainFunctionNameId = assembly->GetConstantPool().GetIdFor(mainFunctionFullName);
        Assert(mainFunctionNameId != noConstantId, "got no constant id");
        mainFunctionNameId.Write(writer);
    }
}

void SymbolTable::Read(SymbolReader& reader)
{
    NamespaceSymbol ns(Span(), assembly->GetConstantPool().GetEmptyStringConstant());
    ns.SetAssembly(assembly);
    ns.Read(reader);
    bool prevDoNotAddTypes = doNotAddTypes;
    doNotAddTypes = true;
    globalNs.Import(&ns, *this);
    doNotAddTypes = prevDoNotAddTypes;
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

TypeSymbol* SymbolTable::GetTypeNoThrow(const utf32_string& typeFullName) const
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

TypeSymbol* SymbolTable::GetType(const utf32_string& typeFullName) const
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
    utf32_string typeFullName = type->FullName();
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
        throw std::runtime_error("name '" + ToUtf8(typeFullName) + "' already found from symbol table of assembly '" + ToUtf8(assembly->Name().Value()) + "'");
    }
    else
    {
        typeSymbolMap[c] = type;
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

void SymbolTable::MapNode(Node& node, Symbol* symbol)
{
    nodeSymbolMap[&node] = symbol;
    symbolNodeMap[symbol] = &node;
}

void SymbolTable::AddConversion(FunctionSymbol* conversionFun)
{
    conversionTable.AddConversion(conversionFun);
}

SymbolCreator::~SymbolCreator()
{
}

template<typename T>
class ConcreteSymbolCreator : public SymbolCreator
{
public:
    virtual Symbol* CreateSymbol(const Span& span, Constant name)
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
    SymbolFactory::Instance().Register(SymbolType::objectTypeSymbol, new ConcreteSymbolCreator<ObjectTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::stringTypeSymbol, new ConcreteSymbolCreator<StringTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::functionSymbol, new ConcreteSymbolCreator<FunctionSymbol>());
    SymbolFactory::Instance().Register(SymbolType::memberFunctionSymbol, new ConcreteSymbolCreator<MemberFunctionSymbol>());
    SymbolFactory::Instance().Register(SymbolType::staticConstructorSymbol, new ConcreteSymbolCreator<StaticConstructorSymbol>());
    SymbolFactory::Instance().Register(SymbolType::constructorSymbol, new ConcreteSymbolCreator<ConstructorSymbol>());
    SymbolFactory::Instance().Register(SymbolType::declarationBlock, new ConcreteSymbolCreator<DeclarationBlock>());
    SymbolFactory::Instance().Register(SymbolType::parameterSymbol, new ConcreteSymbolCreator<ParameterSymbol>());
    SymbolFactory::Instance().Register(SymbolType::localVariableSymbol, new ConcreteSymbolCreator<LocalVariableSymbol>());
    SymbolFactory::Instance().Register(SymbolType::memberVariableSymbol, new ConcreteSymbolCreator<MemberVariableSymbol>());
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
    SymbolFactory::Instance().Register(SymbolType::objectCopyInit, new ConcreteSymbolCreator<ObjectCopyInit>());
    SymbolFactory::Instance().Register(SymbolType::objectNullInit, new ConcreteSymbolCreator<ObjectNullInit>());
    SymbolFactory::Instance().Register(SymbolType::objectAssignment, new ConcreteSymbolCreator<ObjectAssignment>());
}

void DoneSymbol()
{
    SymbolFactory::Done();
}

std::unique_ptr<Assembly> CreateSystemCoreAssembly(Machine& machine, const std::string& config)
{
    std::unique_ptr<Assembly> systemCoreAssembly(new Assembly(machine, U"System.Core", CminorSystemCoreAssemblyFilePath(config)));
    TypeSymbol* boolTypeSymbol = new BoolTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"bool")));
    boolTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(boolTypeSymbol));
    TypeSymbol* charTypeSymbol = new CharTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"char")));
    charTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(charTypeSymbol));
    TypeSymbol* voidTypeSymbol = new VoidTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"void")));
    voidTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(voidTypeSymbol));
    TypeSymbol* sbyteTypeSymbol = new SByteTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"sbyte")));
    sbyteTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(sbyteTypeSymbol));
    TypeSymbol* byteTypeSymbol = new ByteTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"byte")));
    byteTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(byteTypeSymbol));
    TypeSymbol* shortTypeSymbol = new ShortTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"short")));
    shortTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(shortTypeSymbol));
    TypeSymbol* ushortTypeSymbol = new UShortTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"ushort")));
    ushortTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(ushortTypeSymbol));
    TypeSymbol* intTypeSymbol = new IntTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"int")));
    intTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(intTypeSymbol));
    TypeSymbol* uintTypeSymbol = new UIntTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"uint")));
    uintTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(uintTypeSymbol));
    TypeSymbol* longTypeSymbol = new LongTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"long")));
    longTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(longTypeSymbol));
    TypeSymbol* ulongTypeSymbol = new ULongTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"ulong")));
    ulongTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(ulongTypeSymbol));
    TypeSymbol* floatTypeSymbol = new FloatTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"float")));
    floatTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(floatTypeSymbol));
    TypeSymbol* doubleTypeSymbol = new DoubleTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"double")));
    doubleTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(doubleTypeSymbol));
    TypeSymbol* nullReferenceTypeSymbol = new NullReferenceTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"@nullref")));
    nullReferenceTypeSymbol->SetAssembly(systemCoreAssembly.get());
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(nullReferenceTypeSymbol));
/*
    ObjectTypeSymbol* objectTypeSymbol = new ObjectTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"object")));
    objectTypeSymbol->SetAssembly(systemCoreAssembly.get());
    objectTypeSymbol->SetPublic();
    objectTypeSymbol->GetObjectType()->SetNameConstant(objectTypeSymbol->NameConstant());
    objectTypeSymbol->GetObjectType()->AddField(ValueType::classDataPtr);
    objectTypeSymbol->InitVmt();
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(objectTypeSymbol));
    StringTypeSymbol* stringTypeSymbol = new StringTypeSymbol(Span(), systemCoreAssembly->GetConstantPool().GetConstant(systemCoreAssembly->GetConstantPool().Install(U"string")));
    stringTypeSymbol->SetAssembly(systemCoreAssembly.get());
    stringTypeSymbol->SetPublic();
    stringTypeSymbol->SetBaseClass(objectTypeSymbol);
    stringTypeSymbol->GetContainerScope()->SetBase(stringTypeSymbol->BaseClass()->GetContainerScope());
    stringTypeSymbol->GetObjectType()->SetNameConstant(stringTypeSymbol->NameConstant());
    stringTypeSymbol->GetObjectType()->AddFields(objectTypeSymbol->GetObjectType()->Fields());
    stringTypeSymbol->InitVmt();
    systemCoreAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(stringTypeSymbol));
*/
    InitBasicTypeFun(*systemCoreAssembly);
    //InitObjectFun(*systemCoreAssembly);
    return systemCoreAssembly;
}

} } // namespace cminor::symbols
