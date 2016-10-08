// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/OverloadResolution.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>

namespace cminor { namespace binder {

std::string MakeOverloadName(StringPtr groupName, const std::vector<std::unique_ptr<BoundExpression>>& arguments)
{
    std::string overloadName = ToUtf8(groupName.Value());
    overloadName.append(1, '(');
    bool first = true;
    for (const std::unique_ptr<BoundExpression>& argument : arguments)
    {
        if (first)
        {
            overloadName.append(", ");
        }
        else
        {
            first = false;
        }
        overloadName.append(ToUtf8(argument->GetType()->FullName()));
    }
    overloadName.append(1, ')');
    return overloadName;
}

std::unique_ptr<BoundFunctionCall> ResolveOverload(BoundCompileUnit& boundCompileUnit, StringPtr groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups, 
    std::vector<std::unique_ptr<BoundExpression>>& arguments, const Span& span)
{
    int arity = int(arguments.size());
    std::unordered_set<FunctionSymbol*> viableFunctions;
    std::unordered_set<ContainerScope*> scopesLookedUp;
    bool fileScopesLookedUp = false;
    for (const FunctionScopeLookup& functionScopeLookup : functionScopeLookups)
    {
        if (functionScopeLookup.scopeLookup == ScopeLookup::fileScopes && !fileScopesLookedUp)
        {
            fileScopesLookedUp = true;
            for (const std::unique_ptr<FileScope>& fileScope : boundCompileUnit.FileScopes())
            {
                fileScope->CollectViableFunctions(arity, groupName, scopesLookedUp, viableFunctions);
            }
        }
        else
        {
            functionScopeLookup.scope->CollectViableFunctions(arity, groupName, scopesLookedUp, functionScopeLookup.scopeLookup, viableFunctions);
        }
    }
    if (viableFunctions.empty())
    {
        std::string overloadName = MakeOverloadName(groupName, arguments);
        throw Exception("overload resolution failed: '" + overloadName + "' not found. " + 
            "No viable functions taking " + std::to_string(arity) + " arguments found in function group '" + ToUtf8(groupName.Value()) + "'", span);
    }
    else
    {
        std::vector<FunctionSymbol*> matchedFunctions;
        for (FunctionSymbol* viableFunction : viableFunctions)
        {
            matchedFunctions.push_back(viableFunction);
        }
        if (matchedFunctions.empty())
        {
            std::string overloadName = MakeOverloadName(groupName, arguments);
            throw Exception("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                std::to_string(viableFunctions.size()) + " viable functions examined.", span);
        }
        else if (matchedFunctions.size() > 1)
        {
            FunctionSymbol* best = matchedFunctions[0];
            std::unique_ptr<BoundFunctionCall> boundFunctionCall(new BoundFunctionCall(best->ReturnType()));
            for (std::unique_ptr<BoundExpression>& argument : arguments)
            {
                boundFunctionCall->AddArgument(std::unique_ptr<BoundExpression>(argument.release()));
            }
            return boundFunctionCall;
        }
        else
        {
            FunctionSymbol* singleBest = matchedFunctions[0];
            std::unique_ptr<BoundFunctionCall> boundFunctionCall(new BoundFunctionCall(singleBest->ReturnType()));
            for (std::unique_ptr<BoundExpression>& argument : arguments)
            {
                boundFunctionCall->AddArgument(std::unique_ptr<BoundExpression>(argument.release()));
            }
            return boundFunctionCall;
        }
    }
}

} } // namespace cminor::binder
