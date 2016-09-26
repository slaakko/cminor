// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/machine/CodeFormatter.hpp>
#include <string>

namespace cminor { namespace machine {

void CodeFormatter::Write(const std::string& text)
{
    if (atBeginningOfLine)
    {
        if (indent != 0)
        {
            stream << std::string(indentSize * indent, ' ');
            atBeginningOfLine = false;
        }
    }
    stream << text;
}

void CodeFormatter::WriteLine(const std::string& text)
{
    Write(text);
    NewLine();
}

void CodeFormatter::NewLine()
{
    stream << "\n";
    atBeginningOfLine = true;
    ++line;
}

} } // namespace cminor::machine
