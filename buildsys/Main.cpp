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
        ObjectTypeTable::Init();
        InitSymbol();
        InitAssembly();
        Cm::Parsing::Init();
    }
    ~InitDone()
    {
        Cm::Parsing::Done();
        DoneAssembly();
        DoneSymbol();
        ObjectTypeTable::Done();
        ClassDataTable::Done();
        FunctionTable::Done();
    }
};

void GenerateDefaultConstructors(Assembly& assembly, ClassTypeSymbol* classTypeSymbol)
{
    std::unique_ptr<BoundCompileUnit> boundCompileUnit(new BoundCompileUnit(assembly, nullptr));
    std::unique_ptr<BoundClass> boundClass(new BoundClass(classTypeSymbol));
    ConstructorSymbol* defaultConstructorSymbol = classTypeSymbol->DefaultConstructorSymbol();
    if (!classTypeSymbol->IsBound())
    {
        classTypeSymbol->SetBound();
        std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(defaultConstructorSymbol));
        boundFunction->SetBody(std::unique_ptr<BoundCompoundStatement>(new BoundCompoundStatement(assembly)));
        boundClass->AddMember(std::move(boundFunction));
    }
    boundCompileUnit->AddBoundNode(std::move(boundClass));
    GenerateCode(*boundCompileUnit, assembly.GetMachine());
}

int main()
{
    try
    {
        InitDone initDone;
        Machine machine;
        {
            std::unique_ptr<Assembly> systemCoreAssembly = CreateSystemCoreAssembly(machine, "debug");
            TypeSymbol* otype = systemCoreAssembly->GetSymbolTable().GetType(U"object");
            ClassTypeSymbol* objectType = dynamic_cast<ClassTypeSymbol*>(otype);
            GenerateDefaultConstructors(*systemCoreAssembly, objectType);
            TypeSymbol* stype = systemCoreAssembly->GetSymbolTable().GetType(U"string");
            ClassTypeSymbol* stringType = dynamic_cast<ClassTypeSymbol*>(stype);
            GenerateDefaultConstructors(*systemCoreAssembly, stringType);
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