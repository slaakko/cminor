// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <stdexcept>
#include <vector>

const char* version = "0.0.1";

void PrintHelp()
{
    std::cout <<
        "Cminor programming language commander version " << version << "\n\n" <<
        "Usage: cminor [global-options]\n" <<
        "(      run [run-options] program[.cminora] [program-arguments]\n" <<
        "|      debug [debug-options] program[.cminora] [program-arguments]\n" <<
        "|      compile [compile-options] { solution[.cminors] | project[.cminorp] }\n" <<
        "|      dump [dump-options] assembly[.cminora]\n" <<
        ")\n" <<
        "---------------------------------------------------------------------\n" <<
        "global-options:\n" <<
        "   -v | --verbose : verbose output\n" <<
        "   -h | --help    : print this help\n" << 
        "---------------------------------------------------------------------\n" <<
        "cminor run [run-options] program[.cminora] [program-arguments]\n\n" <<
        "Run program.cminora with given program-arguments in virtual machine.\n" <<
        "run-options:\n" <<
        "   -s=SEGMENT-SIZE\n" <<
        "       SEGMENT-SIZE is the size of the garbage collected memory\n" <<
        "       segment in megabytes. The default is 250. SEGMENT-SIZE is\n" <<
        "       also the maximum size of single continuous object.\n" <<
        "---------------------------------------------------------------------\n" <<
        "cminor debug [debug-options] program[.cminora] [program-arguments]\n\n" <<
        "Debug program.cminora with given program arguments.\n"
        "debug-options:\n" <<
        "   -s=SEGMENT-SIZE\n" <<
        "       SEGMENT-SIZE is the size of the garbage collected memory\n" <<
        "       segment in megabytes. The default is 250. SEGMENT-SIZE is\n" <<
        "       also the maximum size of single continuous object.\n" <<
        "---------------------------------------------------------------------\n" <<
        "cminor compile [compile-options] { solution[.cminors] | project[.cminorp] }\n\n" <<
        "Compile each given solution.cminors and project.cminorp.\n" <<
        "compile-options:\n" <<
        "   -c=CONFIG\n" << 
        "       Use CONFIG configuration. CONFIG can be debug or release.\n" <<
        "       The default is debug.\n" <<
        "---------------------------------------------------------------------\n" <<
        "cminor dump [dump-options] assembly[.cminora]\n\n" <<
        "Dump information contained in assembly.cminora.\n" <<
        "---------------------------------------------------------------------\n" <<
        std::endl;
}

enum class State
{
    globalOptions, 
        runOptions, runProgram, programArguments,
        compileOptions, projectsAndSolutions,
        debugOptions,
        dumpOptions, dumpAssemblyName
};

int main(int argc, const char** argv)
{
    try
    {
        if (argc < 2)
        {
            PrintHelp();
            return 0;
        }
        State state = State::globalOptions;
        bool verbose = false;
        std::string command = "run";
        std::vector<std::string> runOptions;
        std::string runProgram;
        std::vector<std::string> programArguments;
        std::vector<std::string> compileOptions;
        std::vector<std::string> projectsAndSolutions;
        std::vector<std::string> debugOptions;
        std::vector<std::string> dumpOptions;
        std::string dumpAssemblyName;
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            switch (state)
            {
                case State::globalOptions:
                {
                    if (!arg.empty() && arg[0] == '-')
                    {
                        if (arg == "-v" || arg == "--verbose")
                        {
                            verbose = true;
                        }
                        else if (arg == "-h" || arg == "--help")
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
                        command = arg;
                        if (command == "run")
                        {
                            state = State::runOptions;
                        }
                        else if (command == "debug")
                        {
                            state = State::debugOptions;
                        }
                        else if (command == "compile")
                        {
                            state = State::compileOptions;
                        }
                        else if (command == "dump")
                        {
                            state = State::dumpOptions;
                        }
                        else
                        {
                            PrintHelp();
                            return 0;
                        }
                    }
                    break;
                }
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
