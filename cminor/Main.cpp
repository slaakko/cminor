// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cminor/machine/System.hpp>
#include <cminor/machine/TextUtils.hpp>

const char* version = "0.0.1";

void PrintHelp()
{
    std::cout <<
        "Cminor programming language commander version " << version << "\n\n" <<
        "Usage: cminor [global-options]\n" <<
        "(      run [run-options] program.cminora [program-arguments]\n" <<
        "|      debug [debug-options] program.cminora [program-arguments]\n" <<
        "|      compile [compile-options] { solution.cminors | project.cminorp }\n" <<
        "|      clean { solution.cminors | project.cminorp }\n" <<
        "|      dump [dump-options] assembly.cminora [outputfile]\n" <<
        ")\n" <<
        "---------------------------------------------------------------------\n" <<
        "global-options:\n" <<
        "   -v | --verbose : verbose output\n" <<
        "   -h | --help    : print this help\n" << 
        "---------------------------------------------------------------------\n" <<
        "cminor run [run-options] program.cminora [program-arguments]\n\n" <<
        "Run program.cminora with given program-arguments in virtual machine.\n" <<
        "run-options:\n" <<
        "   -s=SEGMENT-SIZE\n" <<
        "       SEGMENT-SIZE is the size of the garbage collected memory\n" <<
        "       segment in megabytes. The default is 16 MB. SEGMENT-SIZE is\n" <<
        "       also the maximum size of single continuous object.\n" <<
        "---------------------------------------------------------------------\n" <<
        "cminor debug [debug-options] program.cminora [program-arguments]\n\n" <<
        "Debug program.cminora with given program arguments.\n"
        "debug-options:\n" <<
        "   -s=SEGMENT-SIZE\n" <<
        "       SEGMENT-SIZE is the size of the garbage collected memory\n" <<
        "       segment in megabytes. The default is 16 MB. SEGMENT-SIZE is\n" <<
        "       also the maximum size of single continuous object.\n" <<
        "---------------------------------------------------------------------\n" <<
        "cminor compile [compile-options] { solution.cminors | project.cminorp }\n\n" <<
        "Compile each given solution.cminors and project.cminorp.\n" <<
        "compile-options:\n" <<
        "   -c=CONFIG\n" << 
        "       Use CONFIG configuration. CONFIG can be debug or release.\n" <<
        "       The default is debug.\n" <<
        "   -d | --dparse\n" <<
        "       Debug parsing to standard output\n"
        "   -cl | --clean\n" <<
        "       Clean given projects and solutions.\n" << 
        "---------------------------------------------------------------------\n" <<
        "cminor clean [clean-options] { solution.cminors | project.cminorp }\n\n" <<
        "clean-options:\n" <<
        "   -c=CONFIG\n" <<
        "       Use CONFIG configuration. CONFIG can be debug or release.\n" <<
        "       The default is debug.\n" <<
        "---------------------------------------------------------------------\n" <<
        "cminor dump [dump-options] assembly.cminora [outputfile]\n\n" <<
        "Dump information in assembly.cminora to standard output or given file.\n" <<
        "dump-options:\n" <<
        "   -f | --functions : dump functions\n" <<
        "   -s | --symbols   : dump symbols\n" <<
        "   -a | --all       : dump all (default)\n" <<
        "---------------------------------------------------------------------\n" <<
        std::endl;
}

enum class State
{
    globalOptions, 
    runOptions, runProgram, programArguments,
    compileOptions, projectsAndSolutions,
    cleanOptions,
    debugOptions,
    dumpOptions, dumpAssemblyName, dumpOutputFile, dumpEnd
};

using namespace cminor::machine;

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
        std::string command = "run";
        std::vector<std::string> runOptions;
        std::string runProgram;
        std::vector<std::string> programArguments;
        std::vector<std::string> compileOptions;
        std::vector<std::string> projectsAndSolutions;
        std::vector<std::string> globalOptions;
        std::vector<std::string> debugOptions;
        std::vector<std::string> dumpOptions;
        std::string dumpAssemblyName;
        std::string dumpOutputFile;
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
                            globalOptions.push_back(arg);
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
                        else if (command == "clean")
                        {
                            compileOptions.push_back("--clean");
                            state = State::cleanOptions;
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
                case State::runOptions:
                {
                    if (!arg.empty() && arg[0] == '-')
                    {
                        if (arg.find('=', 0) != std::string::npos)
                        {
                            std::vector<std::string> components = Split(arg, '=');
                            if (components.size() != 2)
                            {
                                throw std::runtime_error("invalid argument '" + arg + "'");
                            }
                            else
                            {
                                if (components[0] == "-s")
                                {
                                    runOptions.push_back(arg);
                                }
                                else
                                {
                                    throw std::runtime_error("unknown run option '" + arg + "'");
                                }
                            }
                        }
                    }
                    else
                    {
                        runProgram = arg;
                        state = State::programArguments;
                    }
                    break;
                }
                case State::programArguments:
                {
                    programArguments.push_back(arg);
                    break;
                }
                case State::dumpOptions:
                {
                    if (!arg.empty() && arg[0] == '-')
                    {
                        if (arg == "-f" || arg == "--functions")
                        {
                            dumpOptions.push_back(arg);
                        }
                        else if (arg == "-s" || arg == "--symbols")
                        {
                            dumpOptions.push_back(arg);
                        }
                        else if (arg == "-a" || arg == "--all")
                        {
                            dumpOptions.push_back(arg);
                        }
                        else
                        {
                            throw std::runtime_error("unknown dump option '" + arg + "'");
                        }
                    }
                    else
                    {
                        if (state == State::dumpOptions)
                        {
                            state = State::dumpAssemblyName;
                        }
                        if (state == State::dumpAssemblyName)
                        {
                            dumpAssemblyName = arg;
                            state = State::dumpOutputFile;
                        }
                        else if (state == State::dumpOutputFile)
                        {
                            dumpOutputFile = arg;
                            state = State::dumpEnd;
                        }
                        else if (state == State::dumpEnd)
                        {
                            throw std::runtime_error("unknown dump argument '" + arg + "'");
                        }
                    }
                    break;
                }
                case State::compileOptions:
                {
                    if (!arg.empty() && arg[0] == '-')
                    {
                        if (arg == "-d" || arg == "--dparse")
                        {
                            compileOptions.push_back(arg);
                        }
                        else if (arg == "-cl" || arg == "--clean")
                        {
                            compileOptions.push_back(arg);
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
                                        compileOptions.push_back(arg);
                                    }
                                    else if (components[1] == "debug")
                                    {
                                        compileOptions.push_back(arg);
                                    }
                                    else
                                    {
                                        throw std::runtime_error("unknown configuration argument '" + arg + "'");
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
                            throw std::runtime_error("unknown compile option '" + arg + "'");
                        }
                    }
                    else
                    {
                        projectsAndSolutions.push_back(arg);
                    }
                    break;
                }
                case State::cleanOptions:
                {
                    if (!arg.empty() && arg[0] == '-')
                    {
                        if (arg.find('=', 0) != std::string::npos)
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
                                        compileOptions.push_back(arg);
                                    }
                                    else if (components[1] == "debug")
                                    {
                                        compileOptions.push_back(arg);
                                    }
                                    else
                                    {
                                        throw std::runtime_error("unknown configuration argument '" + arg + "'");
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
                    break;
                }
            }
        }
        std::string commandLine;
        if (command == "dump")
        {
            if (state != State::dumpOutputFile && state != State::dumpEnd)
            {
                throw std::runtime_error("incomplete dump command: assembly filename missing");
            }
            commandLine = "cminordump";
            for (const std::string& globalOption : globalOptions)
            {
                commandLine.append(" ").append(globalOption);
            }
            for (const std::string& dumpOption : dumpOptions)
            {
                commandLine.append(" ").append(dumpOption);
            }
            commandLine.append(" ").append(QuotedPath(dumpAssemblyName));
            if (!dumpOutputFile.empty())
            {
                commandLine.append(" ").append(QuotedPath(dumpOutputFile));
            }
        }
        else if (command == "compile" || command == "clean")
        {
            if (projectsAndSolutions.empty())
            {
                throw std::runtime_error("incomplete compile/clean command: no project or solution given");
            }
            commandLine = "cminorc";
            for (const std::string& globalOption : globalOptions)
            {
                commandLine.append(" ").append(globalOption);
            }
            for (const std::string& compileOption : compileOptions)
            {
                commandLine.append(" ").append(compileOption);
            }
            for (const std::string& projectOrSolution : projectsAndSolutions)
            {
                commandLine.append(" ").append(QuotedPath(projectOrSolution));
            }
        }
        else if (command == "run")
        {
            if (runProgram.empty())
            {
                throw std::runtime_error("incomplete run command: program assembly missing");
            }
            commandLine = "cminorvm";
            for (const std::string& globalOption : globalOptions)
            {
                commandLine.append(" ").append(globalOption);
            }
            commandLine.append(" ").append(runProgram);
            for (const std::string& programArgument : programArguments)
            {
                commandLine.append(" ").append(programArgument);
            }
        }
        System(commandLine);
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
