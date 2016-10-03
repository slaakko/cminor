// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_SYMBOL_READER_INCLUDED
#define CMINOR_SYMBOLS_SYMBOL_READER_INCLUDED
#include <cminor/symbols/Symbol.hpp>
#include <cminor/machine/Reader.hpp>

namespace cminor { namespace symbols {

using namespace cminor::machine;

class SymbolReader : public Reader
{
public:
    SymbolReader(Machine& machine_, const std::string& fileName_);
    Assembly* GetAssembly() const { Assert(assembly, "assembly not set"); return assembly; }
    void SetAssembly(Assembly* assembly_) { assembly = assembly_; }
    Symbol* GetSymbol();
private:
    Assembly* assembly;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_SYMBOL_READER_INCLUDED
