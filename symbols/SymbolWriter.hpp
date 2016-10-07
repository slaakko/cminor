// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_SYMBOLS_SYMBOL_WRITER_INCLUDED
#define CMINOR_SYMBOLS_SYMBOL_WRITER_INCLUDED
#include <cminor/symbols/Symbol.hpp>
#include <cminor/machine/Writer.hpp>

namespace cminor { namespace symbols {

using namespace cminor::machine;

class Assembly;
class Symbol;

class SymbolWriter : public Writer
{
public:
    SymbolWriter(const std::string& fileName_);
    Assembly* GetAssembly() const { Assert(assembly, "assembly not set"); return assembly; }
    void SetAssembly(Assembly* assembly_) { assembly = assembly_; }
    void Put(Symbol* symbol);
private:
    Assembly* assembly;
};

} } // namespace cminor::symbols

#endif // CMINOR_SYMBOLS_SYMBOL_WRITER_INCLUDED