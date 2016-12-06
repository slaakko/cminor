// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/symbols/Value.hpp>
#include <Cm.Util/Path.hpp>
#include <boost/filesystem.hpp>
#include <stdexcept>

using namespace cminor::machine;
using namespace cminor::symbols;
using namespace cminor::ast;
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
    }
    ~InitDone()
    {
        DoneAssembly();
        ValueDone();
        DoneSymbol();
        NodeDone();
        TypeDone();
        ClassDataTable::Done();
        FunctionTable::Done();
    }
};

const char* version = "0.0.1";

void PrintHelp()
{
    std::cout << "Cminor assembly dump version " << version << "\n\n" <<
        "Usage: cminordump [options] assembly.cminora [outputfile]\n" <<
        "Dump information in assembly.cminora.\n" <<
        "Options:\n" <<
        "-h | --help     : print this help message" <<
        std::endl;
}

int main(int argc, const char** argv)
{
    try
    {
        std::string assemblyFileName;
        std::string outputFileName;
        if (argc < 2)
        {
            PrintHelp();
            return 0;
        }
        InitDone initDone;
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (!arg.empty() && arg[0] == '-')
            {
                if (arg == "-h" || arg == "--help")
                {
                    PrintHelp();
                    return 0;
                }
                else
                {
                    throw std::runtime_error("unknown argument '" + arg + "'");
                }
            }
            else
            {
                if (assemblyFileName.empty())
                {
                    assemblyFileName = arg;
                }
                else
                {
                    outputFileName = arg;
                }
            }
        }
        if (assemblyFileName.empty())
        {
            throw std::runtime_error("no assembly file given");
        }
        boost::filesystem::path afp = assemblyFileName;
        if (afp.extension() != ".cminora")
        {
            throw std::runtime_error("argument '" + afp.generic_string() + "' has invalid extension (not .cminora)");
        }
        std::string assemblyFilePath = GetFullPath(afp.generic_string());
        if (!boost::filesystem::exists(afp))
        {
            throw std::runtime_error("assembly file '" + assemblyFilePath + "' not found");
        }
        Machine machine;
        Assembly assembly(machine);
        const Assembly* rootAssembly = &assembly;
        SymbolReader symbolReader(assemblyFilePath);
        std::vector<CallInst*> callInstructions;
        std::vector<TypeInstruction*> typeInstructions;
        std::vector<SetClassDataInst*> setClassDataInstructions;
        std::vector<ClassTypeSymbol*> classTypes;
        std::string currentAssemblyDir = GetFullPath(boost::filesystem::path(assemblyFilePath).remove_filename().generic_string());
        std::unordered_set<std::string> importSet;
        std::unordered_set<utf32_string> classTemplateSpecializationNames;
        assembly.Read(symbolReader, LoadType::execute, rootAssembly, currentAssemblyDir, importSet, callInstructions, typeInstructions, setClassDataInstructions, classTypes, 
            classTemplateSpecializationNames);
        Link(callInstructions, typeInstructions, setClassDataInstructions, classTypes);
        assembly.GetSymbolTable().MergeClassTemplateSpecializations();
        callInstructions.clear();
        typeInstructions.clear();
        setClassDataInstructions.clear();
        classTypes.clear();
        FunctionTable::Done();
        ClassDataTable::Done();
        TypeTable::Done();
        std::ostream* outputStream = &std::cout;
        std::ofstream outputFileStream;
        if (!outputFileName.empty())
        {
            outputFileStream.open(outputFileName);
            outputStream = &outputFileStream;
        }
        CodeFormatter codeFormatter(*outputStream);
        assembly.Dump(codeFormatter);
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
