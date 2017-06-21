// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_PARSING_PARSING_DATA_INCLUDED
#define CMINOR_PARSING_PARSING_DATA_INCLUDED
#include <cminor/util/Error.hpp>
#include <memory>
#include <stack>
#include <vector>

namespace cminor { namespace parsing {

struct Context
{
    virtual ~Context();
};

class ContextStack
{
public:
    void Push(Context* context) { stack.push(std::unique_ptr<Context>(context)); }
    Context* Pop() { Assert(!stack.empty(), "context stack is empty"); Context* context = stack.top().release(); stack.pop(); return context; }
private:
    std::stack<std::unique_ptr<Context>> stack;
};

struct RuleData
{
    std::unique_ptr<Context> context;
    ContextStack contextStack;
};

class ParsingData
{
public:
    ParsingData(int numRules);
    void PushContext(int ruleId, Context* context) 
    { 
        Assert(ruleId >= 0 && ruleId < ruleData.size(), "invalid rule id"); 
        ruleData[ruleId].contextStack.Push(ruleData[ruleId].context.release()); 
        ruleData[ruleId].context.reset(context);
    }
    void PopContext(int ruleId) 
    { 
        Assert(ruleId >= 0 && ruleId < ruleData.size(), "invalid rule id"); 
        ruleData[ruleId].context.reset(ruleData[ruleId].contextStack.Pop());  
    }
    Context* GetContext(int ruleId) const 
    { 
        Assert(ruleId >= 0 && ruleId < ruleData.size(), "invalid rule id"); 
        return ruleData[ruleId].context.get();  
    }
private:
    std::vector<RuleData> ruleData;
};

} } // namespace cminor::parsing

#endif // CMINOR_PARSING_PARSING_DATA_INCLUDED
