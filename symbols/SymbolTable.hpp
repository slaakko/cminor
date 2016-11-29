// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_SYMBOL_TABLE_INCLUDED
#define CMINOR_SYMBOLS_SYMBOL_TABLE_INCLUDED
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/ast/Expression.hpp>
#include <cminor/ast/Interface.hpp>
#include <cminor/ast/Constant.hpp>
#include <cminor/ast/Enumeration.hpp>

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
    void BeginClassTemplateSpecialization(ClassNode& classInstanceNode, ClassTemplateSpecializationSymbol* classTemplateSpecialization);
    void EndClassTemplateSpecialization();
    void BeginInterface(InterfaceNode& interfaceNode);
    void EndInterface();
    void AddParameter(ParameterNode& parameterNode);
    void AddTemplateParameter(TemplateParameterNode& templateParameterNode);
    void BeginDeclarationBlock(Node& node);
    void EndDeclarationBlock();
    void AddLocalVariable(ConstructionStatementNode& constructionStatementNode);
    void AddLocalVariable(IdentifierNode& idNode);
    void AddMemberVariable(MemberVariableNode& memberVariableNode);
    void BeginProperty(PropertyNode& propertyNode);
    void EndProperty();
    void BeginPropertyGetter(PropertyNode& propertyNode);
    void EndPropertyGetter();
    void BeginPropertySetter(PropertyNode& propertyNode);
    void EndPropertySetter();
    void BeginIndexer(IndexerNode& indexerNode);
    void EndIndexer();
    void BeginIndexerGetter(IndexerNode& indexerNode);
    void EndIndexerGetter();
    void BeginIndexerSetter(IndexerNode& indexerNode);
    void EndIndexerSetter();
    void BeginEnumType(EnumTypeNode& enumTypeNode);
    void EndEnumType();
    void AddEnumConstant(EnumConstantNode& enumConstantNode);
    void AddConstant(ConstantNode& constantNode);
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader);
    void Import(SymbolTable& symbolTable);
    void Clear();
    TypeSymbol* GetTypeNoThrow(const utf32_string& typeFullName) const;
    TypeSymbol* GetType(const utf32_string& typeFullName) const;
    void AddType(TypeSymbol* type);
    Symbol* GetSymbol(Node& node) const;
    Symbol* GetSymbol(uint32_t symbolId) const;
    Symbol* GetSymbolNothrow(uint32_t symbolId) const;
    void AddSymbol(Symbol* symbol);
    Node* GetNode(Symbol* symbol) const;
    Node* GetNodeNothrow(Symbol* symbol) const;
    void MapNode(Node& node, Symbol* symbol);
    void AddConversion(FunctionSymbol* conversionFun);
    const ConversionTable& GetConversionTable() const { return conversionTable; }
    TypeSymbol* CreateArrayType(ArrayNode& arrayNode, TypeSymbol* elementType);
    const std::vector<ArrayTypeSymbol*>& CreatedArrays() const { return createdArrays; }
    ClassTemplateSpecializationSymbol* MakeClassTemplateSpecialization(ClassTypeSymbol* primaryClassTemplate, const std::vector<TypeSymbol*>& typeArguments, const Span& span);
    bool AddTypes() const { return !doNotAddTypes; }
    void MergeClassTemplateSpecializations();
private:
    Assembly* assembly;
    NamespaceSymbol globalNs;
    ContainerSymbol* container;
    std::stack<ContainerSymbol*> containerStack;
    FunctionSymbol* function;
    FunctionSymbol* mainFunction;
    ClassTypeSymbol* currentClass;
    std::stack<ClassTypeSymbol*> classStack;
    InterfaceTypeSymbol* currentInterface;
    std::stack<InterfaceTypeSymbol*> interfaceStack;
    std::unordered_map<Constant, TypeSymbol*, ConstantHash> typeSymbolMap;
    std::unordered_map<Node*, Symbol*> nodeSymbolMap;
    std::unordered_map<Symbol*, Node*> symbolNodeMap;
    ConversionTable conversionTable;
    int declarationBlockId;
    bool doNotAddTypes;
    bool doNotAddClassTemplateSpecializations;
    std::vector<ArrayTypeSymbol*> createdArrays;
    std::unordered_map<ClassTemplateSpecializationKey, ClassTemplateSpecializationSymbol*, ClassTemplateSpecializationKeyHash> classTemplateSpecializationMap;
    uint32_t nextSymbolId;
    std::unordered_map<uint32_t, Symbol*> idSymbolMap;
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
