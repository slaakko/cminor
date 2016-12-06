/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/pl/Scanner.hpp>
#include <cminor/pl/Parser.hpp>
#include <cminor/pl/Exception.hpp>
#include <cminor/pl/Rule.hpp>
#include <algorithm>
#include <cctype>

namespace cminor { namespace parsing {

std::string NarrowString(const char* start, const char* end)
{
    std::string s;
    s.reserve(end - start);
    while (start != end)
    {
        if (*start != '\r')
        {
            s.append(1, *start);
        }
        ++start;
    }
    return s;
}

bool countSourceLines = false;;
int numParsedSourceLines = 0;

void SetCountSourceLines(bool count)
{
    countSourceLines = count;
}

int GetParsedSourceLines()
{
    return numParsedSourceLines;
}

bool cc = false;

void SetCC(bool cc_)
{
    cc = cc_;
}

bool CC()
{
    return cc;
}

int CCDiff(const char* start, const char* end, char ccStart, char ccEnd)
{
    int ccs = 0;
    int cce = 0;
    int state = 0;
    while (start != end)
    {
        char c = *start;
        switch (state)
        {
            case 0:
            {
                if (c == ccStart)
                {
                    ++ccs;
                }
                else if (c == ccEnd)
                {
                    ++cce;
                }
                else if (c == '"')
                {
                    state = 1;
                }
                else if (c == '\'')
                {
                    state = 3;
                }
                break;
            }
            case 1:
            {
                if (c == '"')
                {
                    state = 0;
                }
                else if (c == '\\')
                {
                    state = 2;
                }
                break;
            }
            case 2:
            {
                state = 1;
                break;
            }
            case 3:
            {
                if (c == '\'')
                {
                    state = 0;
                }
                else if (c == '\\')
                {
                    state = 4;
                }
                break;
            }
            case 4:
            {
                state = 3;
                break;
            }
        }
        ++start;
    }
    return cce - ccs;
}

Scanner::Scanner(const char* start_, const char* end_, const std::string& fileName_, int fileIndex_, Parser* skipper_):
    start(start_), end(end_), skipper(skipper_), skipping(false), tokenCounter(0), fileName(fileName_), span(fileIndex_),
    log(nullptr), atBeginningOfLine(true), synchronizing(false), ccRule(nullptr)
{
    if (countSourceLines)
    {
        numParsedSourceLines = 0;
    }
}

void Scanner::operator++()
{
    char c = GetChar();
    if (countSourceLines)
    {
        if (!skipping && atBeginningOfLine && !std::isspace(c))
        {
            ++numParsedSourceLines;
            atBeginningOfLine = false;
        }
    }
    ++span;
    if (c == '\n')
    {
        atBeginningOfLine = true;
        span.IncLineNumber();
    }
    if (CC() && c == '`' && !synchronizing && ccRule != nullptr)
    {
        synchronizing = true;
        const char* s = start + span.Start();
        char ccStart = ccRule->CCStart();
        char ccEnd = ccRule->CCEnd();
        int ccDiff = CCDiff(start, end, ccStart, ccEnd);
        while (ccDiff > 0)
        {
            --ccDiff;
            ccRule->DecCCCount();
        }
        int count = 0;
        if (ccRule->CCSkip())
        {
            count = std::max(0, ccRule->CCCount() - 1);
        }
        int state = 0;
        bool stop = false;
        while (s != end && !stop)
        {
            char c = *s;
            switch (state)
            {
                case 0:
                {
                    if (c == ccStart)
                    {
                        ++count;
                    }
                    else if (c == ccEnd)
                    {
                        if (count == 0)
                        {
                            stop = true;
                        }
                        else
                        {
                            --count;
                        }
                    }
                    else if (c == '"')
                    {
                        state = 1;
                    }
                    else if (c == '\'')
                    {
                        state = 3;
                    }
                    break;
                }
                case 1:
                {
                    if (c == '"')
                    {
                        state = 0;
                    }
                    else if (c == '\\')
                    {
                        state = 2;
                    }
                    break;
                }
                case 2:
                {
                    state = 1;
                    break;
                }
                case 3:
                {
                    if (c == '\'')
                    {
                        state = 0;
                    }
                    else if (c == '\\')
                    {
                        state = 4;
                    }
                    break;
                }
                case 4:
                {
                    state = 3;
                    break;
                }
            }
            if (!stop)
            {
                ++s;
                ++span;
            }
        }
    }
}

void Scanner::Skip()
{
    if (tokenCounter == 0 && skipper)
    {
        Span save = span;
        BeginToken();
        skipping = true;
        ObjectStack stack;
        Match match = skipper->Parse(*this, stack);
        skipping = false;
        EndToken();
        if (!match.Hit())
        {
            span = save;
        }
    }
}

int Scanner::LineEndIndex()
{
    int lineEndIndex = span.Start();
    int contentLength = static_cast<int>(end - start);
    while (lineEndIndex < contentLength && (start[lineEndIndex] != '\r' && start[lineEndIndex] != '\n'))
    {
        ++lineEndIndex;
    }
    return lineEndIndex;
}

std::string Scanner::RestOfLine()
{
    std::string restOfLine(start + span.Start(), start + LineEndIndex());
    return restOfLine;
}

void Scanner::PushCCRule(Rule* ccRule_)
{
    ccRuleStack.push(ccRule);
    ccRule = ccRule_;
}

void Scanner::PopCCRule()
{
    ccRule = ccRuleStack.top();
    ccRuleStack.pop();
}


} } // namespace Cm::Parsing
