/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <cminor/pom/Operator.hpp>
#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace cminor { namespace pom {

struct OperatorHash
{
    size_t operator()(Operator op) const { return size_t(op); }
};

class OperatorTable
{
public:
    static void Init();
    static void Done();
    static OperatorTable& Instance() 
    {
        return *instance;
    }
    Operator GetOperator(const std::string& operatorName);
    std::string GetOperatorStr(Operator op);
private:
    static std::unique_ptr<OperatorTable> instance;
    OperatorTable();
    std::unordered_map<std::string, Operator> operatorMap;
    std::unordered_map<Operator, std::string, OperatorHash> operatorStrMap;
};

std::unique_ptr<OperatorTable>  OperatorTable::instance;

OperatorTable::OperatorTable()
{
    operatorMap["*"] = mul;
    operatorMap["&&"] = and_;
    operatorMap["||"] = or_;
    operatorMap["&"] = bitand_;
    operatorMap["^"] = bitxor;
    operatorMap["|"] = bitor_;
    operatorMap["+"] = plus;
    operatorMap["-"] = minus;
    operatorMap["!"] = not_;
    operatorMap["~"] = neg;
    operatorMap[".*"] = dotStar;
    operatorMap["->*"] = arrowStar;
    operatorMap["/"] = div;
    operatorMap["%"] = rem;
    operatorMap["<<"] = shiftLeft;
    operatorMap[">>"] = shiftRight;
    operatorMap["<"] = less;
    operatorMap[">"] = greater;
    operatorMap["<="] = lessOrEq;
    operatorMap[">="] = greaterOrEq;
    operatorMap["=="] = eq;
    operatorMap["!="] = notEq;
    operatorMap[","] = comma;
    operatorMap["="] = assign;
    operatorMap["*="] = mulAssing;
    operatorMap["/="] = divAssign;
    operatorMap["%="] = remAssign;
    operatorMap["+="] = plusAssign;
    operatorMap["-="] = minusAssing;
    operatorMap["<<="] = shiftLeftAssign;
    operatorMap[">>="] = shiftRightAssign;
    operatorMap["&="] = andAssing;
    operatorMap["^="] = xorAssing;
    operatorMap["|="] = orAssign;
    operatorStrMap[deref] = "*";
    operatorStrMap[addrOf] = "&";
    operatorStrMap[plus] = "+";
    operatorStrMap[minus] = "-";
    operatorStrMap[not_] = "!";
    operatorStrMap[neg] = "~";
    operatorStrMap[dotStar] = ".*";
    operatorStrMap[arrowStar] = "->*";
    operatorStrMap[and_] = "&&";
    operatorStrMap[or_] = "||";
    operatorStrMap[bitand_] = "&";
    operatorStrMap[bitxor] = "^";
    operatorStrMap[bitor_] = "|";
    operatorStrMap[mul] = "*";
    operatorStrMap[div] = "/";
    operatorStrMap[rem] = "%";
    operatorStrMap[shiftLeft] = "<<";
    operatorStrMap[shiftRight] = ">>";
    operatorStrMap[less] = "<";
    operatorStrMap[greater] = ">";
    operatorStrMap[lessOrEq] = "<=";
    operatorStrMap[greaterOrEq] = ">=";
    operatorStrMap[eq] = "==";
    operatorStrMap[notEq] = "!=";
    operatorStrMap[comma] = ",";
    operatorStrMap[assign] = "=";
    operatorStrMap[mulAssing] = "*=";
    operatorStrMap[divAssign] = "/=";
    operatorStrMap[remAssign] = "%=";
    operatorStrMap[plusAssign] = "+=";
    operatorStrMap[minusAssing] = "-=";
    operatorStrMap[shiftLeftAssign] = "<<=";
    operatorStrMap[shiftRightAssign] = ">>=";
    operatorStrMap[andAssing] = "&=";
    operatorStrMap[xorAssing] = "^=";
    operatorStrMap[orAssign] = "|=";
}

void OperatorTable::Init()
{
    instance.reset(new OperatorTable());
}

void OperatorTable::Done()
{
    instance.reset();
}

Operator OperatorTable::GetOperator(const std::string& operatorName)
{
    std::unordered_map<std::string, Operator>::const_iterator i = operatorMap.find(operatorName);
    if (i != operatorMap.end())
    {
        return i->second;
    }
    throw std::runtime_error("unknown operator '" + operatorName + "'");
}

std::string OperatorTable::GetOperatorStr(Operator op)
{
    std::unordered_map<Operator, std::string, OperatorHash>::const_iterator i = operatorStrMap.find(op);
    if (i != operatorStrMap.end())
    {
        return i->second;
    }
    throw std::runtime_error("unknown operator");
}

Operator GetOperator(const std::string& operatorName)
{
    return OperatorTable::Instance().GetOperator(operatorName);
}

std::string GetOperatorStr(Operator op)
{
    return OperatorTable::Instance().GetOperatorStr(op);
}

void OperatorInit()
{
    OperatorTable::Init();
}

void OperatorDone()
{
    OperatorTable::Done();
}

} } // namespace cminor::pom
