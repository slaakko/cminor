// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Scanner.hpp>
#include <cminor/pl/Visitor.hpp>
#include <cminor/pl/Exception.hpp>

namespace cminor { namespace parsing {

UnaryParser::UnaryParser(const std::string& name_, Parser* child_, const std::string& info_): Parser(name_, info_), child(child_) 
{
    Own(child);
}

OptionalParser::OptionalParser(Parser* child_): UnaryParser("optional", child_, child_->Info() + "?") 
{
}

Match OptionalParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    Span save = scanner.GetSpan();
    bool synchronizing = scanner.Synchronizing();
    Match match = Child()->Parse(scanner, stack, parsingData);
    if (match.Hit())
    {
        return match;
    }
    scanner.SetSpan(save);
    scanner.SetSynchronizing(synchronizing);
    return Match::Empty();
}

void OptionalParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    Child()->Accept(visitor);
    visitor.EndVisit(*this);
}

PositiveParser::PositiveParser(Parser* child_): UnaryParser("positive", child_, child_->Info()) 
{
}

Match PositiveParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    Match match = Child()->Parse(scanner, stack, parsingData);
    if (match.Hit())
    {
        for (;;)
        {
            Span save = scanner.GetSpan();
            bool synchronizing = scanner.Synchronizing();
            scanner.Skip();
            Match next = Child()->Parse(scanner, stack, parsingData);
            if (next.Hit())
            {
                match.Concatenate(next);
            }
            else
            {
                scanner.SetSpan(save);
                scanner.SetSynchronizing(synchronizing);
                break;
            }
        }
    }
    return match;
}

void PositiveParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    Child()->Accept(visitor);
    visitor.EndVisit(*this);
}

KleeneStarParser::KleeneStarParser(Parser* child_): UnaryParser("kleene", child_, child_->Info()) 
{
}

Match KleeneStarParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    Match match = Match::Empty();
    bool first = true;
    for (;;)
    {
        Span save = scanner.GetSpan();
        bool synchronizing = scanner.Synchronizing();
        if (first)
        {
            first = false;
        }
        else
        {
            scanner.Skip();
        }
        Match next = Child()->Parse(scanner, stack, parsingData);
        if (next.Hit())
        {
            match.Concatenate(next);
        }
        else
        {
            scanner.SetSpan(save);
            scanner.SetSynchronizing(synchronizing);
            break;
        }
    }
    return match;
}

void KleeneStarParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    Child()->Accept(visitor);
    visitor.EndVisit(*this);
}

ExpectationParser::ExpectationParser(Parser* child_): UnaryParser("expectation", child_, child_->Info()) 
{
}

Match ExpectationParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    Span expectationSpan = scanner.GetSpan();
    Match match = Match::Nothing();
    try
    {
        match = Child()->Parse(scanner, stack, parsingData);
    }
    catch (const ExpectationFailure& ex)
    {
        throw ExpectationFailure(Child()->Info() + ex.Info(), ex.FileName(), ex.GetSpan(), ex.Start(), ex.End());
    }
    if (match.Hit() || CC())
    {
        return match;
    }
    else
    {
        throw ExpectationFailure(Child()->Info(), scanner.FileName(), expectationSpan, scanner.Start(), scanner.End());
    }
}

void ExpectationParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    Child()->Accept(visitor);
    visitor.EndVisit(*this);
}

CCOptParser::CCOptParser(Parser* child_) : UnaryParser("ccopt", child_, child_->Info())
{
}

Match CCOptParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    Span save = scanner.GetSpan();
    bool synchronizing = scanner.Synchronizing();
    Match match = Child()->Parse(scanner, stack, parsingData);
    if (match.Hit())
    {
        return match;
    }
    if (CC())
    {
        scanner.SetSpan(save);
        scanner.SetSynchronizing(synchronizing);
        return Match::Empty();
    }
    return Match::Nothing();
}

void CCOptParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    Child()->Accept(visitor);
    visitor.EndVisit(*this);
}

TokenParser::TokenParser(Parser* child_): UnaryParser("token", child_, child_->Info()) 
{
}

Match TokenParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    scanner.BeginToken();
    Match match = Child()->Parse(scanner, stack, parsingData);
    scanner.EndToken();
    return match;
}

void TokenParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    Child()->Accept(visitor);
    visitor.EndVisit(*this);
}

BinaryParser::BinaryParser(const std::string& name_, Parser* left_, Parser* right_, const std::string& info_): Parser(name_, info_), left(left_), right(right_) 
{
    Own(left);
    Own(right);
}

SequenceParser::SequenceParser(Parser* left_, Parser* right_): BinaryParser("sequence", left_, right_, left_->Info()) 
{
}

Match SequenceParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    Match leftMatch = Left()->Parse(scanner, stack, parsingData);
    if (leftMatch.Hit())
    {
        scanner.Skip();
        Match rightMatch = Right()->Parse(scanner, stack, parsingData);
        if (rightMatch.Hit())
        {
            leftMatch.Concatenate(rightMatch);
            return leftMatch;
        }
    }
    return Match::Nothing();
}

void SequenceParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    Left()->Accept(visitor);
    visitor.Visit(*this);
    Right()->Accept(visitor);
    visitor.EndVisit(*this);
}

AlternativeParser::AlternativeParser(Parser* left_, Parser* right_): BinaryParser("alternative", left_, right_, left_->Info() + " | " + right_->Info()) 
{
}

Match AlternativeParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    Span save = scanner.GetSpan();
    bool synchronizing = scanner.Synchronizing();
    Match leftMatch = Left()->Parse(scanner, stack, parsingData);
    if (leftMatch.Hit())
    {
        return leftMatch;
    }
    scanner.SetSpan(save);
    scanner.SetSynchronizing(synchronizing);
    return Right()->Parse(scanner, stack, parsingData);
}

void AlternativeParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    Left()->Accept(visitor);
    visitor.Visit(*this);
    Right()->Accept(visitor);
    visitor.EndVisit(*this);
}

DifferenceParser::DifferenceParser(Parser* left_, Parser* right_): BinaryParser("difference", left_, right_, left_->Info() + " - " + right_->Info()) 
{
}

Match DifferenceParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    Span save = scanner.GetSpan();
    bool synchronizing = scanner.Synchronizing();
    Match leftMatch = Left()->Parse(scanner, stack, parsingData);
    if (leftMatch.Hit())
    {
        Span tmp = scanner.GetSpan();
        bool tmpSynchronizing = scanner.Synchronizing();
        scanner.SetSpan(save);
        scanner.SetSynchronizing(synchronizing);
        save = tmp;
        Match rightMatch = Right()->Parse(scanner, stack, parsingData);
        if (!rightMatch.Hit() || rightMatch.Length() < leftMatch.Length())
        {
            scanner.SetSpan(save);
            scanner.SetSynchronizing(tmpSynchronizing);
            return leftMatch;
        }
    }
    return Match::Nothing();
}

void DifferenceParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    Left()->Accept(visitor);
    visitor.Visit(*this);
    Right()->Accept(visitor);
    visitor.EndVisit(*this);
}

ExclusiveOrParser::ExclusiveOrParser(Parser* left_, Parser* right_): BinaryParser("exclusiveOr", left_, right_, left_->Info() + " ^ " + right_->Info()) 
{
}

Match ExclusiveOrParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    Span save = scanner.GetSpan();
    bool synchronizing = scanner.Synchronizing();
    Match leftMatch = Left()->Parse(scanner, stack, parsingData);
    Span temp = scanner.GetSpan();
    bool tempSynchronizing = scanner.Synchronizing();
    scanner.SetSpan(save);
    scanner.SetSynchronizing(synchronizing);
    save = temp;
    Match rightMatch = Right()->Parse(scanner, stack, parsingData);
    bool match = leftMatch.Hit() ? !rightMatch.Hit() : rightMatch.Hit();
    if (match)
    {
        if (leftMatch.Hit())
        {
            scanner.SetSpan(save);
            scanner.SetSynchronizing(tempSynchronizing);
        }
        return leftMatch.Hit() ? leftMatch : rightMatch;
    }
    return Match::Nothing();
}

void ExclusiveOrParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    Left()->Accept(visitor);
    visitor.Visit(*this);
    Right()->Accept(visitor);
    visitor.EndVisit(*this);
}

IntersectionParser::IntersectionParser(Parser* left_, Parser* right_): BinaryParser("intersection", left_, right_, left_->Info() + " ^ " + right_->Info()) 
{
}

Match IntersectionParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    Span save = scanner.GetSpan();
    bool synchronizing = scanner.Synchronizing();
    Match leftMatch = Left()->Parse(scanner, stack, parsingData);
    if (leftMatch.Hit())
    {
        scanner.SetSpan(save);
        scanner.SetSynchronizing(synchronizing);
        Match rightMatch = Right()->Parse(scanner, stack, parsingData);
        if (leftMatch.Length() == rightMatch.Length())
        {
            return leftMatch;
        }
    }
    return Match::Nothing();
}

void IntersectionParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    Left()->Accept(visitor);
    visitor.Visit(*this);
    Right()->Accept(visitor);
    visitor.EndVisit(*this);
}

ListParser::ListParser(Parser* left_, Parser* right_): 
    UnaryParser("list", new SequenceParser(left_, new KleeneStarParser(new SequenceParser(right_, left_))), 
        left_->Info() + " % " + right_->Info()), left(left_), right(right_)
{
}

Match ListParser::Parse(Scanner& scanner, ObjectStack& stack, ParsingData* parsingData)
{
    return Child()->Parse(scanner, stack, parsingData);
}

void ListParser::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    left->Accept(visitor);
    visitor.Visit(*this);
    right->Accept(visitor);
    visitor.EndVisit(*this);
}

} } // namespace cminor::parsing
