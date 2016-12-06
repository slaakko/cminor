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
    operatorMap["*"] = Operator::mul;
    operatorMap["&&"] = Operator::and_;
    operatorMap["||"] = Operator::or_;
    operatorMap["&"] = Operator::bitand_;
    operatorMap["^"] = Operator::bitxor;
    operatorMap["|"] = Operator::bitor_;
    operatorMap["+"] = Operator::plus;
    operatorMap["-"] = Operator::minus;
    operatorMap["!"] = Operator::not_;
    operatorMap["~"] = Operator::neg;
    operatorMap[".*"] = Operator::dotStar;
    operatorMap["->*"] = Operator::arrowStar;
    operatorMap["/"] = Operator::div;
    operatorMap["%"] = Operator::rem;
    operatorMap["<<"] = Operator::shiftLeft;
    operatorMap[">>"] = Operator::shiftRight;
    operatorMap["<"] = Operator::less;
    operatorMap[">"] = Operator::greater;
    operatorMap["<="] = Operator::lessOrEq;
    operatorMap[">="] = Operator::greaterOrEq;
    operatorMap["=="] = Operator::eq;
    operatorMap["!="] = Operator::notEq;
    operatorMap[","] = Operator::comma;
    operatorMap["="] = Operator::assign;
    operatorMap["*="] = Operator::mulAssing;
    operatorMap["/="] = Operator::divAssign;
    operatorMap["%="] = Operator::remAssign;
    operatorMap["+="] = Operator::plusAssign;
    operatorMap["-="] = Operator::minusAssing;
    operatorMap["<<="] = Operator::shiftLeftAssign;
    operatorMap[">>="] = Operator::shiftRightAssign;
    operatorMap["&="] = Operator::andAssing;
    operatorMap["^="] = Operator::xorAssing;
    operatorMap["|="] = Operator::orAssign;
    operatorStrMap[Operator::deref] = "*";
    operatorStrMap[Operator::addrOf] = "&";
    operatorStrMap[Operator::plus] = "+";
    operatorStrMap[Operator::minus] = "-";
    operatorStrMap[Operator::not_] = "!";
    operatorStrMap[Operator::neg] = "~";
    operatorStrMap[Operator::dotStar] = ".*";
    operatorStrMap[Operator::arrowStar] = "->*";
    operatorStrMap[Operator::and_] = "&&";
    operatorStrMap[Operator::or_] = "||";
    operatorStrMap[Operator::bitand_] = "&";
    operatorStrMap[Operator::bitxor] = "^";
    operatorStrMap[Operator::bitor_] = "|";
    operatorStrMap[Operator::mul] = "*";
    operatorStrMap[Operator::div] = "/";
    operatorStrMap[Operator::rem] = "%";
    operatorStrMap[Operator::shiftLeft] = "<<";
    operatorStrMap[Operator::shiftRight] = ">>";
    operatorStrMap[Operator::less] = "<";
    operatorStrMap[Operator::greater] = ">";
    operatorStrMap[Operator::lessOrEq] = "<=";
    operatorStrMap[Operator::greaterOrEq] = ">=";
    operatorStrMap[Operator::eq] = "==";
    operatorStrMap[Operator::notEq] = "!=";
    operatorStrMap[Operator::comma] = ",";
    operatorStrMap[Operator::assign] = "=";
    operatorStrMap[Operator::mulAssing] = "*=";
    operatorStrMap[Operator::divAssign] = "/=";
    operatorStrMap[Operator::remAssign] = "%=";
    operatorStrMap[Operator::plusAssign] = "+=";
    operatorStrMap[Operator::minusAssing] = "-=";
    operatorStrMap[Operator::shiftLeftAssign] = "<<=";
    operatorStrMap[Operator::shiftRightAssign] = ">>=";
    operatorStrMap[Operator::andAssing] = "&=";
    operatorStrMap[Operator::xorAssing] = "^=";
    operatorStrMap[Operator::orAssign] = "|=";
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
