// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/build/Build.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Util.hpp>
#include <cminor/machine/Class.hpp>
#include <Cm.Parsing/InitDone.hpp>
#include <Cm.Util/Path.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace cminor::build;
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
        ObjectTypeTable::Init();
        ClassDataTable::Init();
        FunctionTable::Init();
    }
};

const char* version = "0.0.1";

void PrintHelp()
{
    std::cout << 
        "Cminor compiler version " << version << "\n\n" <<
        "Usage: cminorc [options] { solution.cminors | project.cminorp }\n" <<
        "Compile given solutions and projects.\n" <<
        "Options:\n" <<
        "-v | --verbose : verbose output\n" << 
        "-h | --help    : print this help message\n" <<
        "-c=CONFIG\n" <<
        "   Use CONFIG configuration. CONFIG can be debug or release.\n" <<
        "   The default is debug.\n" <<
        std::endl;
}

int main(int argc, const char** argv)
{
    try
    {
        InitDone initDone;        
        std::vector<std::string> projectsAndSolutions;
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (!arg.empty() && arg[0] == '-')
            {
                if (arg == "-v" || arg == "--verbose")
                {
                    SetGlobalFlag(GlobalFlags::verbose);
                }
                else if (arg == "-h" || arg == "--help")
                {
                    PrintHelp();
                    return 0;
                }
                else if (arg.find('=', 0) != std::string::npos)
                {
                    std::vector<std::string> components = Split(arg, '=');
                    if (components.size() != 2)
                    {
                        throw std::runtime_error("invalid argument '" + arg + "'");
                    }
                    else
                    {
                        if (components[0] == "-c")
                        {
                            if (components[1] == "release")
                            {
                                SetGlobalFlag(GlobalFlags::release);
                            }
                            else if (components[1] != "debug")
                            {
                                throw std::runtime_error("invalid configuration argument '" + arg + "'");
                            }
                        }
                        else
                        {
                            throw std::runtime_error("unknown argument '" + arg + "'");
                        }
                    }
                }
                else
                {
                    throw std::runtime_error("unknown argument '" + arg + "'");
                }
            }
            else
            {
                projectsAndSolutions.push_back(arg);
            }
        }
        if (projectsAndSolutions.empty())
        {
            PrintHelp();
            return 0;
        }
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            std::cout << "Cminor compiler version " << version << std::endl;
        }
        for (const std::string& projectOrSolution : projectsAndSolutions)
        {
            boost::filesystem::path fp(projectOrSolution);
            if (fp.extension() == ".cminors")
            {
                if (!boost::filesystem::exists(fp))
                {
                    throw std::runtime_error("solution file '" + fp.generic_string() + " not found");
                }
                else
                {
                    BuildSolution(GetFullPath(fp.generic_string()));
                }
            }
            else if (fp.extension() == ".cminorp")
            {
                if (!boost::filesystem::exists(fp))
                {
                    throw std::runtime_error("project file '" + fp.generic_string() + " not found");
                }
                else
                {
                    std::set<AssemblyReferenceInfo> assemblyReferenceInfos;
                    BuildProject(GetFullPath(fp.generic_string()), assemblyReferenceInfos);
                }
            }
            else
            {
                throw std::runtime_error("Argument '" + fp.generic_string() + "' has invalid extension. Not Cminor solution (.cminors) or project (.cminorp) file.");
            }
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
