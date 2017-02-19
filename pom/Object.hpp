// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_POM_OBJECT_INCLUDED
#define CMINOR_POM_OBJECT_INCLUDED
#include <cminor/util/CodeFormatter.hpp>
#include <string>
#include <vector>
#include <memory>

namespace cminor { namespace pom {

using cminor::util::CodeFormatter;
class Visitor;

class CppObject
{
public:
    CppObject(const std::string& name_);
    virtual ~CppObject();
    const std::string& Name() const { return name; }
    void SetName(const std::string& newName) { name = newName; }
    void Own(CppObject* object);
    bool IsOwned() const { return isOwned; }
    void SetOwned() { isOwned = true; }
    void ResetOwned() { isOwned = false; }
    virtual std::string ToString() const;
    virtual void Print(CodeFormatter& formatter);
    virtual void Accept(Visitor& visitor) = 0;
    virtual int Rank() const { return 100; }
private:
    std::string name;
    bool isOwned;
    std::vector<std::unique_ptr<CppObject>> ownedObjects;
};

} } // namespace cminor::pom

#endif // CMINOR_POM_OBJECT_INCLUDED
