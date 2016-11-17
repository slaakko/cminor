// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_SYMBOL_READER_INCLUDED
#define CMINOR_SYMBOLS_SYMBOL_READER_INCLUDED
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/Value.hpp>
#include <cminor/ast/AstReader.hpp>

namespace cminor { namespace symbols {

using namespace cminor::machine;
using namespace cminor::ast;

class LocalVariableSymbol;

struct TypeRequest
{
    TypeRequest(Symbol* symbol_, ConstantId typeNameId_, int index_) : symbol(symbol_), typeNameId(typeNameId_), index(index_) {}
    Symbol* symbol;
    ConstantId typeNameId;
    int index;
};

class SymbolReader : public AstReader
{
public:
    SymbolReader(const std::string& fileName_);
    Assembly* GetAssembly() const { Assert(assembly, "assembly not set"); return assembly; }
    void SetAssembly(Assembly* assembly_) { assembly = assembly_; }
    Symbol* GetSymbol();
    Value* GetValue();
    void AddLocalVariable(LocalVariableSymbol* localVariable) { localVariables.push_back(localVariable); }
    std::vector<LocalVariableSymbol*> GetLocalVariables() { return std::move(localVariables); }
    void ResetLocalVariables() { localVariables = std::vector<LocalVariableSymbol*>(); }
    void AddType(TypeSymbol* type) { types.push_back(type); }
    const std::vector<TypeSymbol*>& Types() const { return types; }
    void ClearTypes() { types.clear(); }
    void EmplaceTypeRequest(Symbol* forSymbol, ConstantId typeNameId, int index);
    void ProcessTypeRequests();
    void AddClassTypeSymbol(ClassTypeSymbol* classType);
    std::vector<ClassTypeSymbol*> GetClassTypeSymbols() { return std::move(classTypeSymbols); }
    const std::vector<ClassTypeSymbol*>& ClassTypeSymbols() const { return classTypeSymbols; }
    void AddConversionFun(FunctionSymbol* conversion);
    const std::vector<FunctionSymbol*>& Conversions() const { return conversions; }
    void ClearConversions() { conversions.clear(); }
    void AddClassTemplateSpecialization(ClassTemplateSpecializationSymbol* classTemplateSpecialization);
    const std::vector<ClassTemplateSpecializationSymbol*>& ClassTemplateSpecializations() const { return classTemplateSpecializations; }
    void ClearClassTemplateSpecializations() { classTemplateSpecializations.clear(); }
private:
    Assembly* assembly;
    std::vector<LocalVariableSymbol*> localVariables;
    std::vector<TypeSymbol*> types;
    std::vector<TypeRequest> typeRequests;
    std::vector<ClassTypeSymbol*> classTypeSymbols;
    std::vector<FunctionSymbol*> conversions;
    std::vector<ClassTemplateSpecializationSymbol*> classTemplateSpecializations;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_SYMBOL_READER_INCLUDED
