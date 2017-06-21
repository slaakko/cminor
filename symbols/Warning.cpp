// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/symbols/Warning.hpp>

namespace cminor { namespace symbols {

Warning::Warning(const std::u32string& project_, const std::string& message_) : project(project_), message(message_)
{
}

CompileWarningCollection::CompileWarningCollection()
{
}

void CompileWarningCollection::Init()
{
    instance.reset(new CompileWarningCollection());
}

void CompileWarningCollection::Done()
{
    instance.reset();
}

CompileWarningCollection& CompileWarningCollection::Instance()
{
    return *instance;
}

void CompileWarningCollection::SetCurrentProjectName(const std::u32string& currentProjectName_)
{
    currentProjectName = currentProjectName_;
}

std::unique_ptr<CompileWarningCollection> CompileWarningCollection::instance;

void CompileWarningCollection::AddWarning(const Warning& warning)
{
    warnings.push_back(warning);
}

void InitWarning()
{
    CompileWarningCollection::Init();
}

void DoneWarning()
{
    CompileWarningCollection::Done();
}

} } // namespace cminor::symbols
