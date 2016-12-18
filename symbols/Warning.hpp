// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_WARNING_INCLUDED
#define CMINOR_SYMBOLS_WARNING_INCLUDED
#include <cminor/pl/Scanner.hpp>

namespace cminor { namespace symbols {

using cminor::parsing::Span;

class Warning
{
public:
    Warning(const std::string& project_, const std::string& message_);
    const std::string& Project() const { return project; }
    const std::string& Message() const { return message; }
    const Span& Defined() const { return defined; }
    const Span& Referenced() const { return referenced; }
    void SetDefined(const Span& defined_) { defined = defined_; }
    void SetReferenced(const Span& referenced_) { referenced = referenced_; }
private:
    std::string project;
    std::string message;
    Span defined;
    Span referenced;
};

class CompileWarningCollection
{
public:
    static void Init();
    static void Done();
    static CompileWarningCollection& Instance();
    void SetCurrentProjectName(const std::string& currentProjectName_);
    const std::string& GetCurrentProjectName() const { return currentProjectName; }
    void AddWarning(const Warning& warning);
    const std::vector<Warning>& Warnings() const { return warnings; }
private:
    static std::unique_ptr<CompileWarningCollection> instance;
    CompileWarningCollection();
    std::string currentProjectName;
    std::vector<Warning> warnings;
};

void InitWarning();
void DoneWarning();

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_WARNING_INCLUDED
