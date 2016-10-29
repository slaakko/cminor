// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_OVERLOAD_RESOLUTION_INCLUDED
#define CMINOR_BINDER_OVERLOAD_RESOLUTION_INCLUDED
#include <cminor/binder/BoundExpression.hpp>
#include <cminor/symbols/FunctionSymbol.hpp>

namespace cminor { namespace binder {

class BoundCompileUnit;

enum class OverloadResolutionFlags : uint8_t
{
    none = 0,
    dontThrow = 1 << 0
};

inline OverloadResolutionFlags operator|(OverloadResolutionFlags left, OverloadResolutionFlags right)
{
    return OverloadResolutionFlags(uint8_t(left) | uint8_t(right));
}

inline OverloadResolutionFlags operator&(OverloadResolutionFlags left, OverloadResolutionFlags right)
{
    return OverloadResolutionFlags(uint8_t(left) & uint8_t(right));
}

struct FunctionScopeLookup
{
    FunctionScopeLookup(ScopeLookup scopeLookup_) : scopeLookup(scopeLookup_), scope(nullptr) {}
    FunctionScopeLookup(ScopeLookup scopeLookup_, ContainerScope* scope_) : scopeLookup(scopeLookup_), scope(scope_) {}
    ScopeLookup scopeLookup;
    ContainerScope* scope;
};

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
    FunctionMatch(FunctionSymbol* fun_) : fun(fun_), numConversions(0), castRequired(false), castSourceType(nullptr), castTargetType(nullptr) {}
    FunctionSymbol* fun;
    std::vector<ArgumentMatch> argumentMatches;
    int numConversions;
    bool castRequired;
    TypeSymbol* castSourceType;
    TypeSymbol* castTargetType;
};

struct BetterFunctionMatch
{
    bool operator()(const FunctionMatch& left, const FunctionMatch& right) const;
};

bool FindConversions(BoundCompileUnit& boundCompileUnit, FunctionSymbol* function, const std::vector<std::unique_ptr<BoundExpression>>& arguments, FunctionMatch& functionMatch,
    ConversionType conversionType);

std::unique_ptr<BoundFunctionCall> ResolveOverload(BoundCompileUnit& boundCompileUnit, StringPtr groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups, 
    std::vector<std::unique_ptr<BoundExpression>>& arguments, const Span& span);

std::unique_ptr<BoundFunctionCall> ResolveOverload(BoundCompileUnit& boundCompileUnit, StringPtr groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups,
    std::vector<std::unique_ptr<BoundExpression>>& arguments, const Span& span, OverloadResolutionFlags flags, std::unique_ptr<Exception>& exception);


} } // namespace cminor::binder

#endif // CMINOR_BINDER_OVERLOAD_RESOLUTION_INCLUDED
