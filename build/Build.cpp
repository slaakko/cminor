// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/build/Build.hpp>
#include <cminor/build/NativeCompiler.hpp>
#include <cminor/parser/ProjectFile.hpp>
#include <cminor/parser/SolutionFile.hpp>
#include <cminor/parser/CompileUnit.hpp>
#include <cminor/binder/TypeBinderVisitor.hpp>
#include <cminor/binder/StatementBinderVisitor.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/binder/BoundFunction.hpp>
#include <cminor/binder/ControlFlowAnalyzer.hpp>
#include <cminor/emitter/Emitter.hpp>
#include <cminor/symbols/GlobalFlags.hpp>
#include <cminor/symbols/Assembly.hpp>
#include <cminor/symbols/SymbolWriter.hpp>
#include <cminor/symbols/SymbolCreatorVisitor.hpp>
#include <cminor/symbols/MappingSymbolVisitor.hpp>
#include <cminor/symbols/SymbolReader.hpp>
#include <cminor/symbols/Warning.hpp>
#include <cminor/machine/FileRegistry.hpp>
#include <cminor/util/MappedInputFile.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/util/Path.hpp>
#include <iostream>

namespace cminor { namespace build {

using namespace cminor::parser;
using namespace cminor::ast;
using namespace cminor::symbols;
using namespace cminor::binder;
using namespace cminor::emitter;
using namespace cminor::machine;

CompileUnitGrammar* compileUnitGrammar = nullptr;

std::vector<std::unique_ptr<CompileUnitNode>> ParseSources(const std::vector<std::string>& sourceFilePaths)
{
    if (!compileUnitGrammar)
    {
        compileUnitGrammar = CompileUnitGrammar::Create();
    }
    std::vector<std::unique_ptr<CompileUnitNode>> compileUnits;
    for (const std::string& sourceFilePath : sourceFilePaths)
    {
        MappedInputFile sourceFile(sourceFilePath);
        int fileIndex = FileRegistry::RegisterParsedFile(sourceFilePath);
        ParsingContext parsingContext;
        if (GetGlobalFlag(GlobalFlags::debugParsing))
        {
            compileUnitGrammar->SetLog(&std::cout);
        }
        std::unique_ptr<CompileUnitNode> compileUnit(compileUnitGrammar->Parse(sourceFile.Begin(), sourceFile.End(), fileIndex, sourceFilePath, &parsingContext));
        compileUnits.push_back(std::move(compileUnit));
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::string s;
        if (sourceFilePaths.size() != 1)
        {
            s = "s";
        }
        std::cout << sourceFilePaths.size() << " source file" << s << " parsed" << std::endl;
    }
    return compileUnits;
}

void BuildSymbolTable(Assembly& assembly, const std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits)
{
    SymbolCreatorVisitor symbolCreatorVisitor(assembly);
    for (const std::unique_ptr<CompileUnitNode>& compileUnit : compileUnits)
    {
        compileUnit->Accept(symbolCreatorVisitor);
    }
}

std::vector<std::unique_ptr<BoundCompileUnit>> BindTypes(Assembly& assembly, const std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits)
{
    std::vector<std::unique_ptr<BoundCompileUnit>> boundCompileUnits;
    for (const std::unique_ptr<CompileUnitNode>& compileUnit : compileUnits)
    {
        std::unique_ptr<BoundCompileUnit> boundCompileUnit(new BoundCompileUnit(assembly, compileUnit.get()));
        TypeBinderVisitor typeBinderVisitor(*boundCompileUnit);
        compileUnit->Accept(typeBinderVisitor);
        boundCompileUnits.push_back(std::move(boundCompileUnit));
    }
    return boundCompileUnits;
}

void BindStatements(BoundCompileUnit& boundCompileUnit)
{
    StatementBinderVisitor statementBinderVisitor(boundCompileUnit);
    boundCompileUnit.GetCompileUnitNode()->Accept(statementBinderVisitor);
}

void CopyFile(const std::string& from, const std::string& to, std::unordered_set<std::string>& copied)
{
    if (copied.find(from) == copied.cend())
    {
        copied.insert(from);
        if (from != to)
        {
            if (boost::filesystem::exists(to))
            {
                boost::filesystem::remove(to);
            }
            boost::filesystem::copy(from, to);
        }
    }
}

BoundCompoundStatement* CreateBodyForArrayGetEnumeratorMemberFunctionSymbol(BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ArrayTypeSymbol* arrayType,
    ArrayGetEnumeratorMemberFunctionSymbol* getEnumeratorMemberFunctionSymbol)
{
    const Span& span = getEnumeratorMemberFunctionSymbol->GetSpan();
    CompoundStatementNode bodyNode(span);
    TemplateIdNode* arrayEnumeratorTemplateId = new TemplateIdNode(span, new IdentifierNode(span, "System.ArrayEnumerator"));
    arrayEnumeratorTemplateId->AddTemplateArgument(new IdentifierNode(span, ToUtf8(arrayType->ElementType()->FullName())));
    NewNode* newNode = new NewNode(span, arrayEnumeratorTemplateId);
    newNode->AddArgument(new ThisNode(span));
    bodyNode.AddStatement(new ReturnStatementNode(span, newNode));
    boundCompileUnit.GetAssembly().GetSymbolTable().BeginContainer(arrayType);
    SymbolCreatorVisitor symbolCreatorVisitor(boundCompileUnit.GetAssembly());
    bodyNode.Accept(symbolCreatorVisitor);
    boundCompileUnit.GetAssembly().GetSymbolTable().EndContainer();
    TypeBinderVisitor typeBinder(boundCompileUnit);
    typeBinder.SetContainerScope(getEnumeratorMemberFunctionSymbol->GetContainerScope());
    bodyNode.Accept(typeBinder);
    StatementBinderVisitor statementBinder(boundCompileUnit);
    statementBinder.SetBoundFunction(boundFunction);
    bodyNode.Accept(statementBinder);
    BoundStatement* statement = statementBinder.ReleaseStatement();
    BoundCompoundStatement* compoundStatement = dynamic_cast<BoundCompoundStatement*>(statement);
    Assert(compoundStatement, "compound statement expected");
    return compoundStatement;
}

void GenerateCodeForCreatedArrays(Assembly& assembly, std::unordered_set<ClassTemplateSpecializationSymbol*>& classTemplateSpecializations)
{
    BoundCompileUnit synthesizedCompileUnit(assembly, nullptr);
    for (ArrayTypeSymbol* arrayType : assembly.GetSymbolTable().CreatedArrays())
    {
        arrayType->InitVmt();
        arrayType->InitImts();
        for (ConstructorSymbol* ctor : arrayType->Constructors())
        {
            std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(ctor));
            synthesizedCompileUnit.AddBoundNode(std::move(boundFunction));
        }
        for (MemberFunctionSymbol* memFun : arrayType->MemberFunctions())
        {
            std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(memFun));
            if (ArrayGetEnumeratorMemberFunctionSymbol* getEnumeratorMemFun = dynamic_cast<ArrayGetEnumeratorMemberFunctionSymbol*>(memFun))
            {
                boundFunction->SetBody(std::unique_ptr<BoundCompoundStatement>(CreateBodyForArrayGetEnumeratorMemberFunctionSymbol(synthesizedCompileUnit, boundFunction.get(), arrayType, 
                    getEnumeratorMemFun)));
            }
            synthesizedCompileUnit.AddBoundNode(std::move(boundFunction));
        }
    }
    GenerateCode(synthesizedCompileUnit, assembly.GetMachine());
    const std::unordered_set<ClassTemplateSpecializationSymbol*>& specializations = synthesizedCompileUnit.GetClassTemplateRepository().ClassTemplateSpecializations();
    for (ClassTemplateSpecializationSymbol* specialization : specializations)
    {
        classTemplateSpecializations.insert(specialization);
    }
}

void GenerateCodeForClassTemplateSpecializations(Assembly& assembly, std::unordered_set<ClassTemplateSpecializationSymbol*>&& classTemplateSpecializations)
{
    BoundCompileUnit synthesizedCompileUnit(assembly, nullptr);
    synthesizedCompileUnit.AddFileScope(std::unique_ptr<FileScope>(new FileScope()));
    std::unordered_set<ClassTemplateSpecializationSymbol*> specializations = std::move(classTemplateSpecializations);
    while (!specializations.empty())
    {
        for (ClassTemplateSpecializationSymbol* specialization : specializations)
        {
            if (!specialization->HasGlobalNs())
            {
                specialization->ReadGlobalNs();
            }
            NamespaceNode* globalNs = specialization->GlobalNs();
            MappingSymbolVisitor mappingSymbolVisitor(assembly, *specialization->GetAssembly());
            mappingSymbolVisitor.SetImplementedInterfaces(&specialization->ImplementedInterfaces());
            globalNs->Accept(mappingSymbolVisitor);
            specialization->ResetFlag(SymbolFlags::bound);
            TypeBinderVisitor typeBinderVisitor(synthesizedCompileUnit);
            typeBinderVisitor.SetInstantiateRequested();
            globalNs->Accept(typeBinderVisitor);
            StatementBinderVisitor statementBinderVisitor(synthesizedCompileUnit);
            statementBinderVisitor.SetInstantiateRequested();
            globalNs->Accept(statementBinderVisitor);
            if (!specialization->IsProject())
            {
                specialization->SetReopened();
                specialization->SetProject();
                specialization->InitVmt();
                specialization->InitImts();
            }
        }
        specializations.clear();
        for (ClassTemplateSpecializationSymbol* specialization : synthesizedCompileUnit.GetClassTemplateRepository().ClassTemplateSpecializations())
        {
            specializations.insert(specialization);
        }
        synthesizedCompileUnit.GetClassTemplateRepository().Clear();
    }
    GenerateCode(synthesizedCompileUnit, assembly.GetMachine());
}

void CheckValidityOfMainFunction(Target target, Assembly& assembly)
{
    if (target != Target::program) return;
    FunctionSymbol* mainFunctionSymbol = assembly.GetSymbolTable().GetMainFunction();
    if (!mainFunctionSymbol)
    {
        throw Exception("program has no main function", Span());
    }
    Assert(mainFunctionSymbol->ReturnType(), "main function has no return type");
    if (!mainFunctionSymbol->ReturnType()->IsVoidType() && mainFunctionSymbol->ReturnType() != assembly.GetSymbolTable().GetType(U"System.Int32"))
    {
        throw Exception("main function should either be void or return an int", mainFunctionSymbol->GetSpan());
    }
    if (mainFunctionSymbol->Arity() != 0)
    {
        bool valid = true;
        if (mainFunctionSymbol->Arity() != 1)
        {
            valid = false;
        }
        else
        {
            TypeSymbol* paramType = mainFunctionSymbol->Parameters()[0]->GetType();
            ArrayTypeSymbol* arrayType = dynamic_cast<ArrayTypeSymbol*>(paramType);
            if (arrayType)
            {
                TypeSymbol* elementType = arrayType->ElementType();
                if (elementType->FullName() != U"System.String")
                {
                    valid = false;
                }
            }
            else
            {
                valid = false;
            }
        }
        if (!valid)
        {
            throw Exception("main function should either have no parameters or have one string[] parameter", mainFunctionSymbol->GetSpan());
        }
    }
    mainFunctionSymbol->SetExported();
}

void GetAssemblyReferenceClosureFor(Assembly* referencedAssembly, std::set<AssemblyReferenceInfo>& assemblyReferenceInfos)
{
    std::string assemblyFilePath = referencedAssembly->OriginalFilePath();
    if (!boost::filesystem::exists(assemblyFilePath))
    {
        assemblyFilePath = referencedAssembly->FilePathReadFrom();
    }
    std::string nativeImportLibraryFilePath;
    std::string nativeSharedLibraryFilePath;
    if (referencedAssembly->IsNative())
    {
        nativeImportLibraryFilePath = Path::Combine(Path::GetDirectoryName(assemblyFilePath), referencedAssembly->NativeImportLibraryFileName());
        nativeSharedLibraryFilePath = Path::Combine(Path::GetDirectoryName(assemblyFilePath), referencedAssembly->NativeSharedLibraryFileName());
    }
    assemblyReferenceInfos.insert(AssemblyReferenceInfo(assemblyFilePath, nativeImportLibraryFilePath, nativeSharedLibraryFilePath, referencedAssembly->IsSystemAssembly()));
    for (const std::unique_ptr<Assembly>& childReferencedAssembly : referencedAssembly->ReferencedAssemblies())
    {
        GetAssemblyReferenceClosureFor(childReferencedAssembly.get(), assemblyReferenceInfos);
    }
}

void CleanProject(Project* project)
{
    std::string config = GetConfig();
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Cleaning project '" << project->Name() << "' (" << project->FilePath() << ") using " << config << " configuration." << std::endl;
    }
    boost::filesystem::path afp = project->AssemblyFilePath();
    afp.remove_filename();
    boost::filesystem::remove_all(afp);
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Project '" << project->Name() << "' cleaned successfully." << std::endl;
    }
}

void BuildProject(Project* project, std::set<AssemblyReferenceInfo>& assemblyReferenceInfos, bool keepProjectBitForSymbols)
{
    FunctionTable::Init();
    ClassDataTable::Init();
    TypeInit();
    std::string config = GetConfig();
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Building project '" << project->Name() << "' (" << project->FilePath() << ") using " << config << " configuration." << std::endl;
    }
    CompileWarningCollection::Instance().SetCurrentProjectName(project->Name());
    std::vector<std::unique_ptr<CompileUnitNode>> compileUnits = ParseSources(project->SourceFilePaths());
    utf32_string assemblyName = ToUtf32(project->Name());
    Machine machine;
    AssemblyTable::Init();
    Assembly assembly(machine, assemblyName, project->AssemblyFilePath());
    assembly.PrepareForCompilation(project->AssemblyReferences());
    BuildSymbolTable(assembly, compileUnits);
    std::vector<std::unique_ptr<BoundCompileUnit>> boundCompileUnits = BindTypes(assembly, compileUnits);
    std::unordered_set<ClassTemplateSpecializationSymbol*> classTemplateSpecializations;
    for (std::unique_ptr<BoundCompileUnit>& boundCompileUnit : boundCompileUnits)
    {
        BindStatements(*boundCompileUnit);
        if (boundCompileUnit->HasGotos())
        {
            AnalyzeControlFlow(*boundCompileUnit);
        }
        GenerateCode(*boundCompileUnit, machine);
        const std::unordered_set<ClassTemplateSpecializationSymbol*>& specializations = boundCompileUnit->GetClassTemplateRepository().ClassTemplateSpecializations();
        for (ClassTemplateSpecializationSymbol* specialization : specializations)
        {
            classTemplateSpecializations.insert(specialization);
        }
        boundCompileUnit.reset();
    }
    GenerateCodeForCreatedArrays(assembly, classTemplateSpecializations);
    GenerateCodeForClassTemplateSpecializations(assembly, std::move(classTemplateSpecializations));
    CheckValidityOfMainFunction(project->GetTarget(), assembly);
    boost::filesystem::path obp(assembly.OriginalFilePath());
    obp.remove_filename();
    boost::filesystem::create_directories(obp);
    if (!CompileWarningCollection::Instance().Warnings().empty())
    {
        for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
        {
            std::string warningMessage = Expand(warning.Message(), warning.Defined(), warning.Referenced(), "Warning");
            std::cerr << warningMessage << std::endl;
        }
    }
    SymbolWriter writer(assembly.OriginalFilePath());
    writer.SetKeepProjectBitForSymbols(keepProjectBitForSymbols);
    assembly.Write(writer);
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Project '" << project->Name() << "' built successfully." << std::endl;
        std::cout << "=> " << assembly.OriginalFilePath() << std::endl;
    }
    if (assembly.IsSystemAssembly())
    {
        project->SetSystemProject();
    }
    for (const std::unique_ptr<Assembly>& referencedAssembly : assembly.ReferencedAssemblies())
    {
        GetAssemblyReferenceClosureFor(referencedAssembly.get(), assemblyReferenceInfos);
    }
    bool buildingSystemProject = project->IsSystemProject();
    boost::filesystem::path projectAssemblyDir = project->BasePath();
    projectAssemblyDir /= "assembly";
    projectAssemblyDir /= config;
    std::unordered_set<std::string> copied;
    for (const AssemblyReferenceInfo& assemblyReferenceInfo : assemblyReferenceInfos)
    {
        if (buildingSystemProject)
        {
            for (const AssemblyReferenceInfo& assemblyReferenceInfo : assemblyReferenceInfos)
            {
                boost::filesystem::path afp = assemblyReferenceInfo.assemblyFilePath;
                boost::filesystem::path pafp = projectAssemblyDir;
                pafp /= afp.filename();
                std::string from = GetFullPath(afp.generic_string());
                std::string to = GetFullPath(pafp.generic_string());
                CopyFile(from, to, copied);
                if (!assemblyReferenceInfo.nativeImportLibraryFilePath.empty())
                {
                    boost::filesystem::path ifp = assemblyReferenceInfo.nativeImportLibraryFilePath;
                    boost::filesystem::path pifp = projectAssemblyDir;
                    pifp /= ifp.filename();
                    std::string from = GetFullPath(ifp.generic_string());
                    std::string to = GetFullPath(pifp.generic_string());
                    CopyFile(from, to, copied);
                }
                if (!assemblyReferenceInfo.nativeSharedLibraryFilePath.empty())
                {
                    boost::filesystem::path sfp = assemblyReferenceInfo.nativeSharedLibraryFilePath;
                    boost::filesystem::path psfp = projectAssemblyDir;
                    psfp /= sfp.filename();
                    std::string from = GetFullPath(sfp.generic_string());
                    std::string to = GetFullPath(psfp.generic_string());
                    CopyFile(from, to, copied);
                }
            }
        }
        else
        {
            if (!assemblyReferenceInfo.isSystemAssembly)
            {
                boost::filesystem::path afp = assemblyReferenceInfo.assemblyFilePath;
                boost::filesystem::path pafp = projectAssemblyDir;
                pafp /= afp.filename();
                std::string from = GetFullPath(afp.generic_string());
                std::string to = GetFullPath(pafp.generic_string());
                CopyFile(from, to, copied);
                if (!assemblyReferenceInfo.nativeImportLibraryFilePath.empty())
                {
                    boost::filesystem::path ifp = assemblyReferenceInfo.nativeImportLibraryFilePath;
                    boost::filesystem::path pifp = projectAssemblyDir;
                    pifp /= ifp.filename();
                    std::string from = GetFullPath(ifp.generic_string());
                    std::string to = GetFullPath(pifp.generic_string());
                    CopyFile(from, to, copied);
                }
                if (!assemblyReferenceInfo.nativeSharedLibraryFilePath.empty())
                {
                    boost::filesystem::path sfp = assemblyReferenceInfo.nativeSharedLibraryFilePath;
                    boost::filesystem::path psfp = projectAssemblyDir;
                    psfp /= sfp.filename();
                    std::string from = GetFullPath(sfp.generic_string());
                    std::string to = GetFullPath(psfp.generic_string());
                    CopyFile(from, to, copied);
                }
            }
        }
    }
    AssemblyTable::Done();
    TypeDone();
    ClassDataTable::Done();
    FunctionTable::Done();
}

void BuildProject(Project* project, std::set<AssemblyReferenceInfo>& assemblyReferenceInfos)
{
    bool keepProjectBitForSymbols = false;
    BuildProject(project, assemblyReferenceInfos, keepProjectBitForSymbols);
}

void BuildNativeProject(Project* project, std::set<AssemblyReferenceInfo>& assemblyReferenceInfos)
{
    bool keepProjectBitForSymbols = true;
    BuildProject(project, assemblyReferenceInfos, keepProjectBitForSymbols);
    NativeCompiler nativeCompiler;
    std::string nativeImportLibraryFilePath;
    std::string nativeSharedLibraryFilePath;
    nativeCompiler.Compile(project->AssemblyFilePath(), nativeImportLibraryFilePath, nativeSharedLibraryFilePath);
    project->SetNativeImportLibraryFilePath(nativeImportLibraryFilePath);
    project->SetNativeSharedLibraryFilePath(nativeSharedLibraryFilePath);
}

ProjectGrammar* projectGrammar = nullptr;

void BuildProject(const std::string& projectFilePath, std::set<AssemblyReferenceInfo>& assemblyReferenceInfos)
{
    if (!projectGrammar)
    {
        projectGrammar = ProjectGrammar::Create();
    }
    std::string config = GetConfig();
    MappedInputFile projectFile(projectFilePath);
    std::unique_ptr<Project> project(projectGrammar->Parse(projectFile.Begin(), projectFile.End(), 0, projectFilePath, config));
    project->ResolveDeclarations();
    if (GetGlobalFlag(GlobalFlags::clean))
    {
        CleanProject(project.get());
    }
    else
    {
        if (GetGlobalFlag(GlobalFlags::native))
        {
            BuildNativeProject(project.get(), assemblyReferenceInfos);
        }
        else
        {
            BuildProject(project.get(), assemblyReferenceInfos);
        }
    }
}

SolutionGrammar* solutionGrammar = nullptr;

void BuildSolution(const std::string& solutionFilePath)
{
    if (!solutionGrammar)
    {
        solutionGrammar = SolutionGrammar::Create();
    }
    if (!projectGrammar)
    {
        projectGrammar = ProjectGrammar::Create();
    }
    MappedInputFile solutionFile(solutionFilePath);
    std::unique_ptr<Solution> solution(solutionGrammar->Parse(solutionFile.Begin(), solutionFile.End(), 0, solutionFilePath));
    solution->ResolveDeclarations();
    std::string config = GetConfig();
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        if (GetGlobalFlag(GlobalFlags::clean))
        {
            std::cout << "Cleaning solution '" << solution->Name() << "' (" << solution->FilePath() << ") using " << config << " configuration." << std::endl;
        }
        else
        {
            std::cout << "Building solution '" << solution->Name() << "' (" << solution->FilePath() << ") using " << config << " configuration." << std::endl;
        }
    }
    for (const std::string& projectFilePath : solution->ProjectFilePaths())
    {
        MappedInputFile projectFile(projectFilePath);
        std::unique_ptr<Project> project(projectGrammar->Parse(projectFile.Begin(), projectFile.End(), 0, projectFilePath, config));
        project->ResolveDeclarations();
        solution->AddProject(std::move(project));
    }
    std::vector<Project*> buildOrder = solution->CreateBuildOrder();
    std::set<AssemblyReferenceInfo> assemblyReferenceInfos;
    bool buildingSystemProjects = false;
    for (Project* project : buildOrder)
    {
        if (GetGlobalFlag(GlobalFlags::clean))
        {
            CleanProject(project);
        }
        else
        {
            if (GetGlobalFlag(GlobalFlags::native))
            {
                BuildNativeProject(project, assemblyReferenceInfos);
            }
            else
            {
                BuildProject(project, assemblyReferenceInfos);
            }
            if (project->IsSystemProject())
            {
                buildingSystemProjects = true;
            }
        }
    }
    boost::filesystem::path solutionAssemblyDir = solution->BasePath();
    solutionAssemblyDir /= "assembly";
    solutionAssemblyDir /= config;
    if (GetGlobalFlag(GlobalFlags::clean))
    {
        boost::filesystem::remove_all(solutionAssemblyDir);
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            std::cout << "Solution '" << solution->Name() << "' cleaned successfully." << std::endl;
        }
        return;
    }
    boost::filesystem::create_directories(solutionAssemblyDir);
    std::unordered_set<std::string> copied;
    if (buildingSystemProjects)
    {
        for (const AssemblyReferenceInfo& assemblyReferenceInfo : assemblyReferenceInfos)
        {
            boost::filesystem::path afp = assemblyReferenceInfo.assemblyFilePath;
            boost::filesystem::path safp = solutionAssemblyDir;
            safp /= afp.filename();
            std::string from = GetFullPath(afp.generic_string());
            std::string to = GetFullPath(safp.generic_string());
            CopyFile(from, to, copied);
            if (!assemblyReferenceInfo.nativeImportLibraryFilePath.empty())
            {
                boost::filesystem::path ifp = assemblyReferenceInfo.nativeImportLibraryFilePath;
                boost::filesystem::path sifp = solutionAssemblyDir;
                sifp /= ifp.filename();
                std::string from = GetFullPath(ifp.generic_string());
                std::string to = GetFullPath(sifp.generic_string());
                CopyFile(from, to, copied);
            }
            if (!assemblyReferenceInfo.nativeSharedLibraryFilePath.empty())
            {
                boost::filesystem::path sfp = assemblyReferenceInfo.nativeSharedLibraryFilePath;
                boost::filesystem::path ssfp = solutionAssemblyDir;
                ssfp /= sfp.filename();
                std::string from = GetFullPath(sfp.generic_string());
                std::string to = GetFullPath(ssfp.generic_string());
                CopyFile(from, to, copied);
            }
        }
    }
    else
    {
        for (const AssemblyReferenceInfo& assemblyReferenceInfo : assemblyReferenceInfos)
        {
            if (!assemblyReferenceInfo.isSystemAssembly)
            {
                boost::filesystem::path afp = assemblyReferenceInfo.assemblyFilePath;
                boost::filesystem::path safp = solutionAssemblyDir;
                safp /= afp.filename();
                std::string from = GetFullPath(afp.generic_string());
                std::string to = GetFullPath(safp.generic_string());
                CopyFile(from, to, copied);
                if (!assemblyReferenceInfo.nativeImportLibraryFilePath.empty())
                {
                    boost::filesystem::path ifp = assemblyReferenceInfo.nativeImportLibraryFilePath;
                    boost::filesystem::path sifp = solutionAssemblyDir;
                    sifp /= ifp.filename();
                    std::string from = GetFullPath(ifp.generic_string());
                    std::string to = GetFullPath(sifp.generic_string());
                    CopyFile(from, to, copied);
                }
                if (!assemblyReferenceInfo.nativeSharedLibraryFilePath.empty())
                {
                    boost::filesystem::path sfp = assemblyReferenceInfo.nativeSharedLibraryFilePath;
                    boost::filesystem::path ssfp = solutionAssemblyDir;
                    ssfp /= sfp.filename();
                    std::string from = GetFullPath(sfp.generic_string());
                    std::string to = GetFullPath(ssfp.generic_string());
                    CopyFile(from, to, copied);
                }
            }
        }
    }
    for (Project* project : buildOrder)
    {
        boost::filesystem::path afp = project->AssemblyFilePath();
        boost::filesystem::path safp = solutionAssemblyDir;
        safp /= afp.filename();
        std::string from = GetFullPath(afp.generic_string());
        std::string to = GetFullPath(safp.generic_string());
        CopyFile(from, to, copied);
        if (!project->NativeImportLibraryFilePath().empty())
        {
            boost::filesystem::path ifp = project->NativeImportLibraryFilePath();
            boost::filesystem::path sifp = solutionAssemblyDir;
            sifp /= ifp.filename();
            std::string from = GetFullPath(ifp.generic_string());
            std::string to = GetFullPath(sifp.generic_string());
            CopyFile(from, to, copied);
        }
        if (!project->NativeSharedLibraryFilePath().empty())
        {
            boost::filesystem::path sfp = project->NativeSharedLibraryFilePath();
            boost::filesystem::path ssfp = solutionAssemblyDir;
            ssfp /= sfp.filename();
            std::string from = GetFullPath(sfp.generic_string());
            std::string to = GetFullPath(ssfp.generic_string());
            CopyFile(from, to, copied);
        }
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Solution '" << solution->Name() << "' build successfully." << std::endl;
        std::cout << "=> " << GetFullPath(solutionAssemblyDir.generic_string()) << std::endl;
    }
}

} } // namespace cminor::build
