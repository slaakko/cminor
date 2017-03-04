// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cminor/util/System.hpp>
#include <cminor/util/TextUtils.hpp>

const char* version = "0.1.0";

enum class HelpTopics : uint8_t
{
    none = 0,
    abstract_ = 1 << 0,
    build = 1 << 1,
    clean = 1 << 2,
    run = 1 << 3,
    debug = 1 << 4,
    dump = 1 << 5,
    all = abstract_ | build | clean | run | debug | dump
};

inline HelpTopics operator|(HelpTopics left, HelpTopics right)
{
    return HelpTopics(uint8_t(left) | uint8_t(right));
}

inline HelpTopics operator&(HelpTopics left, HelpTopics right)
{
    return HelpTopics(uint8_t(left) & uint8_t(right));
}

void PrintHelp(HelpTopics helpTopics)
{
    if (helpTopics == HelpTopics::none)
    {
        helpTopics = HelpTopics::all;
    }
    std::cout << "\n";
    if ((helpTopics & HelpTopics::abstract_) != HelpTopics::none)
    {
        std::cout <<
            "Cminor programming language commander version " << version << "\n\n" <<
            "Usage: cminor [global-options]\n" <<
            "(      build [build-options] { SOLUTION.cminors | PROJECT.cminorp } | system\n" <<
            "|      clean [clean-options] { SOLUTION.cminors | PROJECT.cminorp } | system\n" <<
            "|      run [run-options] PROGRAM.cminora [program-arguments]\n" <<
            "|      debug [debug-options] PROGRAM.cminora [program-arguments]\n" <<
            "|      dump [dump-options] ASSEMBLY.cminora [output-file]\n" <<
            ")\n" <<
            "---------------------------------------------------------------------\n" <<
            "global-options:\n\n" <<
            "   --verbose (-v)\n" <<
            "       Verbose output.\n" <<
            "   --help (-h) [abstract | build | clean | run | debug | dump]\n" <<
            "       Print help about specified topic(s).\n" <<
            "---------------------------------------------------------------------\n" <<
            std::endl;
    }
    if ((helpTopics & HelpTopics::build) != HelpTopics::none)
    {
        std::cout <<
            "cminor build [build-options] { SOLUTION.cminors | PROJECT.cminorp } | system\n\n" <<
            "Build each given SOLUTION.cminors and PROJECT.cminorp, or build system libraries.\n\n" <<
            "build-options:\n\n" <<
            "   --config=CONFIG (-c=CONFIG)\n" <<
            "       Use CONFIG configuration. CONFIG can be debug or release. Default is debug.\n" << 
            "   --clean (-e)\n" <<
            "       Clean given projects and solutions, or clean system libraries.\n" <<
            "   --debug (-d)\n" <<
            "       Run debug version of the compiler.\n" <<
            "   --debug-parse (-p)\n" <<
            "       Debug parsing to standard output.\n" <<
            "   --native (-n)\n" <<
            "       Compile to native object code. Generates DLLs in Windows, shared objects in Linux.\n" <<
            "   --optimization-level=LEVEL (-O=LEVEL)\n" <<
            "       Set optimization level to LEVEL=0-3 (used with --native).\n" <<
            "       Defaults:\n" <<
            "           0 for debug configuration\n" <<
            "           2 for release configuration\n" <<
            "   --inline-limit=LIMIT (-i=LIMIT)\n" <<
            "       Set inline limit to LIMIT intermediate instructions (used with --native).\n" <<
            "       Functions that have LIMIT or fewer intermediate instructions are chosen to be suitable for inlining.\n" <<
            "       Eventually LLVM will decide which functions to inline.\n" <<
            "       Defaults:\n" <<
            "           for optimization level 0: 0 (no inlining)\n" <<
            "           for optimization level 1: 0 (no inlining)\n" <<
            "           for optimization level 2: 8 instructions\n" <<
            "           for optimization level 3: 16 instructions\n" <<
            "   --debug-pass=VALUE (-s=VALUE)\n" <<
            "       Generate debug output for LLVM passes to stderr (used with --native).\n" <<
            "       VALUE can be Arguments, Structure, Executions or Details:\n" <<
            "           Arguments: print pass arguments to pass to 'opt'\n" <<
            "           Structure: print pass structure before run()\n" <<
            "           Executions: print pass name before it is executed\n" <<
            "           Details: print pass details when it is executed\n" <<
            "   --debug-llvm (-b)\n" <<
            "       Generate LLVM debug output to stderr (used with --native).\n" <<
            "   --list (-l)\n" <<
            "       Generate listing to ASSEMBLY_NAME.list (used with --native).\n" <<
            "   --emit-llvm (-m)\n" <<
            "       Emit LLVM intermediate code to ASSEMBLY_NAME.ll (used with --native).\n" <<
            "   --emit-opt-llvm (-t)\n" <<
            "       Emit optimized LLVM intermediate code to ASSEMBLY_NAME.opt.ll (used with --native).\n" <<
            "   --use-ms-link (-u)\n" <<
            "       Use Microsoft's link.exe as a linker if in path (used with --native).\n" << 
            "       Otherwise uses LLVM's lld-link as a linker.\n" <<
            "---------------------------------------------------------------------\n" <<
            std::endl;
    }
    if ((helpTopics & HelpTopics::clean) != HelpTopics::none)
    {
        std::cout <<
            "cminor clean [clean-options] { SOLUTION.cminors | PROJECT.cminorp } | system\n\n" <<
            "Clean each given SOLUTION.cminors and PROJECT.cminorp, or clean system libraries.\n\n" <<
            "clean-options:\n\n" <<
            "   --config=CONFIG (-c=CONFIG)\n" <<
            "       Use CONFIG configuration. CONFIG can be debug or release. Default is debug.\n" <<
            "---------------------------------------------------------------------\n" <<
            std::endl;
    }
    if ((helpTopics & HelpTopics::run) != HelpTopics::none)
    {
        std::cout <<
            "cminor run [run-options] PROGRAM.cminora [program-arguments]\n\n" <<
            "Run PROGRAM.cminora with given program-arguments in virtual machine.\n\n" <<
            "run-options:\n\n" <<
            "   --native (-n)" <<
            "       Run program built with --native option.\n" <<
            "   --trace (-t)\n" <<
            "       Trace execution of native program to stderr (used with --native).\n" <<
            "   --segment-size=SEGMENT-SIZE (-s=SEGMENT-SIZE)\n" <<
            "       SEGMENT-SIZE is the size of the garbage collected memory segment in megabytes.\n" <<
            "       Default is 16 MB.\n" <<
            "   --pool-threshold=POOL-THRESHOLD (-p=POOL-THRESHOLD)\n" <<
            "       POOL-THRESHOLD is the grow threshold of the managed memory pool in megabytes.\n" <<
            "       Default is 16 MB.\n" <<
            "---------------------------------------------------------------------\n" <<
            std::endl;
    }
    if ((helpTopics & HelpTopics::debug) != HelpTopics::none)
    {
        std::cout <<
            "cminor debug [debug-options] PROGRAM.cminora [program-arguments]\n\n" <<
            "Debug PROGRAM.cminora with given program-arguments.\n\n"
            "debug-options:\n\n" <<
            "   --segment-size=SEGMENT-SIZE (-s=SEGMENT-SIZE)\n" <<
            "       SEGMENT-SIZE is the size of the garbage collected memory segment in megabytes.\n" <<
            "       Default is 16 MB.\n" <<
            "   --pool-threshold=POOL-THRESHOLD (-p=POOL-THRESHOLD)\n" <<
            "       POOL-THRESHOLD is the grow threshold of the managed memory pool in megabytes.\n" <<
            "       Default is 16 MB.\n" <<
            "---------------------------------------------------------------------\n" <<
            std::endl;
    }
    if ((helpTopics & HelpTopics::dump) != HelpTopics::none)
    {
        std::cout <<
            "cminor dump [dump-options] ASSEMBLY.cminora [output-file]\n\n" <<
            "Dump information in ASSEMBLY.cminora to standard output or to given file.\n\n" <<
            "dump-options:\n\n" <<
            "   --all (-a)\n" <<
            "       Dump all (default).\n" <<
            "   --header (-e)\n" <<
            "       Dump assembly header.\n" <<
            "   --constants (-c)\n" <<
            "       Dump constant pool.\n" <<
            "   --functions (-f)\n" <<
            "       Dump function table.\n" <<
            "   --symbols (-s)\n" <<
            "       Dump symbol table.\n" <<
            "   --mappings (-m)\n" <<
            "       Dump assembly mappings.\n" <<
            "---------------------------------------------------------------------\n" <<
            std::endl;
    }
}

enum class State
{
    globalOptions, helpTopic,
    runOptions, runProgram, programArguments,
    buildOptions, projectsAndSolutions,
    cleanOptions,
    debugOptions,
    dumpOptions, dumpAssemblyName, dumpOutputFile, dumpEnd
};

using namespace cminor::util;

int main(int argc, const char** argv)
{
    try
    {
        if (argc < 2)
        {
            PrintHelp(HelpTopics::abstract_);
            return 0;
        }
        State state = State::globalOptions;
        std::string command = "help";
        HelpTopics helpTopics = HelpTopics::none;
        std::vector<std::string> runOptions;
        std::string runProgram;
        std::vector<std::string> programArguments;
        std::vector<std::string> buildOptions;
        std::vector<std::string> projectsAndSolutions;
        std::vector<std::string> globalOptions;
        std::vector<std::string> debugOptions;
        std::vector<std::string> dumpOptions;
        std::string dumpAssemblyName;
        std::string dumpOutputFile;
        bool debug = false;
        bool buildSystem = false;
        bool cleanSystem = false;
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
                            command = "help";
                            state = State::helpTopic;
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
                        else if (command == "build")
                        {
                            state = State::buildOptions;
                        }
                        else if (command == "clean")
                        {
                            buildOptions.push_back("--clean");
                            state = State::cleanOptions;
                        }
                        else if (command == "dump")
                        {
                            state = State::dumpOptions;
                        }
                        else
                        {
                            PrintHelp(HelpTopics::abstract_);
                            return 1;
                        }
                    }
                    break;
                }
                case State::helpTopic:
                {
                    if (arg == "abstract")
                    {
                        helpTopics = helpTopics | HelpTopics::abstract_;
                    }
                    else if (arg == "build")
                    {
                        helpTopics = helpTopics | HelpTopics::build;
                    }
                    else if (arg == "clean")
                    {
                        helpTopics = helpTopics | HelpTopics::clean;
                    }
                    else if (arg == "run")
                    {
                        helpTopics = helpTopics | HelpTopics::run;
                    }
                    else if (arg == "debug")
                    {
                        helpTopics = helpTopics | HelpTopics::debug;
                    }
                    else if (arg == "dump")
                    {
                        helpTopics = helpTopics | HelpTopics::dump;
                    }
                    else
                    {
                        helpTopics = HelpTopics::all;
                    }
                    break;
                }
                case State::runOptions:
                {
                    if (!arg.empty() && arg[0] == '-')
                    {
                        if (arg == "-v" || arg == "--verbose")
                        {
                            globalOptions.push_back(arg);
                        }
                        else if (arg == "-n" || arg == "--native")
                        {
                            runOptions.push_back(arg);
                        }
                        else if (arg == "-t" || arg == "--trace")
                        {
                            runOptions.push_back(arg);
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
                                if (components[0] == "-s" || components[0] == "--segment-size")
                                {
                                    runOptions.push_back(arg);
                                }
                                else if (components[0] == "-p" || components[0] == "--pool-threshold")
                                {
                                    runOptions.push_back(arg);
                                }
                                else
                                {
                                    throw std::runtime_error("unknown run option '" + arg + "'");
                                }
                            }
                        }
                        else
                        {
                            throw std::runtime_error("unknown run option '" + arg + "'");
                        }
                    }
                    else
                    {
                        runProgram = arg;
                        state = State::programArguments;
                    }
                    break;
                }
                case State::debugOptions:
                {
                    if (arg == "-v" || arg == "--verbose")
                    {
                        globalOptions.push_back(arg);
                    }
                    else if (!arg.empty() && arg[0] == '-')
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
                                if (components[0] == "-s" || components[0] == "--segment-size")
                                {
                                    debugOptions.push_back(arg);
                                }
                                else if (components[0] == "-p" || components[0] == "--pool-threshold")
                                {
                                    runOptions.push_back(arg);
                                }
                                else
                                {
                                    throw std::runtime_error("unknown run option '" + arg + "'");
                                }
                            }
                        }
                        else
                        {
                            throw std::runtime_error("unknown debug option '" + arg + "'");
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
                case State::dumpOptions: case State::dumpAssemblyName: case State::dumpOutputFile: case State::dumpEnd:
                {
                    if (state == State::dumpOptions)
                    {
                        if (!arg.empty() && arg[0] == '-')
                        {
                            if (arg == "-v" || arg == "--verbose")
                            {
                                globalOptions.push_back(arg);
                            }
                            else if (arg == "-a" || arg == "--all")
                            {
                                dumpOptions.push_back(arg);
                            }
                            else if (arg == "-e" || arg == "--header")
                            {
                                dumpOptions.push_back(arg);
                            }
                            else if (arg == "-c" || arg == "--constants")
                            {
                                dumpOptions.push_back(arg);
                            }
                            else if (arg == "-f" || arg == "--functions")
                            {
                                dumpOptions.push_back(arg);
                            }
                            else if (arg == "-s" || arg == "--symbols")
                            {
                                dumpOptions.push_back(arg);
                            }
                            else if (arg == "-m" || arg == "--mappings")
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
                            state = State::dumpAssemblyName;
                        }
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
                    break;
                }
                case State::buildOptions:
                {
                    if (!arg.empty() && arg[0] == '-')
                    {
                        if (arg == "-v" || arg == "--verbose")
                        {
                            globalOptions.push_back(arg);
                        }
                        else if (arg == "-e" || arg == "--clean")
                        {
                            buildOptions.push_back(arg);
                        }
                        else if (arg == "-d" || arg == "--debug")
                        {
                            debug = true;
                        }
                        else if (arg == "-p" || arg == "--debug-parse")
                        {
                            buildOptions.push_back(arg);
                        }
                        else if (arg == "-n" || arg == "--native")
                        {
                            buildOptions.push_back(arg);
                        }
                        else if (arg == "-l" || arg == "--list")
                        {
                            buildOptions.push_back(arg);
                        }
                        else if (arg == "-m" || arg == "--emit-llvm")
                        {
                            buildOptions.push_back(arg);
                        }
                        else if (arg == "-t" || arg == "--emit-opt-llvm")
                        {
                            buildOptions.push_back(arg);
                        }
                        else if (arg == "-b" || arg == "--debug-llvm")
                        {
                            debug = true;
                            buildOptions.push_back(arg);
                        }
                        else if (arg == "-u" || arg == "--use-ms-link")
                        {
                            buildOptions.push_back(arg);
                        }
                        else if (arg == "--link-with-debug-machine")
                        {
                            buildOptions.push_back(arg);
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
                                if (components[0] == "-c" || components[0] == "--config")
                                {
                                    if (components[1] == "release")
                                    {
                                        buildOptions.push_back(arg);
                                    }
                                    else if (components[1] == "debug")
                                    {
                                        buildOptions.push_back(arg);
                                    }
                                    else
                                    {
                                        throw std::runtime_error("unknown configuration argument '" + arg + "'");
                                    }
                                }
                                else if (components[0] == "-O" || components[0] == "--optimization-level")
                                {
                                    const std::string& levelStr = components[1];
                                    if (levelStr != "0" && levelStr != "1" && levelStr != "2" && levelStr != "3")
                                    {
                                        throw std::runtime_error("unknown optimization level " + levelStr);
                                    }
                                    else
                                    {
                                        buildOptions.push_back(arg);
                                    }
                                }
                                else if (components[0] == "-i" || components[0] == "--inline-limit")
                                {
                                    buildOptions.push_back(arg);
                                }
                                else if (components[0] == "-s" || components[0] == "--debug-pass")
                                {
                                    const std::string& value = components[1];
                                    if (value != "Arguments" &&  value != "Structure" && value != "Executions" && value != "Details")
                                    {
                                        throw std::runtime_error("unknown LLVM debug pass value " + value);
                                    }
                                    else
                                    {
                                        buildOptions.push_back(arg);
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
                            throw std::runtime_error("unknown build option '" + arg + "'");
                        }
                    }
                    else
                    {
                        if (arg == "system")
                        {
                            buildSystem = true;
                        }
                        else
                        {
                            projectsAndSolutions.push_back(arg);
                        }
                    }
                    break;
                }
                case State::cleanOptions:
                {
                    if (!arg.empty() && arg[0] == '-')
                    {
                        if (arg == "-v" || arg == "--verbose")
                        {
                            globalOptions.push_back(arg);
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
                                if (components[0] == "-c" || components[0] == "--config")
                                {
                                    if (components[1] == "release")
                                    {
                                        buildOptions.push_back(arg);
                                    }
                                    else if (components[1] == "debug")
                                    {
                                        buildOptions.push_back(arg);
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
                        if (arg == "system")
                        {
                            cleanSystem = true;
                        }
                        else
                        {
                            projectsAndSolutions.push_back(arg);
                        }
                    }
                    break;
                }
            }
        }
        std::string commandLine;
        if (command == "help")
        {
            if (helpTopics == HelpTopics::none)
            {
                helpTopics = HelpTopics::all;
            }
            PrintHelp(helpTopics);
            return 0;
        }
        else if (command == "dump")
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
        else if (command == "build" || command == "clean")
        {
            if (projectsAndSolutions.empty() && !buildSystem && !cleanSystem)
            {
                throw std::runtime_error("incomplete build/clean command: no project or solution given");
            }
            else if (buildSystem || cleanSystem)
            {
                if (!projectsAndSolutions.empty())
                {
                    throw std::runtime_error("system libraries cannot be built/cleaned at the same time as other projects or solutions");
                }
            }
            if (buildSystem || cleanSystem)
            {
                if (debug)
                {
                    commandLine = "cminorbuildsysd";
                }
                else
                {
                    commandLine = "cminorbuildsys";
                }
            }
            else
            {
                if (debug)
                {
                    commandLine = "cminorcd";
                }
                else
                {
                    commandLine = "cminorc";
                }
            }
            for (const std::string& globalOption : globalOptions)
            {
                commandLine.append(" ").append(globalOption);
            }
            for (const std::string& buildOption : buildOptions)
            {
                commandLine.append(" ").append(buildOption);
            }
            for (const std::string& projectOrSolution : projectsAndSolutions)
            {
                commandLine.append(" ").append(QuotedPath(projectOrSolution));
            }
        }
        else if (command == "run" || command == "debug")
        {
            if (runProgram.empty())
            {
                throw std::runtime_error("incomplete run/debug command: program assembly missing");
            }
            if (command == "run")
            {
                commandLine = "cminorvm";
            }
            else if (command == "debug")
            {
                commandLine = "cminordb";
            }
            for (const std::string& globalOption : globalOptions)
            {
                commandLine.append(" ").append(globalOption);
            }
            for (const std::string& runOption : runOptions)
            {
                commandLine.append(" ").append(runOption);
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
