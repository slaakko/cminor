// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/SymbolWriter.hpp>

using namespace cminor::machine;
using namespace cminor::symbols;

struct InitDone
{
    InitDone()
    {
        InitSymbol();
    }
    ~InitDone()
    {
        DoneSymbol();
    }
};

int main()
{
    try
    {
        InitDone initDone;
        std::unique_ptr<Assembly> systemAssembly = CreateSystemAssembly();
        SymbolWriter writer(systemAssembly->FileName());
        systemAssembly->Write(writer);
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
