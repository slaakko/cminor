// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/Assembly.hpp>

namespace cminor { namespace symbols {

SymbolReader::SymbolReader(const std::string& fileName_) : AstReader(fileName_), assembly(nullptr), readingClassTemplateSpecialization(false), classTemplateSpecializationNames(nullptr)
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
    assembly->GetSymbolTable().AddSymbol(symbol);
    if (ClassTemplateSpecializationSymbol* classTemplateSpecialization = dynamic_cast<ClassTemplateSpecializationSymbol*>(symbol))
    {
        classTemplateSpecializations.push_back(classTemplateSpecialization);
    }
    return symbol;
}

Value* SymbolReader::GetValue()
{
    ValueKind kind = static_cast<ValueKind>(GetByte());
    Value* value = ValueFactory::Instance().CreateValue(kind);
    value->Read(*this);
    return value;
}

void SymbolReader::EmplaceTypeRequest(Symbol* forSymbol, ConstantId typeNameId, int index)
{
    typeRequests.push_back(TypeRequest(forSymbol, typeNameId, index));
}

void SymbolReader::ProcessTypeRequests()
{
    int n = int(typeRequests.size());
    for (int i = 0; i < n; ++i)
    {
        const TypeRequest& typeRequest = typeRequests[i];
        Constant typeNameConstant = assembly->GetConstantPool().GetConstant(typeRequest.typeNameId);
        utf32_string typeName = typeNameConstant.Value().AsStringLiteral();
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

void SymbolReader::AddConversionFun(FunctionSymbol* conversion)
{
    conversions.push_back(conversion);
}

void SymbolReader::AddClassTemplateSpecialization(ClassTemplateSpecializationSymbol* classTemplateSpecialization)
{
    classTemplateSpecializations.push_back(classTemplateSpecialization);
}

void SymbolReader::BeginReadingClassTemplateSpecialization()
{
    readingClassTemplateSpecializationStack.push(readingClassTemplateSpecialization);
    readingClassTemplateSpecialization = true;
}

void SymbolReader::EndReadingClassTemplateSpecialization()
{
    readingClassTemplateSpecialization = readingClassTemplateSpecializationStack.top();
    readingClassTemplateSpecializationStack.pop();
}

void SymbolReader::BeginNamespace(StringPtr ns)
{
    currentNamespaceComponents.push_back(ns.Value());
}

void SymbolReader::EndNamespace()
{
    currentNamespaceComponents.pop_back();
}

utf32_string SymbolReader::MakeFullClassTemplateSpecializationName(const utf32_string& classTemplateSpecializationName)
{
    utf32_string currentNamespaceName;
    int n = int(currentNamespaceComponents.size());
    bool first = true;
    for (int i = 0; i < n; ++i)
    {
        const utf32_string& nsNameComponent = currentNamespaceComponents[i];
        if (!nsNameComponent.empty())
        {
            if (first)
            {
                first = false;
            }
            else
            {
                currentNamespaceName.append(1, U'.');
            }
            currentNamespaceName.append(nsNameComponent);
        }
    }
    utf32_string fullClassTemplateSpecializationName = currentNamespaceName;
    if (!fullClassTemplateSpecializationName.empty())
    {
        fullClassTemplateSpecializationName.append(1, U'.');
    }
    fullClassTemplateSpecializationName.append(classTemplateSpecializationName);
    return fullClassTemplateSpecializationName;
}

bool SymbolReader::FoundInClassTemplateSpecializationNames(const utf32_string& fullClassTemplateSpecializationName) const
{
    Assert(classTemplateSpecializationNames, "class template specilization names not set");
    bool found = classTemplateSpecializationNames->find(fullClassTemplateSpecializationName) != classTemplateSpecializationNames->cend();
    return found;
}

void SymbolReader::AddToClassTemplateSpecializationNames(const utf32_string& fullClassTemplateSpecializationName)
{
    Assert(classTemplateSpecializationNames, "class template specilization names not set");
    classTemplateSpecializationNames->insert(fullClassTemplateSpecializationName);
}

} } // namespace cminor::symbols
