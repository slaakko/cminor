// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/OverloadResolution.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/BasicTypeFun.hpp>
#include <algorithm>

namespace cminor { namespace binder {

struct ArgumentMatch
{
    ArgumentMatch() : conversionFun(nullptr), conversionDistance(0) {}
    ArgumentMatch(FunctionSymbol* conversionFun_) : conversionFun(conversionFun_), conversionDistance(0) {}
    ArgumentMatch(FunctionSymbol* conversionFun_, int conversionDistance_) : conversionFun(conversionFun_), conversionDistance(conversionDistance_) {}
    FunctionSymbol* conversionFun;
    int conversionDistance;
};

inline bool BetterArgumentMatch(const ArgumentMatch& left, const ArgumentMatch& right) 
{
    if (left.conversionFun == nullptr && right.conversionFun != nullptr) return true;
    if (right.conversionFun == nullptr && left.conversionFun != nullptr) return false;
    if (left.conversionDistance < right.conversionDistance) return true;
    return false;
}

struct FunctionMatch
{
    FunctionMatch(FunctionSymbol* fun_) : fun(fun_), numConversions(0) {}
    FunctionSymbol* fun;
    std::vector<ArgumentMatch> argumentMatches;
    int numConversions;
};

struct BetterFunctionMatch
{
    bool operator()(const FunctionMatch& left, const FunctionMatch& right) const
    {
        int leftBetterArgumentMatches = 0;
        int rightBetterArgumentMatches = 0;
        int n = std::max(int(left.argumentMatches.size()), int(right.argumentMatches.size()));
        for (int i = 0; i < n; ++i)
        {
            ArgumentMatch leftMatch;
            if (i < int(left.argumentMatches.size()))
            {
                leftMatch = left.argumentMatches[i];
            }
            ArgumentMatch rightMatch;
            if (i < int(right.argumentMatches.size()))
            {
                rightMatch = right.argumentMatches[i];
            }
            if (BetterArgumentMatch(leftMatch, rightMatch))
            {
                ++leftBetterArgumentMatches;
            }
            else if (BetterArgumentMatch(rightMatch, leftMatch))
            {
                ++rightBetterArgumentMatches;
            }
        }
        if (leftBetterArgumentMatches > rightBetterArgumentMatches)
        {
            return true;
        }
        if (rightBetterArgumentMatches > leftBetterArgumentMatches)
        {
            return false;
        }
        if (left.numConversions < right.numConversions)
        {
            return true;
        }
        return false;
    }
};

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

bool FindConversions(BoundCompileUnit& boundCompileUnit, FunctionSymbol* function, const std::vector<std::unique_ptr<BoundExpression>>& arguments, FunctionMatch& functionMatch, 
    ConversionType conversionType)
{
    int arity = int(arguments.size());
    Assert(arity == function->Arity(), "wrong arity");
    for (int i = 0; i < arity; ++i)
    {
        BoundExpression* argument = arguments[i].get();
        TypeSymbol* sourceType = argument->GetType();
        ParameterSymbol* parameter = function->Parameters()[i];
        TypeSymbol* targetType = parameter->GetType();
        if (sourceType == targetType)
        {
            functionMatch.argumentMatches.push_back(ArgumentMatch());
        }
        else
        {
            FunctionSymbol* conversionFun = boundCompileUnit.GetAssembly().GetSymbolTable().GetConversion(sourceType, targetType);
            if (conversionFun)
            {
                if (conversionFun->GetConversionType() == conversionType)
                {
                    functionMatch.argumentMatches.push_back(ArgumentMatch(conversionFun, conversionFun->ConversionDistance()));
                    ++functionMatch.numConversions;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
    }
    return true;
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
        std::vector<FunctionMatch> functionMatches;
        for (FunctionSymbol* viableFunction : viableFunctions)
        {
            FunctionMatch functionMatch(viableFunction);
            if (FindConversions(boundCompileUnit, viableFunction, arguments, functionMatch, ConversionType::implicit_))
            {
                functionMatches.push_back(functionMatch);
            }
        }
        if (functionMatches.empty())
        {
            std::string overloadName = MakeOverloadName(groupName, arguments);
            throw Exception("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                std::to_string(viableFunctions.size()) + " viable functions examined.", span);
        }
        else if (functionMatches.size() > 1)
        {
            std::sort(functionMatches.begin(), functionMatches.end(), BetterFunctionMatch());
            if (BetterFunctionMatch()(functionMatches[0], functionMatches[1]))
            {
                const FunctionMatch& bestMatch = functionMatches[0];
                FunctionSymbol* bestFun = bestMatch.fun;
                std::unique_ptr<BoundFunctionCall> boundFunctionCall(new BoundFunctionCall(bestFun));
                for (int i = 0; i < arity; ++i)
                {
                    std::unique_ptr<BoundExpression>& argument = arguments[i];
                    const ArgumentMatch& argumentMatch = bestMatch.argumentMatches[i];
                    if (argumentMatch.conversionFun)
                    {
                        BoundConversion* conversion = new BoundConversion(std::move(argument), argumentMatch.conversionFun);
                        argument.reset(conversion);
                    }
                    boundFunctionCall->AddArgument(std::unique_ptr<BoundExpression>(argument.release()));
                }
                return boundFunctionCall;
            }
            else
            {
                std::string overloadName = MakeOverloadName(groupName, arguments);
                std::string matchedFunctionNames;
                bool first = true;
                FunctionMatch equalMatch = std::move(functionMatches[0]);
                std::vector<FunctionMatch> equalMatches;
                equalMatches.push_back(std::move(equalMatch));
                int n = int(functionMatches.size());
                for (int i = 1; i < n; ++i)
                {
                    FunctionMatch match = std::move(functionMatches[i]);
                    if (!BetterFunctionMatch()(equalMatches[0], match))
                    {
                        equalMatches.push_back(std::move(match));
                    }
                }
                std::vector<Span> references;
                for (const FunctionMatch& match : equalMatches)
                {
                    if (first)
                    {
                        first = false;
                    }
                    else
                    {
                        matchedFunctionNames.append(", or ");
                    }
                    matchedFunctionNames.append(ToUtf8(match.fun->FullName()));
                    references.push_back(match.fun->GetSpan());
                }
                throw Exception("overload resolution for overload name '" + overloadName + "' failed: call is ambiguous: \n" + matchedFunctionNames, span, references);
            }
        }
        else
        {
            const FunctionMatch& bestMatch = functionMatches[0];
            FunctionSymbol* singleBest = bestMatch.fun;
            std::unique_ptr<BoundFunctionCall> boundFunctionCall(new BoundFunctionCall(singleBest));
            for (int i = 0; i < arity; ++i)
            {
                std::unique_ptr<BoundExpression>& argument = arguments[i];
                const ArgumentMatch& argumentMatch = bestMatch.argumentMatches[i];
                if (argumentMatch.conversionFun)
                {
                    BoundConversion* conversion = new BoundConversion(std::move(argument), argumentMatch.conversionFun);
                    argument.reset(conversion);
                }
                boundFunctionCall->AddArgument(std::unique_ptr<BoundExpression>(argument.release()));
            }
            return boundFunctionCall;
        }
    }
}

} } // namespace cminor::binder
