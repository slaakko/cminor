// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/VariableSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolReader.hpp>

namespace cminor { namespace symbols {

FunctionSymbol::FunctionSymbol(const Span& span_, Constant name_) : ContainerSymbol(span_, name_), returnType(nullptr)
{
}

void FunctionSymbol::Write(SymbolWriter& writer)
{
    ContainerSymbol::Write(writer);
    ConstantId groupNameId = GetAssembly()->GetConstantPool().GetIdFor(groupName);
    Assert(groupNameId != noConstantId, "no id for group name found from constant pool");
    groupNameId.Write(static_cast<Writer&>(writer));
}

void FunctionSymbol::Read(SymbolReader& reader)
{
    ContainerSymbol::Read(reader);
    ConstantId groupNameId;
    groupNameId.Read(reader);
    groupName = reader.GetAssembly()->GetConstantPool().GetConstant(groupNameId);
}

void FunctionSymbol::AddSymbol(std::unique_ptr<Symbol>&& symbol)
{
    Symbol* s = symbol.get();
    ContainerSymbol::AddSymbol(std::move(symbol));
    if (ParameterSymbol* parameterSymbol = dynamic_cast<ParameterSymbol*>(s))
    {
        parameters.push_back(parameterSymbol);
    }
}

void FunctionSymbol::GenerateCode(Machine& machine, Function& function, std::vector<GenObject*>& objects)
{
}

FunctionGroupSymbol::FunctionGroupSymbol(const Span& span_, Constant name_, ContainerScope* containerScope_) : Symbol(span_, name_), containerScope(containerScope_)
{
}

void FunctionGroupSymbol::AddFunction(FunctionSymbol* function)
{
    Assert(function->GroupName() == Name(), "attempt to insert a function with group name '" + ToUtf8(function->GroupName().Value()) + "' to wrong function group '" + ToUtf8(Name().Value()) + "'");
    int arity = function->Arity();
    std::vector<FunctionSymbol*>& functionList = arityFunctionListMap[arity];
    functionList.push_back(function);
}

void FunctionGroupSymbol::CollectViableFunctions(int arity, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    auto it = arityFunctionListMap.find(arity);
    if (it != arityFunctionListMap.cend())
    {
        std::vector<FunctionSymbol*>& functionList = it->second;
        for (FunctionSymbol* function : functionList)
        {
            viableFunctions.insert(function);
        }
    }
}

} } // namespace cminor::symbols
