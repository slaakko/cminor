// =================================
// Copyright (c) 2016 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_BINDER_CONTROL_FLOW_ANALYZER_INCLUDED
#define CMINOR_BINDER_CONTROL_FLOW_ANALYZER_INCLUDED

namespace cminor { namespace binder {

class BoundCompileUnit;

void AnalyzeControlFlow(BoundCompileUnit& boundCompileUnit);

} } // namespace cminor::binder

#endif // CMINOR_BINDER_CONTROL_FLOW_ANALYZER_INCLUDED
