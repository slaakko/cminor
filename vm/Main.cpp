// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolReader.hpp>

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
        Machine machine;
        SymbolReader reader(machine, CminorSystemAssemblyFilePath());
        Assembly systemAssembly;
        systemAssembly.Read(reader);
        Assembly testAssembly(U"test", "file.cmna");
        testAssembly.GetSymbolTable().Import(systemAssembly.GetSymbolTable());
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}