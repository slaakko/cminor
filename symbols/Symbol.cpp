// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/ConstantSymbol.hpp>
#include <cminor/symbols/BasicTypeFun.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/machine/Util.hpp>
#include <boost/filesystem.hpp>

namespace cminor { namespace symbols {

const char* symbolTypeStr[uint8_t(SymbolType::maxSymbol)] =
{
    "boolSymbol", "charSymbol", "voidSymbol", "sbyteSymbol", "byteSymbol", "shortSymbol", "ushortSymbol", "intSymbol", "uintSymbol", "longSymbol", "ulongSymbol", "floatSymbol", "doubleSymbol",
    "classSymbol", "functionSymbol", "functionGroupSymbol", "parameterSymbol", "localVariableSymbol", "memberVariableSymbol", "namespaceSymbol"
};

std::string SymbolTypeStr(SymbolType symbolType)
{
    return symbolTypeStr[uint8_t(symbolType)];
}

Symbol::Symbol(const Span& span_, Constant name_) : span(span_), name(name_), flags(SymbolFlags::none), parent(nullptr), assembly(nullptr)
{
    Assert(name.Value().GetType() == ValueType::stringLiteral, "string literal expected");
    SetSource(SymbolSource::project);
}

Symbol::~Symbol()
{
}

bool Symbol::IsExportSymbol() const
{
    return Source() == SymbolSource::project;
}

void Symbol::Write(SymbolWriter& writer)
{
    static_cast<Writer&>(writer).Put(uint8_t(flags & ~SymbolFlags::project));
}

void Symbol::Read(SymbolReader& reader)
{
    flags = static_cast<SymbolFlags>(reader.GetByte());
}

StringPtr Symbol::Name() const
{
    StringPtr s(name.Value().AsStringLiteral());
    return s;
}

void Symbol::SetName(StringPtr newName)
{
    ConstantPool& constantPool = assembly->GetConstantPool();
    name = constantPool.GetConstant(constantPool.Install(newName));
}

utf32_string Symbol::FullName() const
{
    utf32_string fullName;
    if (parent)
    {
        fullName.append(parent->FullName());
    }
    if (!fullName.empty())
    {
        fullName.append(1, '.');
    }
    fullName.append(Name().Value());
    return fullName;
}

NamespaceSymbol* Symbol::Ns() const
{
    if (const NamespaceSymbol* ns = dynamic_cast<const NamespaceSymbol*>(this))
    {
        return const_cast<NamespaceSymbol*>(ns);
    }
    else
    {
        if (parent)
        {
            return parent->Ns();
        }
        else
        {
            throw std::runtime_error("namespace not found");
        }
    }
}

Scope::~Scope()
{
}

ContainerScope::ContainerScope() : base(nullptr), parent(nullptr), container(nullptr)
{
}

void ContainerScope::Install(Symbol* symbol)
{
    auto it = symbolMap.find(symbol->Name());
    if (it != symbolMap.cend())
    {
        Symbol* prev = it->second;
        if (prev != symbol)
        {
            const Span& defined = symbol->GetSpan();
            const Span& referenced = prev->GetSpan();
            throw Exception("symbol '" + ToUtf8(symbol->Name().Value()) + "' already defined", defined, referenced);
        }
    }
    else
    {
        symbolMap[symbol->Name()] = symbol;
    }
}

Symbol* ContainerScope::LookupQualified(const std::vector<utf32_string>& components, ScopeLookup lookup) const
{
    const ContainerScope* scope = this;
    Symbol* s = nullptr;
    int n = int(components.size());
    bool allComponentsMatched = true;
    for (int i = 0; i < n; ++i)
    {
        const utf32_string& component = components[i];
        if (scope)
        {
            s = scope->Lookup(StringPtr(component.c_str()), ScopeLookup::this_);
            if (s)
            {
                scope = s->GetContainerScope();
            }
            else
            {
                allComponentsMatched = false;
            }
        }
    }
    if (!s || !allComponentsMatched)
    {
        if ((lookup & ScopeLookup::parent) != ScopeLookup::none)
        {
            if (parent)
            {
                return parent->LookupQualified(components, lookup);
            }
            else
            {
                return nullptr;
            }
        }
    }
    return s;
}

void ContainerScope::CollectViableFunctions(int arity, StringPtr groupName, std::unordered_set<ContainerScope*>& scopesLookedUp, ScopeLookup scopeLookup, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    if ((scopeLookup & ScopeLookup::this_) != ScopeLookup::none)
    {
        if (scopesLookedUp.find(this) == scopesLookedUp.cend())
        {
            scopesLookedUp.insert(this);
            Symbol* symbol = Lookup(groupName);
            if (symbol)
            {
                if (FunctionGroupSymbol* functionGroupSymbol = dynamic_cast<FunctionGroupSymbol*>(symbol))
                {
                    functionGroupSymbol->CollectViableFunctions(arity, viableFunctions);
                }
            }
        }
    }
    if ((scopeLookup & ScopeLookup::base) != ScopeLookup::none)
    {
        if (base)
        {
            base->CollectViableFunctions(arity, groupName, scopesLookedUp, scopeLookup, viableFunctions);
        }
    }
    if ((scopeLookup & ScopeLookup::parent) != ScopeLookup::none)
    {
        if (parent)
        {
            parent->CollectViableFunctions(arity, groupName, scopesLookedUp, scopeLookup, viableFunctions);
        }
    }
}

NamespaceSymbol* ContainerScope::Ns() const
{
    return container->Ns();
}

NamespaceSymbol* ContainerScope::CreateNamespace(StringPtr qualifiedNsName, const Span& span)
{
    ContainerScope* scope = this;
    NamespaceSymbol* parentNs = scope->Ns();
    std::vector<utf32_string> components = Split(utf32_string(qualifiedNsName.Value()), '.');
    for (const utf32_string& component : components)
    {
        Symbol* s = scope->Lookup(StringPtr(component.c_str()));
        if (s)
        {
            if (NamespaceSymbol* ns = dynamic_cast<NamespaceSymbol*>(s))
            {
                scope = s->GetContainerScope();
                parentNs = scope->Ns();
            }
            else
            {
                throw Exception("symbol '" + ToUtf8(s->Name().Value()) + "' does not denote a namespace", s->GetSpan());
            }
        }
        else
        {
            Assembly* assembly = container->GetAssembly();
            ConstantPool& constantPool = assembly->GetConstantPool();
            NamespaceSymbol* newNs = new NamespaceSymbol(span, constantPool.GetConstant(constantPool.Install(StringPtr(component.c_str()))));
            newNs->SetAssembly(assembly);
            scope = newNs->GetContainerScope();
            scope->SetParent(parentNs->GetContainerScope());
            parentNs->AddSymbol(std::unique_ptr<Symbol>(newNs));
            parentNs = newNs;
        }
    }
    return parentNs;
}

Symbol* ContainerScope::Lookup(StringPtr name) const
{
    return Lookup(name, ScopeLookup::this_);
}

int CountQualifiedNameComponents(StringPtr qualifiedName)
{
    int numComponents = 0;
    int componentSize = 0;
    int state = 0;
    int angleBracketCount = 0;
    const char32_t* p = qualifiedName.Value();
    while (*p)
    {
        char32_t c = *p;
        switch (state)
        {
        case 0:
        {
            if (c == '.')
            {
                ++numComponents;
                componentSize = 0;
            }
            else if (c == '<')
            {
                ++componentSize;
                angleBracketCount = 1;
                state = 1;
            }
            else
            {
                ++componentSize;
            }
            break;
        }
        case 1:
        {
            ++componentSize;
            if (c == '<')
            {
                ++angleBracketCount;
            }
            else if (c == '>')
            {
                --angleBracketCount;
                if (angleBracketCount == 0)
                {
                    state = 0;
                }
            }
            break;
        }
        }
        ++p;
    }
    if (componentSize > 0)
    {
        ++numComponents;
    }
    return numComponents;
}

std::vector<utf32_string> ParseQualifiedName(StringPtr qualifiedName)
{
    std::vector<utf32_string> components;
    int state = 0;
    utf32_string component;
    int angleBracketCount = 0;
    const char32_t* p = qualifiedName.Value();
    while (*p)
    {
        char32_t c = *p;
        switch (state)
        {
        case 0:
        {
            if (c == '.')
            {
                components.push_back(component);
                component.clear();
            }
            else if (c == '<')
            {
                component.append(1, c);
                angleBracketCount = 1;
                state = 1;
            }
            else
            {
                component.append(1, c);
            }
            break;
        }
        case 1:
        {
            component.append(1, c);
            if (c == '<')
            {
                ++angleBracketCount;
            }
            else if (c == '>')
            {
                --angleBracketCount;
                if (angleBracketCount == 0)
                {
                    state = 0;
                }
            }
            break;
        }
        }
        ++p;
    }
    if (!component.empty())
    {
        components.push_back(component);
    }
    return components;
}

Symbol* ContainerScope::Lookup(StringPtr name, ScopeLookup lookup) const
{
    int numQualifiedNameComponents = CountQualifiedNameComponents(name);
    if (numQualifiedNameComponents > 1)
    {
        std::vector<utf32_string> components = ParseQualifiedName(name);
        return LookupQualified(components, lookup);
    }
    else
    {
        auto it = symbolMap.find(name);
        if (it != symbolMap.end())
        {
            Symbol* s = it->second;
            return s;
        }
        if ((lookup & ScopeLookup::base) != ScopeLookup::none)
        {
            if (base)
            {
                Symbol* s = base->Lookup(name, lookup);
                if (s)
                {
                    return s;
                }
            }
        }
        if ((lookup & ScopeLookup::parent) != ScopeLookup::none)
        {
            if (parent)
            {
                Symbol* s = parent->Lookup(name, lookup);
                if (s)
                {
                    return s;
                }
            }
        }
        return nullptr;
    }
}

void ContainerScope::Clear()
{
    symbolMap.clear();
}

void FileScope::InstallAlias(ContainerScope* containerScope, AliasNode* aliasNode)
{
    Assert(containerScope, "container scope is null");
    utf32_string qualifiedName = ToUtf32(aliasNode->Qid()->Str());
    Symbol* symbol = containerScope->Lookup(StringPtr(qualifiedName.c_str()), ScopeLookup::this_and_parent);
    if (symbol)
    {
        utf32_string aliasName = ToUtf32(aliasNode->Id()->Str());
        ConstantPool& constantPool = containerScope->Container()->GetAssembly()->GetConstantPool();
        Constant constant = constantPool.GetConstant(constantPool.Install(StringPtr(aliasName.c_str())));
        aliasSymbolMap[StringPtr(constant.Value().AsStringLiteral())] = symbol;
    }
    else
    {
        throw Exception("referred symbol '" + aliasNode->Qid()->Str() + "' not found", aliasNode->Qid()->GetSpan());
    }
}

void FileScope::InstallNamespaceImport(ContainerScope* containerScope, NamespaceImportNode* namespaceImportNode)
{
    Assert(containerScope, "container scope is null");
    utf32_string importedNamespaceName = ToUtf32(namespaceImportNode->Ns()->Str());
    Symbol* symbol = containerScope->Lookup(StringPtr(importedNamespaceName.c_str()), ScopeLookup::this_and_parent);
    if (symbol)
    {
        if (dynamic_cast<NamespaceSymbol*>(symbol))
        {
            ContainerScope* symbolContainerScope = symbol->GetContainerScope();
            if (std::find(containerScopes.cbegin(), containerScopes.cend(), symbolContainerScope) == containerScopes.cend())
            {
                containerScopes.push_back(symbolContainerScope);
            }
        }
        else
        {
            throw Exception("'" + namespaceImportNode->Ns()->Str() + "' does not denote a namespace", namespaceImportNode->Ns()->GetSpan());
        }
    }
    else
    {
        throw Exception("referred namespace symbol '" + namespaceImportNode->Ns()->Str() + "' not found", namespaceImportNode->Ns()->GetSpan());
    }
}

Symbol* FileScope::Lookup(StringPtr name) const
{
    return Lookup(name, ScopeLookup::this_);
}

Symbol* FileScope::Lookup(StringPtr name, ScopeLookup lookup) const
{
    if (lookup != ScopeLookup::this_)
    {
        throw std::runtime_error("file scope supports only this scope lookup");
    }
    std::unordered_set<Symbol*> foundSymbols;
    auto it = aliasSymbolMap.find(name);
    if (it != aliasSymbolMap.cend())
    {
        Symbol* symbol = it->second;
        foundSymbols.insert(symbol);
    }
    else
    {
        for (ContainerScope* containerScope : containerScopes)
        {
            Symbol* symbol = containerScope->Lookup(name, ScopeLookup::this_);
            if (symbol)
            {
                foundSymbols.insert(symbol);
            }
        }
    }
    if (foundSymbols.empty())
    {
        return nullptr;
    }
    else if (foundSymbols.size() > 1)
    {
        std::string message("reference to object '" + ToUtf8(name.Value()) + "' is ambiguous: ");
        bool first = true;
        Span span;
        for (Symbol* symbol : foundSymbols)
        {
            if (first)
            {
                first = false;
                span = symbol->GetSpan();
            }
            else
            {
                message.append(" or ");
            }
            message.append(ToUtf8(symbol->FullName()));
        }
        throw Exception(message, span);
    }
    else
    {
        return *foundSymbols.begin();
    }
}

void FileScope::CollectViableFunctions(int arity, StringPtr groupName, std::unordered_set<ContainerScope*>& scopesLookedUp, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    for (ContainerScope* containerScope : containerScopes)
    {
        if (scopesLookedUp.find(containerScope) == scopesLookedUp.cend())
        {
            scopesLookedUp.insert(containerScope);
            containerScope->CollectViableFunctions(arity, groupName, scopesLookedUp, ScopeLookup::this_, viableFunctions);
        }
    }
}

ContainerSymbol::ContainerSymbol(const Span& span_, Constant name_) : Symbol(span_, name_)
{
    containerScope.SetContainer(this);
}

void ContainerSymbol::Write(SymbolWriter& writer)
{
    Symbol::Write(writer);
    std::vector<Symbol*> exportSymbols;
    for (const std::unique_ptr<Symbol>& symbol : symbols)
    {
        if (symbol->IsExportSymbol())
        {
            exportSymbols.push_back(symbol.get());
        }
    }
    int32_t n = int32_t(exportSymbols.size());
    static_cast<Writer&>(writer).Put(n);
    for (int32_t i = 0; i < n; ++i)
    {
        writer.Put(exportSymbols[i]);
    }
}

void ContainerSymbol::Read(SymbolReader& reader)
{
    Symbol::Read(reader);
    int32_t n = reader.GetInt();
    for (int32_t i = 0; i < n; ++i)
    {
        Symbol* symbol = reader.GetSymbol();
        AddSymbol(std::unique_ptr<Symbol>(symbol));
    }
}

void ContainerSymbol::AddSymbol(std::unique_ptr<Symbol>&& symbol)
{
    if (FunctionSymbol* functionSymbol = dynamic_cast<FunctionSymbol*>(symbol.get()))
    {
        FunctionGroupSymbol* functionGroupSymbol = MakeFunctionGroupSymbol(functionSymbol->GroupName(), functionSymbol->GetSpan());
        functionGroupSymbol->AddFunction(functionSymbol);
    }
    else if (!dynamic_cast<DeclarationBlock*>(symbol.get()))
    {
        containerScope.Install(symbol.get());
        if (TypeSymbol* type = dynamic_cast<TypeSymbol*>(symbol.get()))
        {
            GetAssembly()->GetSymbolTable().AddType(type);
        }
    }
    symbol->SetParent(this);
    symbols.push_back(std::move(symbol));
}

void ContainerSymbol::Clear()
{
    containerScope.Clear();
    symbols.clear();
}

FunctionGroupSymbol* ContainerSymbol::MakeFunctionGroupSymbol(StringPtr groupName, const Span& span)
{
    Symbol* symbol = containerScope.Lookup(groupName);
    if (!symbol)
    {
        ConstantPool& constantPool = GetAssembly()->GetConstantPool();
        Constant groupNameConstant = constantPool.GetConstant(constantPool.Install(groupName));
        std::unique_ptr<FunctionGroupSymbol> functionGroupSymbol(new FunctionGroupSymbol(span, groupNameConstant, &containerScope));
        functionGroupSymbol->SetAssembly(GetAssembly());
        FunctionGroupSymbol* fnGroupSymbol = functionGroupSymbol.get();
        AddSymbol(std::move(functionGroupSymbol));
        return fnGroupSymbol;
    }
    if (FunctionGroupSymbol* functionGroupSymbol = dynamic_cast<FunctionGroupSymbol*>(symbol))
    {
        return functionGroupSymbol;
    }
    else
    {
        throw Exception("name of symbol '" + ToUtf8(symbol->FullName()) + "' conflicts with a function group '" + ToUtf8(groupName.Value()) + "'", symbol->GetSpan(), span);
    }
}

NamespaceSymbol::NamespaceSymbol(const Span& span_, Constant name_) : ContainerSymbol(span_, name_)
{
}

void NamespaceSymbol::Import(NamespaceSymbol* that, SymbolTable& symbolTable)
{
    symbolTable.BeginNamespace(that->Name(), that->GetSpan());
    for (std::unique_ptr<Symbol>& symbol : that->Symbols())
    {
        if (NamespaceSymbol* thatNs = dynamic_cast<NamespaceSymbol*>(symbol.get()))
        {
            Import(thatNs, symbolTable);
        }
        else if (!dynamic_cast<FunctionGroupSymbol*>(symbol.get()))
        {
            std::unique_ptr<Symbol> s(symbol.release());
            symbolTable.Container()->AddSymbol(std::move(s));
        }
    }
    symbolTable.EndNamespace();
}

DeclarationBlock::DeclarationBlock(const Span& span_, Constant name_) : ContainerSymbol(span_, name_)
{
}

TypeSymbol::TypeSymbol(const Span& span_, Constant name_) : Symbol(span_, name_)
{
}

BasicTypeSymbol::BasicTypeSymbol(const Span& span_, Constant name_) : TypeSymbol(span_, name_)
{
}

BoolTypeSymbol::BoolTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

CharTypeSymbol::CharTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

VoidTypeSymbol::VoidTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

SByteTypeSymbol::SByteTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

ByteTypeSymbol::ByteTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

ShortTypeSymbol::ShortTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

UShortTypeSymbol::UShortTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

IntTypeSymbol::IntTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

UIntTypeSymbol::UIntTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

LongTypeSymbol::LongTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

ULongTypeSymbol::ULongTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

FloatTypeSymbol::FloatTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

DoubleTypeSymbol::DoubleTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

NullReferenceTypeSymbol::NullReferenceTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
}

ClassTypeSymbol::ClassTypeSymbol(const Span& span_, Constant name_) : TypeSymbol(span_, name_), ContainerSymbol(span_, name_)
{
}

void ClassTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    ContainerSymbol::Write(writer);
}

void ClassTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    ContainerSymbol::Read(reader);
}

StringTypeSymbol::StringTypeSymbol(const Span& span_, Constant name_) : ClassTypeSymbol(span_, name_)
{
}

void StringTypeSymbol::Write(SymbolWriter& writer)
{
    ClassTypeSymbol::Write(writer);
}

void StringTypeSymbol::Read(SymbolReader& reader)
{
    ClassTypeSymbol::Read(reader);
}

SymbolTable::SymbolTable(Assembly* assembly_) : assembly(assembly_), globalNs(Span(), assembly->GetConstantPool().GetEmptyStringConstant()), container(&globalNs), function(nullptr), mainFunction(nullptr)
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
}

void SymbolTable::EndFunction()
{
    EndContainer();
    container->AddSymbol(std::unique_ptr<Symbol>(function));
    function = nullptr;
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
    StringPtr name(U"@local");
    Constant nameConstant = constantPool.GetConstant(constantPool.Install(name));
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

void SymbolTable::Write(SymbolWriter& writer)
{
    globalNs.Write(writer);
}

void SymbolTable::Read(SymbolReader& reader)
{
    globalNs.Read(reader);
}

void SymbolTable::Import(SymbolTable& symbolTable)
{
    globalNs.Import(&symbolTable.globalNs, *this);
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

void SymbolTable::MapNode(Node& node, Symbol* symbol)
{
    nodeSymbolMap[&node] = symbol;
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
        T* p = new T(span, name);
        return p->ToSymbol();
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
    SymbolFactory::Instance().Register(SymbolType::stringTypeSymbol, new ConcreteSymbolCreator<StringTypeSymbol>());
    SymbolFactory::Instance().Register(SymbolType::functionSymbol, new ConcreteSymbolCreator<FunctionSymbol>());
    SymbolFactory::Instance().Register(SymbolType::parameterSymbol, new ConcreteSymbolCreator<ParameterSymbol>());
    SymbolFactory::Instance().Register(SymbolType::localVariableSymbol, new ConcreteSymbolCreator<LocalVariableSymbol>());
    SymbolFactory::Instance().Register(SymbolType::memberVariableSymbol, new ConcreteSymbolCreator<MemberVariableSymbol>());
    SymbolFactory::Instance().Register(SymbolType::constantSymbol, new ConcreteSymbolCreator<ConstantSymbol>());
}

void DoneSymbol()
{
    SymbolFactory::Done();
}

std::unique_ptr<Assembly> CreateSystemAssembly(const std::string& config)
{
    std::unique_ptr<Assembly> systemAssembly(new Assembly(U"system", CminorSystemAssemblyFilePath(config)));
    TypeSymbol* boolTypeSymbol = new BoolTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"bool")));
    boolTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(boolTypeSymbol));
    TypeSymbol* charTypeSymbol = new CharTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"char")));
    charTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(charTypeSymbol));
    TypeSymbol* voidTypeSymbol = new VoidTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"void")));
    voidTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(voidTypeSymbol));
    TypeSymbol* sbyteTypeSymbol = new SByteTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"sbyte")));
    sbyteTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(sbyteTypeSymbol));
    TypeSymbol* byteTypeSymbol = new ByteTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"byte")));
    byteTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(byteTypeSymbol));
    TypeSymbol* shortTypeSymbol = new ShortTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"short")));
    shortTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(shortTypeSymbol));
    TypeSymbol* ushortTypeSymbol = new UShortTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"ushort")));
    ushortTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(ushortTypeSymbol));
    TypeSymbol* intTypeSymbol = new IntTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"int")));
    intTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(intTypeSymbol));
    TypeSymbol* uintTypeSymbol = new UIntTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"uint")));
    uintTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(uintTypeSymbol));
    TypeSymbol* longTypeSymbol = new LongTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"long")));
    longTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(longTypeSymbol));
    TypeSymbol* ulongTypeSymbol = new ULongTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"ulong")));
    ulongTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(ulongTypeSymbol));
    TypeSymbol* floatTypeSymbol = new FloatTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"float")));
    floatTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(floatTypeSymbol));
    TypeSymbol* doubleTypeSymbol = new DoubleTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"double")));
    doubleTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(doubleTypeSymbol));
    TypeSymbol* nullReferenceTypeSymbol = new NullReferenceTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"@nullref")));
    nullReferenceTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(nullReferenceTypeSymbol));
    TypeSymbol* stringTypeSymbol = new StringTypeSymbol(Span(), systemAssembly->GetConstantPool().GetConstant(systemAssembly->GetConstantPool().Install(U"string")));
    stringTypeSymbol->SetAssembly(systemAssembly.get());
    systemAssembly->GetSymbolTable().GlobalNs().AddSymbol(std::unique_ptr<TypeSymbol>(stringTypeSymbol));
    InitBasicTypeFun(*systemAssembly);
    return systemAssembly;
}

} } // namespace cminor::symbols
