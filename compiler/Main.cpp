// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/build/Build.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/symbols/Symbol.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <Cm.Parsing/InitDone.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace cminor::build;
using namespace cminor::symbols;
using namespace cminor::machine;

struct InitDone
{
    InitDone()
    {
        FileRegistry::Init();
        InitSymbol();
        Cm::Parsing::Init();
    }
    ~InitDone()
    {
        Cm::Parsing::Done();
        DoneSymbol();
    }
};

const char* version = "1.0.0";

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
                if (arg == "-v")
                {
                    SetGlobalFlag(GlobalFlags::verbose);
                }
            }
            else
            {
                projectsAndSolutions.push_back(arg);
            }
        }
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            std::cout << "cminor compiler version " << version << std::endl;
        }
        for (const std::string& projectOrSolution : projectsAndSolutions)
        {
            boost::filesystem::path p(projectOrSolution);
            if (!p.has_extension())
            {
                p.replace_extension(".cmnp");
            }
            if (!boost::filesystem::exists(p))
            {
                throw std::runtime_error("project file '" + p.string() + "' not found");
            }
            if (p.extension() == ".cmnp")
            {
                BuildProject(p.generic_string());
            }
            else if (p.extension() == ".cmns")
            {
                // todo
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
