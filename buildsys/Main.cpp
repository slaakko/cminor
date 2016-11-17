// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <cminor/build/Build.hpp>
#include <cminor/emitter/Emitter.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/BoundClass.hpp>
#include <cminor/binder/BoundFunction.hpp>
#include <cminor/binder/TypeBinderVisitor.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/Value.hpp>
#include <cminor/ast/Project.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/machine/Class.hpp>
#include <Cm.Parsing/InitDone.hpp>
#include <Cm.Util/Path.hpp>
#include <boost/filesystem.hpp>

using namespace cminor::build;
using namespace cminor::emitter;
using namespace cminor::binder;
using namespace cminor::symbols;
using namespace cminor::machine;
using namespace Cm::Util;

struct InitDone
{
    InitDone()
    {
        FileRegistry::Init();
        FunctionTable::Init();
        ClassDataTable::Init();
        TypeInit();
        NodeInit();
        InitSymbol();
        ValueInit();
        InitAssembly();
        Cm::Parsing::Init();
    }
    ~InitDone()
    {
        Cm::Parsing::Done();
        DoneAssembly();
        ValueDone();
        DoneSymbol();
        NodeDone();
        TypeDone();
        ClassDataTable::Done();
        FunctionTable::Done();
    }
};

int main()
{
    try
    {
        InitDone initDone;
        Machine machine;
        {
            std::unique_ptr<Assembly> systemCoreAssembly = CreateSystemCoreAssembly(machine, "debug");
            boost::filesystem::path obp(systemCoreAssembly->FilePath());
            obp.remove_filename();
            boost::filesystem::create_directories(obp);
            SymbolWriter writer(systemCoreAssembly->FilePath());
            systemCoreAssembly->Write(writer);
        }
        boost::filesystem::path ssfp = CminorSystemDir();
        ssfp /= "System.cminors";
        std::string systemSolutionFilePath = GetFullPath(ssfp.generic_string());
        BuildSolution(systemSolutionFilePath);
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
