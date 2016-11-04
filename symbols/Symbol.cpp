// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/SymbolTable.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/ConstantSymbol.hpp>
#include <cminor/symbols/BasicTypeFun.hpp>
#include <cminor/symbols/ObjectFun.hpp>
#include <cminor/symbols/IndexerSymbol.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/machine/Util.hpp>
#include <boost/filesystem.hpp>

namespace cminor { namespace symbols {

const char* symbolTypeStr[uint8_t(SymbolType::maxSymbol)] =
{
    "boolTypeSymbol", "charTypeSymbol", "voidTypeSymbol", "sbyteTypeSymbol", "byteTypeSymbol", "shortTypeSymbol", "ushortTypeSymbol", "intTypeSymbol", "uintTypeSymbol", "longTypeSymbol", 
    "ulongTypeSymbol", "floatTypeSymbol", "doubleTypeSymbol", "nullReferenceTypeSymbol", 
    "classTypeSymbol", "stringTypeSymbol", "functionSymbol", "functionGroupSymbol", "parameterSymbol", "localVariableSymbol", "memberVariableSymbol", "constantSymbol", "namespaceSymbol",
    "declarationBlock", "basicTypeDefaultConstructor", "basicTypeInitConstructor", "basicTypeAssignment", "basicTypeReturn", "basicTypeConversion", "basicTypeUnaryOp", "basicTypBinaryOp"
};

std::string SymbolTypeStr(SymbolType symbolType)
{
    return symbolTypeStr[uint8_t(symbolType)];
}

std::string SymbolFlagStr(SymbolFlags flags)
{
    std::string s;
    SymbolAccess access = SymbolAccess(flags & SymbolFlags::access);
    switch (access)
    {
        case SymbolAccess::private_: s.append("private"); break;
        case SymbolAccess::protected_: s.append("protected"); break;
        case SymbolAccess::internal_: s.append("internal"); break;
        case SymbolAccess::public_: s.append("public"); break;
    }
    if ((flags & SymbolFlags::static_) != SymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("static");
    }
    return s;
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
    writer.AsMachineWriter().Put(uint8_t(flags & ~(SymbolFlags::project | SymbolFlags::bound)));
}

void Symbol::Read(SymbolReader& reader)
{
    flags = static_cast<SymbolFlags>(reader.GetByte());
}

void Symbol::SetAccess(Specifiers accessSpecifiers)
{
    ContainerSymbol* cls = ContainingClass();
    SymbolAccess access = SymbolAccess::private_;
    bool classMember = true;
    if (!cls)
    {
        access = SymbolAccess::internal_;
        classMember = false;
        ContainerSymbol* intf = ContainingInterface();
        if (intf)
        {
            access = SymbolAccess::public_;
        }
    }
    if (accessSpecifiers == Specifiers::public_)
    {
        access = SymbolAccess::public_;
    }
    else if (accessSpecifiers == Specifiers::protected_)
    {
        if (classMember)
        {
            access = SymbolAccess::protected_;
        }
        else
        {
            throw Exception("only class members can have protected access", GetSpan());
        }
    }
    else if (accessSpecifiers == Specifiers::internal_)
    {
        access = SymbolAccess::internal_;
    }
    else if (accessSpecifiers == Specifiers::private_)
    {
        if (classMember)
        {
            access = SymbolAccess::private_;
        }
        else
        {
            throw Exception("only class members can have private access", GetSpan());
        }
    }
    else if (accessSpecifiers != Specifiers::none)
    {
        throw Exception("invalid combination of access specifiers: " + SpecifierStr(accessSpecifiers), GetSpan());
    }
    SetAccess(access);
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

bool Symbol::IsSameParentOrAncestorOf(const Symbol* that) const
{
    if (!that)
    {
        return false;
    }
    else if (this == that)
    {
        return true;
    }
    else if (that->parent)
    {
        return IsSameParentOrAncestorOf(that->parent);
    }
    else
    {
        return false;
    }
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

ClassTypeSymbol* Symbol::Class() const
{
    if (const ClassTypeSymbol* cls = dynamic_cast<const ClassTypeSymbol*>(this))
    {
        return const_cast<ClassTypeSymbol*>(cls);
    }
    else
    {
        if (parent)
        {
            return parent->Class();
        }
        else
        {
            throw std::runtime_error("class not found");
        }
    }
}

ClassTypeSymbol* Symbol::ClassNoThrow() const
{
    if (const ClassTypeSymbol* cls = dynamic_cast<const ClassTypeSymbol*>(this))
    {
        return const_cast<ClassTypeSymbol*>(cls);
    }
    else
    {
        if (parent)
        {
            return parent->ClassNoThrow();
        }
        else
        {
            return nullptr;
        }
    }
}

InterfaceTypeSymbol* Symbol::InterfaceNoThrow() const
{
    if (const InterfaceTypeSymbol* intf = dynamic_cast<const InterfaceTypeSymbol*>(this))
    {
        return const_cast<InterfaceTypeSymbol*>(intf);
    }
    else
    {
        if (parent)
        {
            return parent->InterfaceNoThrow();
        }
        else
        {
            return nullptr;
        }
    }
}

ContainerSymbol* Symbol::ClassOrNs() const
{
    if (const NamespaceSymbol* ns = dynamic_cast<const NamespaceSymbol*>(this))
    {
        return const_cast<NamespaceSymbol*>(ns);
    }
    else if (const ClassTypeSymbol* cls = dynamic_cast<const ClassTypeSymbol*>(this))
    {
        return const_cast<ClassTypeSymbol*>(cls);
    }
    else
    {
        if (parent)
        {
            return parent->ClassOrNs();
        }
        else
        {
            throw std::runtime_error("class or namespace not found");
        }
    }
}

ContainerSymbol* Symbol::ClassInterfaceOrNs() const
{
    if (const NamespaceSymbol* ns = dynamic_cast<const NamespaceSymbol*>(this))
    {
        return const_cast<NamespaceSymbol*>(ns);
    }
    else if (const ClassTypeSymbol* cls = dynamic_cast<const ClassTypeSymbol*>(this))
    {
        return const_cast<ClassTypeSymbol*>(cls);
    }
    else if (const InterfaceTypeSymbol* intf = dynamic_cast<const InterfaceTypeSymbol*>(this))
    {
        return const_cast<InterfaceTypeSymbol*>(intf);
    }
    else
    {
        if (parent)
        {
            return parent->ClassInterfaceOrNs();
        }
        else
        {
            throw std::runtime_error("class, interface or namespace not found");
        }
    }
}

FunctionSymbol* Symbol::GetFunction() const
{
    if (const FunctionSymbol* fun = dynamic_cast<const FunctionSymbol*>(this))
    {
        return const_cast<FunctionSymbol*>(fun);
    }
    else
    {
        if (parent)
        {
            return parent->GetFunction();
        }
        else
        {
            return nullptr;
        }
    }
}

FunctionSymbol* Symbol::ContainingFunction() const
{
    if (parent)
    {
        return parent->GetFunction();
    }
    else
    {
        return nullptr;
    }
}

ClassTypeSymbol* Symbol::ContainingClass() const
{
    if (parent)
    {
        return parent->ClassNoThrow();
    }
    else
    {
        return nullptr;
    }
}

InterfaceTypeSymbol* Symbol::ContainingInterface() const
{
    if (parent)
    {
        return  parent->InterfaceNoThrow();
    }
    else
    {
        return nullptr;
    }
}

ContainerScope* Symbol::NsScope() const
{
    ContainerSymbol* ns = Ns();
    return ns->GetContainerScope();
}

ContainerScope* Symbol::ClassOrNsScope() const
{
    ContainerSymbol* classOrNs = ClassOrNs();
    if (classOrNs)
    {
        return classOrNs->GetContainerScope();
    }
    else
    {
        throw std::runtime_error("class or namespace scope not found");
    }
}

ContainerScope* Symbol::ClassInterfaceOrNsScope() const
{
    ContainerSymbol* classInterfaceOrNs = ClassInterfaceOrNs();
    if (classInterfaceOrNs)
    {
        return classInterfaceOrNs->GetContainerScope();
    }
    else
    {
        throw std::runtime_error("class, interface or namespace scope not found");
    }
}

void Symbol::EmplaceType(TypeSymbol* type, int index)
{
    throw std::runtime_error("symbol '" + ToUtf8(FullName()) + "' does not support emplace type");
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
        if (ContainerSymbol* containerSymbol = dynamic_cast<ContainerSymbol*>(symbol))
        {
            containerSymbol->GetContainerScope()->SetParent(this);
        }
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
            containerScope->CollectViableFunctions(arity, groupName, scopesLookedUp, ScopeLookup::this_, viableFunctions);
            scopesLookedUp.insert(containerScope);
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
    writer.AsMachineWriter().Put(n);
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
        if (FunctionSymbol* functionSymbol = dynamic_cast<FunctionSymbol*>(symbol))
        {
            if (functionSymbol->IsConversionFun())
            {
                reader.AddConversionFun(functionSymbol);
            }
        }
    }
}

void ContainerSymbol::AddSymbol(std::unique_ptr<Symbol>&& symbol)
{
    symbol->SetParent(this);
    if (FunctionSymbol* functionSymbol = dynamic_cast<FunctionSymbol*>(symbol.get()))
    {
        FunctionGroupSymbol* functionGroupSymbol = MakeFunctionGroupSymbol(functionSymbol->GroupName(), functionSymbol->GetSpan());
        functionGroupSymbol->AddFunction(functionSymbol);
    }
    else if (IndexerSymbol* indexerSymbol = dynamic_cast<IndexerSymbol*>(symbol.get()))
    {
        IndexerGroupSymbol* indexerGroupSymbol = MakeIndexerGroupSymbol(indexerSymbol->GetSpan());
        indexerGroupSymbol->AddIndexer(indexerSymbol);
    }
    else 
    {
        containerScope.Install(symbol.get());
        if (TypeSymbol* type = dynamic_cast<TypeSymbol*>(symbol.get()))
        {
            GetAssembly()->GetSymbolTable().AddType(type);
        }
    }
    symbols.push_back(std::move(symbol));
}

void ContainerSymbol::Clear()
{
    containerScope.Clear();
    symbols.clear();
}

FunctionGroupSymbol* ContainerSymbol::MakeFunctionGroupSymbol(StringPtr groupName, const Span& span)
{
    if (!groupName.Value())
    {
        throw std::runtime_error("no group name set");
    }
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

IndexerGroupSymbol* ContainerSymbol::MakeIndexerGroupSymbol(const Span& span)
{
    Symbol* symbol = containerScope.Lookup(StringPtr(U"@indexers"));
    if (!symbol)
    {
        ConstantPool& constantPool = GetAssembly()->GetConstantPool();
        Constant nameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(U"@indexers")));
        std::unique_ptr<IndexerGroupSymbol> indexerGroupSymbol(new IndexerGroupSymbol(span, nameConstant));
        indexerGroupSymbol->SetAssembly(GetAssembly());
        IndexerGroupSymbol* indexerGroup = indexerGroupSymbol.get();
        AddSymbol(std::move(indexerGroupSymbol));
        return indexerGroup;
    }
    if (IndexerGroupSymbol* indexerGroupSymbol = dynamic_cast<IndexerGroupSymbol*>(symbol))
    {
        return indexerGroupSymbol;
    }
    else
    {
        Assert(false, "indexer group symbol expected");
        return nullptr;
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

void DeclarationBlock::AddSymbol(std::unique_ptr<Symbol>&& symbol)
{
    Symbol* ptr = symbol.get();
    ContainerSymbol::AddSymbol(std::move(symbol));
    if (LocalVariableSymbol* localVariableSymbol = dynamic_cast<LocalVariableSymbol*>(ptr))
    {
        FunctionSymbol* functionSymbol = GetFunction();
        if (functionSymbol)
        {
            functionSymbol->AddLocalVariable(localVariableSymbol);
        }
    }
}

TypeSymbol::TypeSymbol(const Span& span_, Constant name_) : ContainerSymbol(span_, name_)
{
}

BasicTypeSymbol::BasicTypeSymbol(const Span& span_, Constant name_) : TypeSymbol(span_, name_)
{
}

void BasicTypeSymbol::SetMachineType(BasicType* machineType_)
{
    machineType.reset(machineType_);
    machineType->SetNameConstant(NameConstant());
}

void BasicTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    machineType->Write(writer.AsMachineWriter());
}

void BasicTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    machineType->Read(reader);
    TypeTable::Instance().SetType(machineType.get());
}

BoolTypeSymbol::BoolTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new Bool());
}

CharTypeSymbol::CharTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new Char());
}

VoidTypeSymbol::VoidTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new Void());
}

SByteTypeSymbol::SByteTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new SByte());
}

ByteTypeSymbol::ByteTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new Byte());
}

ShortTypeSymbol::ShortTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new Short());
}

UShortTypeSymbol::UShortTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new UShort());
}

IntTypeSymbol::IntTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new Int());
}

UIntTypeSymbol::UIntTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new UInt());
}

LongTypeSymbol::LongTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new Long());
}

ULongTypeSymbol::ULongTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new ULong());
}

FloatTypeSymbol::FloatTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new Float());
}

DoubleTypeSymbol::DoubleTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new Double());
}

NullReferenceTypeSymbol::NullReferenceTypeSymbol(const Span& span_, Constant name_) : BasicTypeSymbol(span_, name_)
{
    SetMachineType(new Null());
}

ClassTypeSymbol::ClassTypeSymbol(const Span& span_, Constant name_) : TypeSymbol(span_, name_), baseClass(nullptr), objectType(new ObjectType()), classData(new ClassData(objectType.get())), 
    flags(), defaultConstructorSymbol(nullptr), level(0), priority(0), key(0), cid(0)
{
}

void ClassTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    bool hasBaseClass = baseClass != nullptr;
    writer.AsMachineWriter().Put(hasBaseClass);
    if (hasBaseClass)
    {
        utf32_string baseClassFullName = baseClass->FullName();
        ConstantId baseClassNameId = GetAssembly()->GetConstantPool().GetIdFor(baseClassFullName);
        Assert(baseClassNameId != noConstantId, "got no id");
        baseClassNameId.Write(writer);
    }
    int32_t n = int32_t(implementedInterfaces.size());
    writer.AsMachineWriter().Put(n);
    for (int32_t i = 0; i < n; ++i)
    {
        InterfaceTypeSymbol* intf = implementedInterfaces[i];
        utf32_string interfaceFullName = intf->FullName();
        ConstantId intfNameId = GetAssembly()->GetConstantPool().GetIdFor(interfaceFullName);
        Assert(intfNameId != noConstantId, "got no id");
        intfNameId.Write(writer);
    }
    Assert(objectType, "object type not set");
    objectType->Write(writer);
    classData->Write(writer);
    writer.AsMachineWriter().Put(uint8_t(flags));
}

void ClassTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    bool hasBaseClass = reader.GetBool();
    if (hasBaseClass)
    {
        ConstantId baseClassNameId;
        baseClassNameId.Read(reader);
        reader.EmplaceTypeRequest(this, baseClassNameId, 0);
    }
    int32_t n = reader.GetInt();
    for (int32_t i = 0; i < n; ++i)
    {
        ConstantId intfNameId;
        intfNameId.Read(reader);
        reader.EmplaceTypeRequest(this, intfNameId, -1);
    }
    Assert(objectType, "object type not set");
    objectType->Read(reader);
    TypeTable::Instance().SetType(objectType.get());
    classData->Read(reader);
    ClassDataTable::Instance().SetClassData(classData.get());
    flags = ClassTypeSymbolFlags(reader.GetByte());
    reader.AddClassTypeSymbol(this);
}

void ClassTypeSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        SetStatic();
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        SetAbstract();
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be inline", GetSpan());
    }
}

void ClassTypeSymbol::AddSymbol(std::unique_ptr<Symbol>&& symbol)
{
    Symbol* s = symbol.get();
    TypeSymbol::AddSymbol(std::move(symbol));
    if (MemberVariableSymbol* memberVariableSymbol = dynamic_cast<MemberVariableSymbol*>(s))
    {
        if (memberVariableSymbol->IsStatic())
        {
            staticMemberVariables.push_back(memberVariableSymbol);
        }
        else
        {
            memberVariables.push_back(memberVariableSymbol);
        }
    }
    else if (MemberFunctionSymbol* memberFunctionSymbol = dynamic_cast<MemberFunctionSymbol*>(s))
    {
        memberFunctions.push_back(memberFunctionSymbol);
    }
    else if (ConstructorSymbol* constructorSymbol = dynamic_cast<ConstructorSymbol*>(s))
    {
        constructors.push_back(constructorSymbol);
        if (constructorSymbol->IsDefaultConstructorSymbol())
        {
            defaultConstructorSymbol = constructorSymbol;
        }
    }
}

void ClassTypeSymbol::AddImplementedInterface(InterfaceTypeSymbol* interfaceTypeSymbol)
{
    if (std::find(implementedInterfaces.cbegin(), implementedInterfaces.cend(), interfaceTypeSymbol) == implementedInterfaces.cend())
    {
        implementedInterfaces.push_back(interfaceTypeSymbol);
    }
}

void ClassTypeSymbol::EmplaceType(TypeSymbol* type, int index)
{
    if (index == 0)
    {
        if (ClassTypeSymbol* baseClassTypeSymbol = dynamic_cast<ClassTypeSymbol*>(type))
        {
            baseClass = baseClassTypeSymbol;
            GetContainerScope()->SetBase(baseClass->GetContainerScope());
        }
        else
        {
            throw std::runtime_error("class type symbol expected");
        }
    }
    else if (index == -1)
    {
        if (InterfaceTypeSymbol* interfaceTypeSymbol = dynamic_cast<InterfaceTypeSymbol*>(type))
        {
            AddImplementedInterface(interfaceTypeSymbol);
        }
        else
        {
            throw std::runtime_error("interface type symbol expected");
        }
    }
    else
    {
        throw std::runtime_error("class type symbol emplace type got invalid type index " + std::to_string(index));
    }
}

bool ClassTypeSymbol::HasBaseClass(ClassTypeSymbol* cls) const
{
    if (!baseClass) return false;
    if (baseClass == cls || baseClass->HasBaseClass(cls)) return true;
    return false;
}

bool ClassTypeSymbol::HasBaseClass(ClassTypeSymbol* cls, int& distance) const
{
    if (!baseClass) return false;
    ++distance;
    if (baseClass == cls) return true;
    return baseClass->HasBaseClass(cls, distance);
}

void ClassTypeSymbol::SetCid(uint64_t cid_) 
{ 
    cid = cid_; 
    Assert(objectType, "object type not set"); 
    objectType->SetId(cid); 
}

bool Overrides(MemberFunctionSymbol* f, MemberFunctionSymbol* g)
{
    if (f->GroupName() == g->GroupName())
    {
        int n = int(f->Parameters().size());
        if (n == int(g->Parameters().size()))
        {
            for (int i = 1; i < n; ++i)
            {
                ParameterSymbol* p = f->Parameters()[i];
                ParameterSymbol* q = g->Parameters()[i];
                if (p->GetType() != q->GetType()) return false;
            }
            return true;
        }
    }
    return false;
}

void ClassTypeSymbol::InitVmt(std::vector<MemberFunctionSymbol*>& vmt)
{
    if (baseClass)
    {
        baseClass->InitVmt(vmt);
    }
    std::vector<MemberFunctionSymbol*> virtualFunctions;
    int mfn = int(memberFunctions.size());
    for (int i = 0; i < mfn; ++i)
    {
        MemberFunctionSymbol* mf = memberFunctions[i];
        if (mf->IsVirtualAbstractOrOverride())
        {
            virtualFunctions.push_back(mf);
        }
    }
    int32_t n = int32_t(virtualFunctions.size());
    for (int32_t i = 0; i < n; ++i)
    {
        MemberFunctionSymbol* f = virtualFunctions[i];
        bool found = false;
        int32_t m = int32_t(vmt.size());
        for (int32_t j = 0; j < m; ++j)
        {
            MemberFunctionSymbol* v = vmt[j];
            if (Overrides(f, v))
            {
                if (!f->IsOverride())
                {
                    throw Exception("overriding functionn should be declared with override specifier", f->GetSpan());
                }
                f->SetVmtIndex(j);
                vmt[j] = f;
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (f->IsOverride())
            {
                throw Exception("no suitable function to override ('" + ToUtf8(f->FullParsingName()) + "')", f->GetSpan());
            }
            f->SetVmtIndex(m);
            vmt.push_back(f);
        }
    }
}

void ClassTypeSymbol::InitVmt()
{
    std::vector<MemberFunctionSymbol*> vmt;
    InitVmt(vmt);
    Assert(classData, "class data not set");
    int32_t n = int32_t(vmt.size());
    classData->Vmt().Resize(n);
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    for (int32_t i = 0; i < n; ++i)
    {
        MemberFunctionSymbol* f = vmt[i];
        if (f->IsAbstract())
        {
            if (!IsAbstract())
            {
                throw Exception("class containing abstract member functions must be declared abstract", GetSpan(), f->GetSpan());
            }
            classData->Vmt().SetMethodName(i, constantPool.GetEmptyStringConstant());
        }
        else
        {
            utf32_string fullName = f->FullName();
            Constant fullNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullName.c_str())));
            classData->Vmt().SetMethodName(i, fullNameConstant);
        }
    }
}

void ClassTypeSymbol::LinkVmt()
{
    Assert(classData, "class data not set");
    MethodTable& vmt = classData->Vmt();
    int32_t n = vmt.Count();
    for (int32_t i = 0; i < n; ++i)
    {
        StringPtr methodName = vmt.GetMethodName(i);
        if (methodName == StringPtr(U""))
        {
            Function* method = nullptr;
            vmt.SetMethod(i, method);
        }
        else
        {
            Function* method = FunctionTable::Instance().GetFunction(methodName);
            vmt.SetMethod(i, method);
        }
    }
}

bool Implements(MemberFunctionSymbol* classMemFun, MemberFunctionSymbol* interfaceMemFun)
{
    if (classMemFun->GroupName() != interfaceMemFun->GroupName()) return false;
    if (!classMemFun->ReturnType() || !interfaceMemFun->ReturnType()) return false;
    if (classMemFun->ReturnType() != interfaceMemFun->ReturnType()) return false;
    int n = int(classMemFun->Parameters().size());
    if (n != int(interfaceMemFun->Parameters().size())) return false;
    for (int i = 1; i < n; ++i)
    {
        TypeSymbol* classMemFunParamType = classMemFun->Parameters()[i]->GetType();
        TypeSymbol* intfMemFunParamType = interfaceMemFun->Parameters()[i]->GetType();
        if (classMemFunParamType != intfMemFunParamType) return false;
    }
    return true;
}

void ClassTypeSymbol::InitImts()
{
    ConstantPool& constantPool = GetAssembly()->GetConstantPool();
    int32_t n = int32_t(implementedInterfaces.size());
    if (n == 0) return;
    Assert(classData, "class data not set");
    classData->AllocImts(n);
    for (int32_t i = 0; i < n; ++i)
    {
        MethodTable& imt = classData->Imt(i);
        InterfaceTypeSymbol* intf = implementedInterfaces[i];
        int q = int(intf->MemberFunctions().size());
        imt.Resize(q);
        for (int k = 0; k < q; ++k)
        {
            imt.SetMethodName(k, constantPool.GetEmptyStringConstant());
        }
    }
    int m = int(MemberFunctions().size());
    for (int j = 0; j < m; ++j)
    {
        MemberFunctionSymbol* classMemFun = memberFunctions[j];
        for (int32_t i = 0; i < n; ++i)
        {
            MethodTable& imt = classData->Imt(i);
            InterfaceTypeSymbol* intf = implementedInterfaces[i];
            int q = int(intf->MemberFunctions().size());
            for (int k = 0; k < q; ++k)
            {
                MemberFunctionSymbol* intfMemFun = intf->MemberFunctions()[k];
                if (Implements(classMemFun, intfMemFun))
                {
                    utf32_string fullName = classMemFun->FullName();
                    Constant fullMethodNameConstant = constantPool.GetConstant(constantPool.Install(StringPtr(fullName.c_str())));
                    imt.SetMethodName(intfMemFun->ImtIndex(), fullMethodNameConstant);
                    break;
                }
            }
        }
    }
    for (int32_t i = 0; i < n; ++i)
    {
        InterfaceTypeSymbol* intf = implementedInterfaces[i];
        MethodTable& imt = classData->Imt(i);
        int32_t m = imt.Count();
        for (int j = 0; j < m; ++j)
        {
            StringPtr methodName = imt.GetMethodName(j);
            if (!methodName.Value() || methodName == U"")
            {
                MemberFunctionSymbol* intfMemFun = intf->MemberFunctions()[j];
                throw Exception("class '" + ToUtf8(FullName()) + "' does not implement interface '" + ToUtf8(intf->FullName()) + "' because implementation of interface function '" +
                    ToUtf8(intfMemFun->FullParsingName()) + "' is missing", GetSpan(), intfMemFun->GetSpan());
            }
        }
    }
}

void ClassTypeSymbol::LinkImts()
{
    Assert(classData, "class data not set");
    int32_t n = int32_t(implementedInterfaces.size());
    for (int32_t i = 0; i < n; ++i)
    {
        MethodTable& imt = classData->Imt(i);
        int32_t m = imt.Count();
        for (int32_t j = 0; j < m; ++j)
        {
            StringPtr methodName = imt.GetMethodName(j);
            if (methodName == StringPtr(U""))
            {
                Function* method = nullptr;
                imt.SetMethod(j, method);
            }
            else
            {
                Function* method = FunctionTable::Instance().GetFunction(methodName);
                imt.SetMethod(j, method);
            }
        }
    }
}

ArrayTypeSymbol::ArrayTypeSymbol(const Span& span_, Constant name_) : ClassTypeSymbol(span_, name_), elementType(nullptr)
{
}

void ArrayTypeSymbol::Write(SymbolWriter& writer)
{
    ClassTypeSymbol::Write(writer);
    ConstantId elementTypeId = writer.GetConstantPool()->GetIdFor(elementType->FullName());
    Assert(elementTypeId != noConstantId, "got no id for element type");
    elementTypeId.Write(writer);
}

void ArrayTypeSymbol::Read(SymbolReader& reader)
{
    ClassTypeSymbol::Read(reader);
    ConstantId elementTypeId;
    elementTypeId.Read(reader);
    reader.EmplaceTypeRequest(this, elementTypeId, 1);
}

void ArrayTypeSymbol::EmplaceType(TypeSymbol* type, int index)
{
    if (index == 1)
    {
        elementType = type;
    }
    else
    {
        ClassTypeSymbol::EmplaceType(type, index);
    }
}

InterfaceTypeSymbol::InterfaceTypeSymbol(const Span& span_, Constant name_) : TypeSymbol(span_, name_), objectType(new ObjectType())
{
}

void InterfaceTypeSymbol::AddSymbol(std::unique_ptr<Symbol>&& symbol)
{
    Symbol* s = symbol.get();
    TypeSymbol::AddSymbol(std::move(symbol));
    if (MemberFunctionSymbol* memFun = dynamic_cast<MemberFunctionSymbol*>(s))
    {
        memFun->SetImtIndex(int32_t(memberFunctions.size()));
        memberFunctions.push_back(memFun);
    }
}

void InterfaceTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    Assert(objectType, "object type not set");
    objectType->Write(writer);
}

void InterfaceTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    Assert(objectType, "object type not set");
    objectType->Read(reader);
    TypeTable::Instance().SetType(objectType.get());
}

void InterfaceTypeSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("interface cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("interface cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("interface cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("interface cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("interface cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("interface cannot be virtual", GetSpan());
    }
}

} } // namespace cminor::symbols
