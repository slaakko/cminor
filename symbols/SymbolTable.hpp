// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_SYMBOL_TABLE_INCLUDED
#define CMINOR_SYMBOLS_SYMBOL_TABLE_INCLUDED
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/ast/Expression.hpp>

namespace cminor { namespace symbols {

class SymbolTable
{
public:
    SymbolTable(Assembly* assembly_);
    NamespaceSymbol& GlobalNs() { return globalNs; }
    const NamespaceSymbol& GlobalNs() const { return globalNs; }
    ContainerSymbol* Container() const { return container; }
    void BeginContainer(ContainerSymbol* container_);
    void EndContainer();
    void BeginNamespace(NamespaceNode& namespaceNode);
    void BeginNamespace(StringPtr namespaceName, const Span& span);
    void EndNamespace();
    void BeginFunction(FunctionNode& functionNode);
    void EndFunction();
    void BeginStaticConstructor(StaticConstructorNode& staticConstructorNode);
    void EndStaticConstructor();
    void BeginConstructor(ConstructorNode& constructorNode);
    void EndConstructor();
    void BeginMemberFunction(MemberFunctionNode& memberFunctionNode);
    void EndMemberFunction();
    void BeginClass(ClassNode& classNode);
    void EndClass();
    void AddParameter(ParameterNode& parameterNode);
    void BeginDeclarationBlock(StatementNode& statementNode);
    void EndDeclarationBlock();
    void AddLocalVariable(ConstructionStatementNode& constructionStatementNode);
    void AddMemberVariable(MemberVariableNode& memberVariableNode);
    void BeginProperty(PropertyNode& propertyNode);
    void EndProperty();
    void BeginPropertyGetter(PropertyNode& propertyNode);
    void EndPropertyGetter();
    void BeginPropertySetter(PropertyNode& propertyNode);
    void EndPropertySetter();
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader);
    void Import(SymbolTable& symbolTable);
    void Clear();
    TypeSymbol* GetTypeNoThrow(const utf32_string& typeFullName) const;
    TypeSymbol* GetType(const utf32_string& typeFullName) const;
    void AddType(TypeSymbol* type);
    Symbol* GetSymbol(Node& node) const;
    Node* GetNode(Symbol* symbol) const;
    void MapNode(Node& node, Symbol* symbol);
    void AddConversion(FunctionSymbol* conversionFun);
    const ConversionTable& GetConversionTable() const { return conversionTable; }
    TypeSymbol* CreateArrayType(ArrayNode& arrayNode, TypeSymbol* elementType);
    const std::vector<ClassTypeSymbol*>& CreatedClasses() const { return createdClasses; }
private:
    Assembly* assembly;
    NamespaceSymbol globalNs;
    ContainerSymbol* container;
    FunctionSymbol* function;
    FunctionSymbol* mainFunction;
    ClassTypeSymbol* currentClass;
    std::stack<ClassTypeSymbol*> classStack;
    std::stack<ContainerSymbol*> containerStack;
    std::unordered_map<Constant, TypeSymbol*, ConstantHash> typeSymbolMap;
    std::unordered_map<Node*, Symbol*> nodeSymbolMap;
    std::unordered_map<Symbol*, Node*> symbolNodeMap;
    ConversionTable conversionTable;
    int declarationBlockId;
    bool doNotAddTypes;
    std::vector<ClassTypeSymbol*> createdClasses;
};

class SymbolCreator
{
public:
    virtual ~SymbolCreator();
    virtual Symbol* CreateSymbol(const Span& span, Constant name) = 0;
};

class SymbolFactory
{
public:
    SymbolFactory();
    static void Init();
    static void Done();
    static SymbolFactory& Instance();
    void Register(SymbolType symbolType, SymbolCreator* creator);
    Symbol* CreateSymbol(SymbolType symbolType, const Span& span, Constant name);
private:
    static std::unique_ptr<SymbolFactory> instance;
    std::vector<std::unique_ptr<SymbolCreator>> creators;
};

void InitSymbol();
void DoneSymbol();

std::unique_ptr<Assembly> CreateSystemCoreAssembly(Machine& machine_, const std::string& config);

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_SYMBOL_TABLE_INCLUDED
