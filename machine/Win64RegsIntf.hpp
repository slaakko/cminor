// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_MACHINE_WIN64_REGS_INCLUDED
#define CMINOR_MACHINE_WIN64_REGS_INCLUDED

#ifdef _WIN32
extern "C" void* getrsp();
extern "C" void* getrip();
extern "C" void* getrbp();
#endif

#endif // CMINOR_MACHINE_WIN64_REGS_INCLUDED
