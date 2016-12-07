// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_ACTION_INCLUDED
#define CMINOR_PARSING_ACTION_INCLUDED
#include <cminor/pl/Composite.hpp>
#include <cminor/pl/Scanner.hpp>
#include <cminor/pom/Statement.hpp>

namespace cminor { namespace parsing {

class ParsingAction
{
public:
    virtual ~ParsingAction();
    virtual void operator()(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass) const = 0;
};

template<typename Type>
class MemberParsingAction: public ParsingAction
{
public:
    MemberParsingAction(Type* type_, void (Type::*pm)(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)): 
        type(type_), memfun(pm)
    {    
    }
    virtual void operator()(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass) const
    {    
        (type->*memfun)(matchBegin, matchEnd, span, fileName, pass);
    }
private:
    Type* type;
    void (Type::*memfun)(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass);
};

class FailureAction
{
public:
    virtual ~FailureAction();
    virtual void operator()() const = 0;
};

template<typename Type>
class MemberFailureAction: public FailureAction
{
public:
    MemberFailureAction(Type* type_, void (Type::*pm)()):
        type(type_), memfun(pm)
    {
    }
    virtual void operator()() const
    {
        (type->*memfun)();
    }
private:
    Type* type;
    void (Type::*memfun)();
};

class PreCall
{
public:
    virtual ~PreCall();
    virtual void operator()(ObjectStack& stack) const = 0;
};

template <typename Type>
class MemberPreCall: public PreCall
{
public:
    MemberPreCall(Type* type_, void (Type::*pm)(ObjectStack& stack)):
        type(type_), memfun(pm)
    {
    }
    virtual void operator()(ObjectStack& stack) const
    {
        (type->*memfun)(stack);
    }
private:
    Type* type;
    void (Type::*memfun)(ObjectStack& stack);
};

class PostCall
{
public:
    virtual ~PostCall();
    virtual void operator()(ObjectStack& stack, bool matched) const = 0;
};

template <typename Type>
class MemberPostCall: public PostCall
{
public:
    MemberPostCall(Type* type_, void (Type::*pm)(ObjectStack& stack, bool matched)):
        type(type_), memfun(pm)
    {
    }
    virtual void operator()(ObjectStack& stack, bool matched) const
    {
        (type->*memfun)(stack, matched);
    }
private:
    Type* type;
    void (Type::*memfun)(ObjectStack& stack, bool matched);
};

class ActionParser : public UnaryParser
{
public:
    ActionParser(const std::string& name_, Parser* child_);
    ActionParser(const std::string& name_, cminor::pom::CompoundStatement* successCode_, Parser* child_);
    ActionParser(const std::string& name_, cminor::pom::CompoundStatement* successCode_, cminor::pom::CompoundStatement* failCode_, 
        Parser* child_);
    virtual bool IsActionParser() const { return true; }
    std::string MethodName() const;
    std::string VariableName() const;
    cminor::pom::CompoundStatement* SuccessCode() const { return successCode.get(); }
    cminor::pom::CompoundStatement* FailCode() const { return failCode.get(); }
    void SetAction(ParsingAction* action_) { action = std::unique_ptr<ParsingAction>(action_); }
    void SetFailureAction(FailureAction* failureAction_) { failureAction = std::unique_ptr<FailureAction>(failureAction_); }
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
private:
    std::unique_ptr<cminor::pom::CompoundStatement> successCode;
    std::unique_ptr<cminor::pom::CompoundStatement> failCode;
    std::unique_ptr<ParsingAction> action;
    std::unique_ptr<FailureAction> failureAction;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_ACTION_INCLUDED

