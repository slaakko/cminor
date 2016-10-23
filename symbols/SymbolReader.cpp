// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/Assembly.hpp>

namespace cminor { namespace symbols {

SymbolReader::SymbolReader(const std::string& fileName_) : Reader(fileName_), assembly(nullptr)
{
}

Symbol* SymbolReader::GetSymbol()
{
    SymbolType symbolType = static_cast<SymbolType>(GetByte());
    Span span = GetSpan();
    ConstantId id;
    id.Read(*this);
    Constant name = assembly->GetConstantPool().GetConstant(id);
    Symbol* symbol = SymbolFactory::Instance().CreateSymbol(symbolType, span, name);
    symbol->SetAssembly(assembly);
    symbol->Read(*this);
    return symbol;
}

void SymbolReader::EmplaceTypeRequest(Symbol* forSymbol, ConstantId typeNameId, int index)
{
    typeRequests.push_back(TypeRequest(forSymbol, typeNameId, index));
}

void SymbolReader::ProcessTypeRequests()
{
    for (const TypeRequest& typeRequest : typeRequests)
    {
        Constant returnTypeNameConstant = assembly->GetConstantPool().GetConstant(typeRequest.typeNameId);
        utf32_string typeName = returnTypeNameConstant.Value().AsStringLiteral();
        TypeSymbol* type = assembly->GetSymbolTable().GetTypeNoThrow(typeName);
        if (type)
        {
            typeRequest.symbol->EmplaceType(type, typeRequest.index);
        }
        else
        {
            throw std::runtime_error("cannot satisfy type request for symbol '" + ToUtf8(typeRequest.symbol->FullName()) + "': type not found from symbol table");
        }
    }
    typeRequests.clear();
}

void SymbolReader::AddClassTypeSymbol(ClassTypeSymbol* classType)
{
    classTypeSymbols.push_back(classType);
}

} } // namespace cminor::symbols
