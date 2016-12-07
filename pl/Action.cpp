/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/pl/Action.hpp>
#include <cminor/pl/Visitor.hpp>
#include <cctype>

namespace cminor { namespace parsing {

ParsingAction::~ParsingAction()
{
}

FailureAction::~FailureAction()
{
}

PreCall::~PreCall()
{
}

PostCall::~PostCall()
{
}

ActionParser::ActionParser(const std::string& name_, Parser* child_):
    UnaryParser(name_, child_, child_->Info()), successCode(), failCode(), action(), failureAction() 
{
}

ActionParser::ActionParser(const std::string& name_, cminor::pom::CompoundStatement* successCode_, Parser* child_): 
    UnaryParser(name_, child_, child_->Info()), successCode(successCode_), failCode(), action(), failureAction() 
{
}

ActionParser::ActionParser(const std::string& name_, cminor::pom::CompoundStatement* successCode_, cminor::pom::CompoundStatement* failCode_, 
    Parser* child_): UnaryParser(name_, child_, child_->Info()), successCode(successCode_), failCode(failCode_), action(), failureAction() 
{
}

std::string ActionParser::MethodName() const 
{ 
    return Name() + "Action"; 
}

std::string ActionParser::VariableName() const 
{ 
    return (Name().length() > 0 ? std::string(1, std::tolower(Name()[0])) + Name().substr(1) : Name()) + "ActionParser"; 
}

Match ActionParser::Parse(Scanner& scanner, ObjectStack& stack)
{
    Span actionSpan = scanner.GetSpan();
    Match match = Child()->Parse(scanner, stack);
    if (match.Hit())
    {
        if (action)
        {
            bool pass = true;
            actionSpan.SetEnd(scanner.GetSpan().Start());
            const char* matchBegin = scanner.Start() + actionSpan.Start();
            const char* matchEnd = scanner.Start() + actionSpan.End();
            (*action)(matchBegin, matchEnd, actionSpan, scanner.FileName(), pass);
            if (!pass)
            {
                return Match::Nothing();
            }
        }
    }
    else if (failureAction)
    {
        (*failureAction)();
    }
    return match;
}

void ActionParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    Child()->Accept(visitor);
    visitor.EndVisit(*this);
}


} } // namespace cminor::parsing
