// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/binder/OverloadResolution.hpp>
#include <cminor/binder/BoundCompileUnit.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>
#include <cminor/symbols/BasicTypeFun.hpp>
#include <cminor/util/Unicode.hpp>
#include <algorithm>

namespace cminor { namespace binder {

using namespace cminor::unicode;

bool BetterFunctionMatch::operator()(const FunctionMatch& left, const FunctionMatch& right) const
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

std::string MakeOverloadName(StringPtr groupName, const std::vector<std::unique_ptr<BoundExpression>>& arguments)
{
    std::string overloadName = ToUtf8(groupName.Value());
    overloadName.append(1, '(');
    bool first = true;
    for (const std::unique_ptr<BoundExpression>& argument : arguments)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            overloadName.append(", ");
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
            if (arity == 2 && i == 0  && (function->GroupName() == U"@init" || function->GroupName() == U"@assignment"))
            {
                return false;
            }
            if (targetType->IsRefType() && !sourceType->IsRefType())
            {
                functionMatch.refRequired = true;
                return false;
            }
            FunctionSymbol* conversionFun = boundCompileUnit.GetConversion(sourceType, targetType);
            if (conversionFun)
            {
                if (conversionFun->GetConversionType() == conversionType || conversionFun->GetConversionType() == ConversionType::implicit_)
                {
                    functionMatch.argumentMatches.push_back(ArgumentMatch(conversionFun, conversionFun->ConversionDistance()));
                    ++functionMatch.numConversions;
                }
                else
                {
                    if (arity == 2 && i == 1 && conversionType == ConversionType::implicit_ && conversionFun->GetConversionType() == ConversionType::explicit_)
                    {
                        functionMatch.castRequired = true;
                        functionMatch.castSourceType = sourceType;
                        functionMatch.castTargetType = targetType;
                    }
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

std::unique_ptr<BoundFunctionCall> CreateBoundFunctionCall(BoundCompileUnit& boundCompileUnit, FunctionSymbol* bestFun, std::vector<std::unique_ptr<BoundExpression>>& arguments, 
    const FunctionMatch& bestMatch, const Span& span)
{
    std::unique_ptr<BoundFunctionCall> boundFunctionCall(new BoundFunctionCall(boundCompileUnit.GetAssembly(), bestFun));
    int arity = int(arguments.size());
    for (int i = 0; i < arity; ++i)
    {
        std::unique_ptr<BoundExpression>& argument = arguments[i];
        const ArgumentMatch& argumentMatch = bestMatch.argumentMatches[i];
        if (argumentMatch.conversionFun)
        {
            if (argumentMatch.conversionFun->IsMemFunToClassDelegateConversion())
            {
                MemFunToClassDelegateConversion* conversion = static_cast<MemFunToClassDelegateConversion*>(argumentMatch.conversionFun);
                ClassDelegateTypeSymbol* targetClassDelegateType = static_cast<ClassDelegateTypeSymbol*>(conversion->ConversionTargetType());
                std::vector<std::unique_ptr<BoundExpression>> newArguments;
                newArguments.push_back(std::unique_ptr<BoundExpression>(new BoundTypeExpression(boundCompileUnit.GetAssembly(), targetClassDelegateType)));
                std::vector<FunctionScopeLookup> functionScopeLookups;
                functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, targetClassDelegateType->ClassOrNsScope()));
                std::unique_ptr<BoundFunctionCall> constructorCall = ResolveOverload(boundCompileUnit, U"@constructor", functionScopeLookups, newArguments, span);
                std::unique_ptr<BoundExpression> newClassDelegateExpr(new BoundNewExpression(constructorCall.release(), targetClassDelegateType));
                BoundExpression* classObject = nullptr; 
                if (argument->IsBoundMemberExpression())
                {
                    BoundMemberExpression* boundMemberExpr = static_cast<BoundMemberExpression*>(argument.get());
                    classObject = boundMemberExpr->ReleaseClassObject();
                }
                else if (argument->IsBoundFunctionGroupExpression())
                {
                    BoundFunctionGroupExpression* boundFunctionGroupExpr = static_cast<BoundFunctionGroupExpression*>(argument.get());
                    classObject = boundFunctionGroupExpr->ReleaseClassObject();
                }
                else
                {
                    Assert(false, "bound member expression or bound function group expression expected");
                }
                if (!classObject)
                {
                    throw Exception("class object not bound", span);
                }
                argument.reset(new BoundClassDelegateClassObjectPair(boundCompileUnit.GetAssembly(), std::move(newClassDelegateExpr), std::unique_ptr<BoundExpression>(classObject)));
            }
            BoundConversion* conversion = new BoundConversion(boundCompileUnit.GetAssembly(), std::move(argument), argumentMatch.conversionFun);
            argument.reset(conversion);
        }
        boundFunctionCall->AddArgument(std::unique_ptr<BoundExpression>(argument.release()));
    }
    return boundFunctionCall;
}

std::unique_ptr<BoundFunctionCall> ResolveOverload(BoundCompileUnit& boundCompileUnit, StringPtr groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups,
    std::vector<std::unique_ptr<BoundExpression>>& arguments, const Span& span)
{
    std::unique_ptr<Exception> exception;
    return ResolveOverload(boundCompileUnit, groupName, functionScopeLookups, arguments, span, OverloadResolutionFlags::none, exception);
}

std::unique_ptr<BoundFunctionCall> ResolveOverload(BoundCompileUnit& boundCompileUnit, StringPtr groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups, 
    std::vector<std::unique_ptr<BoundExpression>>& arguments, const Span& span, OverloadResolutionFlags flags, std::unique_ptr<Exception>& exception)
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
        if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
        {
            exception.reset(new Exception("overload resolution failed: '" + overloadName + "' not found. " +
                "No viable functions taking " + std::to_string(arity) + " arguments found in function group '" + ToUtf8(groupName.Value()) + "'", span));
            return std::unique_ptr<BoundFunctionCall>();
        }
        else
        {
            throw Exception("overload resolution failed: '" + overloadName + "' not found. " +
                "No viable functions taking " + std::to_string(arity) + " arguments found in function group '" + ToUtf8(groupName.Value()) + "'", span);
        }
    }
    else
    {
        std::vector<FunctionMatch> functionMatches;
        std::vector<FunctionMatch> failedFunctionMatches;
        for (FunctionSymbol* viableFunction : viableFunctions)
        {
            FunctionMatch functionMatch(viableFunction);
            if (FindConversions(boundCompileUnit, viableFunction, arguments, functionMatch, ConversionType::implicit_))
            {
                functionMatches.push_back(functionMatch);
            }
            else
            {
                failedFunctionMatches.push_back(functionMatch);
            }
        }
        if (functionMatches.empty())
        {
            std::string overloadName = MakeOverloadName(groupName, arguments);
            bool castRequired = false;
            bool refRequired = false;
            TypeSymbol* castSourceType = nullptr;
            TypeSymbol* castTargetType = nullptr;
            if (!failedFunctionMatches.empty())
            {
                int n = int(failedFunctionMatches.size());
                for (int i = 0; i < n; ++i)
                {
                    if (failedFunctionMatches[i].castRequired)
                    {
                        castRequired = true;
                        castSourceType = failedFunctionMatches[i].castSourceType;
                        castTargetType = failedFunctionMatches[i].castTargetType;
                        break;
                    }
                    else if (failedFunctionMatches[i].refRequired)
                    {
                        refRequired = true;
                        break;
                    }
                }
            }
            if (castRequired)
            {
                Assert(castSourceType, "cast source type not set");
                Assert(castTargetType, "cast target type not set");
                if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
                {
                    exception.reset(new CastOverloadException("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                        std::to_string(viableFunctions.size()) + " viable functions examined. Note: cannot convert implicitly from '" +
                        ToUtf8(castSourceType->FullName()) + "' to '" + ToUtf8(castTargetType->FullName()) + "' but explicit conversion (cast) exists.", span));
                    return std::unique_ptr<BoundFunctionCall>();
                }
                else
                {
                    throw CastOverloadException("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                        std::to_string(viableFunctions.size()) + " viable functions examined. Note: cannot convert implicitly from '" +
                        ToUtf8(castSourceType->FullName()) + "' to '" + ToUtf8(castTargetType->FullName()) + "' but explicit conversion (cast) exists.", span);
                }
            }
            else if (refRequired)
            {
                if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
                {
                    exception.reset(new RefOverloadException("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                        std::to_string(viableFunctions.size()) + " viable functions examined. Note: ref type parameter needs a ref specifier for the argument at the call site.", span));
                    return std::unique_ptr<BoundFunctionCall>();
                }
                else
                {
                    throw RefOverloadException("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                        std::to_string(viableFunctions.size()) + " viable functions examined. Note: ref type parameter needs a ref specifier for the argument at the call site.", span);
                }
            }
            else
            {
                if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
                {
                    exception.reset(new Exception("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                        std::to_string(viableFunctions.size()) + " viable functions examined.", span));
                    return std::unique_ptr<BoundFunctionCall>();
                }
                else
                {
                    throw Exception("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                        std::to_string(viableFunctions.size()) + " viable functions examined.", span);
                }
            }
        }
        else if (functionMatches.size() > 1)
        {
            std::sort(functionMatches.begin(), functionMatches.end(), BetterFunctionMatch());
            if (BetterFunctionMatch()(functionMatches[0], functionMatches[1]))
            {
                const FunctionMatch& bestMatch = functionMatches[0];
                FunctionSymbol* bestFun = bestMatch.fun;
                if (ClassTemplateSpecializationSymbol* classTemplateSpecialization = dynamic_cast<ClassTemplateSpecializationSymbol*>(bestFun->ContainingClass()))
                {
                    if (!bestFun->IsInstantiated())
                    {
                        bestFun->SetInstantiationRequested();
                        boundCompileUnit.GetClassTemplateRepository().Add(classTemplateSpecialization);
                    }
                }
                return CreateBoundFunctionCall(boundCompileUnit, bestFun, arguments, bestMatch, span);
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
                if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
                {
                    exception.reset(new Exception("overload resolution for overload name '" + overloadName + "' failed: call is ambiguous: \n" + matchedFunctionNames, span, references));
                    return std::unique_ptr<BoundFunctionCall>();
                }
                else
                {
                    throw Exception("overload resolution for overload name '" + overloadName + "' failed: call is ambiguous: \n" + matchedFunctionNames, span, references);
                }
            }
        }
        else
        {
            const FunctionMatch& bestMatch = functionMatches[0];
            FunctionSymbol* singleBest = bestMatch.fun;
            if (ClassTemplateSpecializationSymbol* classTemplateSpecialization = dynamic_cast<ClassTemplateSpecializationSymbol*>(singleBest->ContainingClass()))
            {
                if (!singleBest->IsInstantiated())
                {
                    singleBest->SetInstantiationRequested();
                    boundCompileUnit.GetClassTemplateRepository().Add(classTemplateSpecialization);
                }
            }
            return CreateBoundFunctionCall(boundCompileUnit, singleBest, arguments, bestMatch, span);
        }
    }
}

} } // namespace cminor::binder
