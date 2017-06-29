// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/vmlib/VmFunction.hpp>
#include <cminor/vmlib/File.hpp>
#include <cminor/vmlib/Socket.hpp>
#include <cminor/vmlib/Threading.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/machine/Type.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Runtime.hpp>
#include <cminor/util/Random.hpp>
#include <cminor/util/Unicode.hpp>
#include <cminor/util/Defines.hpp>
#include <boost/filesystem.hpp>
#include <cctype>
#include <chrono>
#include <thread>

namespace cminor { namespace vmlib {

using namespace cminor::unicode;

enum class Platform : uint8_t
{
    unknown = 0, windows = 1, unix = 2
};

class VmSystemPlatform : public VmFunction
{
public:
    VmSystemPlatform(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemPlatform::VmSystemPlatform(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"platform"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemPlatform::Execute(Frame& frame)
{
    Platform platform = Platform::unknown;
#if defined(_WIN32) || defined(WIN32)
    platform = Platform::windows;
#elif defined(__unix__) || defined(__unix) || defined(__posix) || defined(__linux)
    platform = Platform::unix;
#endif
    frame.OpStack().Push(MakeIntegralValue<uint8_t>(uint8_t(platform), ValueType::byteType));
}

class VmSystemPowDoubleInt : public VmFunction
{
public:
    VmSystemPowDoubleInt(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemPowDoubleInt::VmSystemPowDoubleInt(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"pow"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemPowDoubleInt::Execute(Frame& frame)
{
    IntegralValue baseValue = frame.Local(0).GetValue();
    double base = baseValue.AsDouble();
    IntegralValue exponentValue = frame.Local(1).GetValue();
    int exponent = exponentValue.AsInt();
    double result = std::pow(base, exponent);
    frame.OpStack().Push(MakeIntegralValue<double>(result, ValueType::doubleType));
}

class VmSystemIsCSpaceChar : public VmFunction
{
public:
    VmSystemIsCSpaceChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsCSpaceChar::VmSystemIsCSpaceChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"iscspace"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsCSpaceChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isSpace = false;
    if (c < 256)
    {
        isSpace = std::isspace(static_cast<unsigned char>(c));
    }
    frame.OpStack().Push(MakeIntegralValue<bool>(isSpace, ValueType::boolType));
}

class VmSystemIsCAlphaChar : public VmFunction
{
public:
    VmSystemIsCAlphaChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsCAlphaChar::VmSystemIsCAlphaChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"iscalpha"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsCAlphaChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isAlpha = false;
    if (c < 256)
    {
        isAlpha = std::isalpha(static_cast<unsigned char>(c));
    }
    frame.OpStack().Push(MakeIntegralValue<bool>(isAlpha, ValueType::boolType));
}

class VmSystemIsCAlnumChar : public VmFunction
{
public:
    VmSystemIsCAlnumChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsCAlnumChar::VmSystemIsCAlnumChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"iscalnum"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsCAlnumChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isAlnum = false;
    if (c < 256)
    {
        isAlnum = std::isalnum(static_cast<unsigned char>(c));
    }
    frame.OpStack().Push(MakeIntegralValue<bool>(isAlnum, ValueType::boolType));
}

class VmSystemIsCDigitChar : public VmFunction
{
public:
    VmSystemIsCDigitChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsCDigitChar::VmSystemIsCDigitChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"iscdigit"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsCDigitChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isDigit = false;
    if (c < 256)
    {
        isDigit = std::isdigit(static_cast<unsigned char>(c));
    }
    frame.OpStack().Push(MakeIntegralValue<bool>(isDigit, ValueType::boolType));
}

class VmSystemIsCHexDigitChar : public VmFunction
{
public:
    VmSystemIsCHexDigitChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsCHexDigitChar::VmSystemIsCHexDigitChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"iscxdigit"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsCHexDigitChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isHexDigit = false;
    if (c < 256)
    {
        isHexDigit = std::isxdigit(static_cast<unsigned char>(c));
    }
    frame.OpStack().Push(MakeIntegralValue<bool>(isHexDigit, ValueType::boolType));
}

class VmSystemIsCPunctuationChar : public VmFunction
{
public:
    VmSystemIsCPunctuationChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsCPunctuationChar::VmSystemIsCPunctuationChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"iscpunct"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsCPunctuationChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isPunctuation = false;
    if (c < 256)
    {
        isPunctuation = std::ispunct(static_cast<unsigned char>(c));
    }
    frame.OpStack().Push(MakeIntegralValue<bool>(isPunctuation, ValueType::boolType));
}

class VmSystemIsCPrintableChar : public VmFunction
{
public:
    VmSystemIsCPrintableChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsCPrintableChar::VmSystemIsCPrintableChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"iscprint"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsCPrintableChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isPrintable = false;
    if (c < 256)
    {
        isPrintable = std::isprint(static_cast<unsigned char>(c));
    }
    frame.OpStack().Push(MakeIntegralValue<bool>(isPrintable, ValueType::boolType));
}

class VmSystemIsCLowerChar : public VmFunction
{
public:
    VmSystemIsCLowerChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsCLowerChar::VmSystemIsCLowerChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"isclower"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsCLowerChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isLower = false;
    if (c < 256)
    {
        isLower = std::islower(static_cast<unsigned char>(c));
    }
    frame.OpStack().Push(MakeIntegralValue<bool>(isLower, ValueType::boolType));
}

class VmSystemIsCUpperChar : public VmFunction
{
public:
    VmSystemIsCUpperChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsCUpperChar::VmSystemIsCUpperChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"iscupper"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsCUpperChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isUpper = false;
    if (c < 256)
    {
        isUpper = std::isupper(static_cast<unsigned char>(c));
    }
    frame.OpStack().Push(MakeIntegralValue<bool>(isUpper, ValueType::boolType));
}

class VmSystemToCLowerChar : public VmFunction
{
public:
    VmSystemToCLowerChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemToCLowerChar::VmSystemToCLowerChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"toclower"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemToCLowerChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    char32_t toLower = c;
    if (c < 256)
    {
        toLower = std::tolower(static_cast<unsigned char>(c));
    }
    frame.OpStack().Push(MakeIntegralValue<char32_t>(toLower, ValueType::charType));
}

class VmSystemToCUpperChar : public VmFunction
{
public:
    VmSystemToCUpperChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemToCUpperChar::VmSystemToCUpperChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"tocupper"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemToCUpperChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    char32_t toUpper = c;
    if (c < 256)
    {
        toUpper = std::toupper(static_cast<unsigned char>(c));
    }
    frame.OpStack().Push(MakeIntegralValue<char32_t>(toUpper, ValueType::charType));
}

class VmSystemObjectToString : public VmFunction
{
public:
    VmSystemObjectToString(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemObjectToString::VmSystemObjectToString(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"o2s"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemObjectToString::Execute(Frame& frame)
{
    try
    {
        IntegralValue value = frame.Local(0).GetValue();
        Assert(value.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference reference(value.Value());
        IntegralValue classDataValue = GetManagedMemoryPool().GetField(reference, 0);
        Assert(classDataValue.GetType() == ValueType::classDataPtr, "class data pointer expected");
        ClassData* classData = classDataValue.AsClassDataPtr();
        StringPtr name = classData->Type()->Name();
        StrLitToStringInst strLit2StringInst;
        const char32_t* nameLiteral = name.Value();
        frame.OpStack().Push(IntegralValue(nameLiteral));
        strLit2StringInst.Execute(frame);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemObjectGetHashCode : public VmFunction
{
public:
    VmSystemObjectGetHashCode(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemObjectGetHashCode::VmSystemObjectGetHashCode(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"ohash"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemObjectGetHashCode::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    Assert(value.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference reference(value.Value());
    if (reference.IsNull())
    {
        frame.OpStack().Push(MakeIntegralValue<uint64_t>(0, ValueType::ulongType));
    }
    else
    {
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        void* object = memoryPool.GetObject(reference, lock);
        ManagedAllocationHeader* header = GetAllocationHeader(object);
        ObjectHeader* objectHeader = &header->objectHeader;
        uint64_t hashCode = objectHeader->HashCode();
        frame.OpStack().Push(MakeIntegralValue<uint64_t>(hashCode, ValueType::ulongType));
    }
}

class VmSystemObjectEqual : public VmFunction
{
public:
    VmSystemObjectEqual(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemObjectEqual::VmSystemObjectEqual(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"oeq"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemObjectEqual::Execute(Frame& frame)
{
    try
    {
        IntegralValue leftValue = frame.Local(0).GetValue();
        Assert(leftValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference leftRef(leftValue.Value());
        IntegralValue rightValue = frame.Local(1).GetValue();
        Assert(rightValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference rightRef(rightValue.Value());
        bool result = false;
        if (leftRef.IsNull() && rightRef.IsNull())
        {
            result = true;
        }
        else if (leftRef.IsNull() || rightRef.IsNull())
        {
            result = false;
        }
        else
        {
            ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
            std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
            void* left = memoryPool.GetObjectNoThrowNoLock(leftRef);
            void* right = memoryPool.GetObjectNoThrowNoLock(rightRef);
            result = left == right;
        }
        frame.OpStack().Push(MakeIntegralValue<bool>(result, ValueType::boolType));
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemObjectLess : public VmFunction
{
public:
    VmSystemObjectLess(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemObjectLess::VmSystemObjectLess(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"oless"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemObjectLess::Execute(Frame& frame)
{
    try
    {
        IntegralValue leftValue = frame.Local(0).GetValue();
        Assert(leftValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference leftRef(leftValue.Value());
        IntegralValue rightValue = frame.Local(1).GetValue();
        Assert(rightValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference rightRef(rightValue.Value());
        bool result = false;
        if (leftRef.IsNull() && rightRef.IsNull())
        {
            result = false;
        }
        else if (leftRef.IsNull())
        {
            result = true;
        }
        else if (rightRef.IsNull())
        {
            result = false;
        }
        else
        {
            ManagedMemoryPool& pool = GetManagedMemoryPool();
            std::unique_lock<std::recursive_mutex> lock(pool.AllocationsMutex());
            void* left = pool.GetObjectNoThrowNoLock(leftRef);
            void* right = pool.GetObjectNoThrowNoLock(rightRef);
            result = left < right;
        }
        frame.OpStack().Push(MakeIntegralValue<bool>(result, ValueType::boolType));
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemStringConstructorCharArray : public VmFunction
{
public:
    VmSystemStringConstructorCharArray(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemStringConstructorCharArray::VmSystemStringConstructorCharArray(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"ca2s"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemStringConstructorCharArray::Execute(Frame& frame)
{
    try
    {
        IntegralValue stringValue = frame.Local(0).GetValue();
        Assert(stringValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference stringRef(stringValue.Value());
        IntegralValue charArrayValue = frame.Local(1).GetValue();
        Assert(charArrayValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference charArrayRef(charArrayValue.Value());
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        std::pair<AllocationHandle, int32_t> stringCharactersHandleStringSLengthPair = memoryPool.CreateStringCharsFromCharArray(frame.GetThread(), charArrayRef, lock);
        AllocationHandle stringCharactersHandle = stringCharactersHandleStringSLengthPair.first;
        int32_t stringLength = stringCharactersHandleStringSLengthPair.second;
        memoryPool.SetField(stringRef, 1, MakeIntegralValue<int32_t>(stringLength, ValueType::intType), lock);
        memoryPool.SetField(stringRef, 2, MakeIntegralValue<uint64_t>(stringCharactersHandle.Value(), ValueType::allocationHandle), lock);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemIOOpenFile : public VmFunction
{
public:
    VmSystemIOOpenFile(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIOOpenFile::VmSystemIOOpenFile(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"fopen"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIOOpenFile::Execute(Frame& frame)
{
    try
    {
        IntegralValue filePathValue = frame.Local(0).GetValue();
        Assert(filePathValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference filePathStr(filePathValue.Value());
        std::string filePath = GetManagedMemoryPool().GetUtf8String(filePathStr);
        IntegralValue modeValue = frame.Local(1).GetValue();
        Assert(modeValue.GetType() == ValueType::byteType, "byte expected");
        FileMode mode = static_cast<FileMode>(modeValue.AsByte());
        IntegralValue accessValue = frame.Local(2).GetValue();
        Assert(accessValue.GetType() == ValueType::byteType, "byte expected");
        FileAccess access = static_cast<FileAccess>(accessValue.AsByte());
        int32_t fileHandle = OpenFile(filePath, mode, access);
        frame.OpStack().Push(MakeIntegralValue<int32_t>(fileHandle, ValueType::intType));
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const FileSystemError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowFileSystemException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemIOCloseFile : public VmFunction
{
public:
    VmSystemIOCloseFile(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIOCloseFile::VmSystemIOCloseFile(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"fclose"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIOCloseFile::Execute(Frame& frame)
{
    try
    {
        IntegralValue fileHandleValue = frame.Local(0).GetValue();
        Assert(fileHandleValue.GetType() == ValueType::intType, "int expected");
        int32_t fileHandle = fileHandleValue.AsInt();
        CloseFile(fileHandle);
    }
    catch (const FileSystemError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowFileSystemException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemIOWriteByteToFile : public VmFunction
{
public:
    VmSystemIOWriteByteToFile(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIOWriteByteToFile::VmSystemIOWriteByteToFile(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"fputb"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIOWriteByteToFile::Execute(Frame& frame)
{
    try
    {
        IntegralValue fileHandleValue = frame.Local(0).GetValue();
        Assert(fileHandleValue.GetType() == ValueType::intType, "int expected");
        int32_t fileHandle = fileHandleValue.AsInt();
        IntegralValue byteValue = frame.Local(1).GetValue();
        Assert(byteValue.GetType() == ValueType::byteType, "byte expected");
        uint8_t value = byteValue.AsByte();
        WriteByteToFile(fileHandle, value);
    }
    catch (const FileSystemError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowFileSystemException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemIOWriteFile : public VmFunction
{
public:
    VmSystemIOWriteFile(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIOWriteFile::VmSystemIOWriteFile(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"fwrite"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIOWriteFile::Execute(Frame& frame)
{
    try
    {
        IntegralValue fileHandleValue = frame.Local(0).GetValue();
        Assert(fileHandleValue.GetType() == ValueType::intType, "int expected");
        int32_t fileHandle = fileHandleValue.AsInt();
        IntegralValue bufferValue = frame.Local(1).GetValue();
        Assert(bufferValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference buffer(bufferValue.Value());
        std::vector<uint8_t> bytes = GetManagedMemoryPool().GetBytes(buffer);
        IntegralValue countValue = frame.Local(2).GetValue();
        Assert(countValue.GetType() == ValueType::intType, "int expected");
        int32_t count = countValue.AsInt();
        WriteFile(fileHandle, &bytes[0], count);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const IndexOutOfRangeException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowIndexOutOfRangeException(ex, frame);
    }
    catch (const FileSystemError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowFileSystemException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemIOReadByteFromFile : public VmFunction
{
public:
    VmSystemIOReadByteFromFile(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIOReadByteFromFile::VmSystemIOReadByteFromFile(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"fgetb"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIOReadByteFromFile::Execute(Frame& frame)
{
    try
    {
        IntegralValue fileHandleValue = frame.Local(0).GetValue();
        Assert(fileHandleValue.GetType() == ValueType::intType, "int expected");
        int32_t fileHandle = fileHandleValue.AsInt();
        int32_t value = ReadByteFromFile(fileHandle);
        frame.OpStack().Push(MakeIntegralValue<int32_t>(value, ValueType::intType));
    }
    catch (const FileSystemError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowFileSystemException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemIOReadFile : public VmFunction
{
public:
    VmSystemIOReadFile(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIOReadFile::VmSystemIOReadFile(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"fread"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIOReadFile::Execute(Frame& frame)
{
    try
    {
        IntegralValue fileHandleValue = frame.Local(0).GetValue();
        Assert(fileHandleValue.GetType() == ValueType::intType, "int expected");
        int32_t fileHandle = fileHandleValue.AsInt();
        IntegralValue bufferValue = frame.Local(1).GetValue();
        Assert(bufferValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference buffer(bufferValue.Value());
        std::vector<uint8_t> bytes;
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        bytes.resize(memoryPool.GetNumArrayElements(buffer));
        int32_t result = ReadFile(fileHandle, &bytes[0], int32_t(bytes.size()));
        if (result > 0)
        {
            memoryPool.SetBytes(buffer, bytes, result);
        }
        frame.OpStack().Push(MakeIntegralValue<int32_t>(result, ValueType::intType));
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const IndexOutOfRangeException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowIndexOutOfRangeException(ex, frame);
    }
    catch (const FileSystemError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowFileSystemException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemIOSeekFile : public VmFunction
{
public:
    VmSystemIOSeekFile(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIOSeekFile::VmSystemIOSeekFile(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"fseek"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIOSeekFile::Execute(Frame& frame)
{
    try
    {
        IntegralValue fileHandleValue = frame.Local(0).GetValue();
        Assert(fileHandleValue.GetType() == ValueType::intType, "int expected");
        int32_t fileHandle = fileHandleValue.AsInt();
        IntegralValue posValue = frame.Local(1).GetValue();
        Assert(posValue.GetType() == ValueType::intType, "int expected");
        int32_t pos = posValue.AsInt();
        IntegralValue originValue = frame.Local(2).GetValue();
        Assert(originValue.GetType() == ValueType::byteType, "byte expected");
        SeekFile(fileHandle, pos, Origin(originValue.AsByte()));
    }
    catch (const FileSystemError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowFileSystemException(ex, frame);
    }
}

class VmSystemIOTellFile : public VmFunction
{
public:
    VmSystemIOTellFile(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIOTellFile::VmSystemIOTellFile(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"ftell"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIOTellFile::Execute(Frame& frame)
{
    try
    {
        IntegralValue fileHandleValue = frame.Local(0).GetValue();
        Assert(fileHandleValue.GetType() == ValueType::intType, "int expected");
        int32_t fileHandle = fileHandleValue.AsInt();
        int32_t filePos = TellFile(fileHandle);
        frame.OpStack().Push(MakeIntegralValue<int32_t>(filePos, ValueType::intType));
    }
    catch (const FileSystemError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowFileSystemException(ex, frame);
    }
}

class VmSystemIOFileExists : public VmFunction
{
public:
    VmSystemIOFileExists(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIOFileExists::VmSystemIOFileExists(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"fexists"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIOFileExists::Execute(Frame& frame)
{
    try
    {
        IntegralValue filePathValue = frame.Local(0).GetValue();
        Assert(filePathValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference filePathStr(filePathValue.Value());
        std::string filePath = GetManagedMemoryPool().GetUtf8String(filePathStr);
        bool exists = boost::filesystem::exists(filePath);
        frame.OpStack().Push(MakeIntegralValue<bool>(exists, ValueType::boolType));
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemIOLastWriteTimeLess : public VmFunction
{
public:
    VmSystemIOLastWriteTimeLess(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIOLastWriteTimeLess::VmSystemIOLastWriteTimeLess(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"writetimeless"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIOLastWriteTimeLess::Execute(Frame& frame)
{
    try
    {
        IntegralValue firstFilePathValue = frame.Local(0).GetValue();
        Assert(firstFilePathValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference firstFilePathStr(firstFilePathValue.Value());
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::string firstFilePath = memoryPool.GetUtf8String(firstFilePathStr);
        IntegralValue secondFilePathValue = frame.Local(1).GetValue();
        Assert(secondFilePathValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference secondFilePathStr(secondFilePathValue.Value());
        std::string secondFilePath = memoryPool.GetUtf8String(secondFilePathStr);
        bool less = boost::filesystem::last_write_time(firstFilePath) < boost::filesystem::last_write_time(secondFilePath);
        frame.OpStack().Push(MakeIntegralValue<bool>(less, ValueType::boolType));
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemNetSocketsCreateSocket : public VmFunction
{
public:
    VmSystemNetSocketsCreateSocket(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemNetSocketsCreateSocket::VmSystemNetSocketsCreateSocket(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"socket"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemNetSocketsCreateSocket::Execute(Frame& frame)
{
    try
    {
        int32_t socketHandle = CreateSocket();
        frame.OpStack().Push(MakeIntegralValue<int32_t>(socketHandle, ValueType::intType));
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SocketError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSocketException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemNetSocketsConnectSocket : public VmFunction
{
public:
    VmSystemNetSocketsConnectSocket(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemNetSocketsConnectSocket::VmSystemNetSocketsConnectSocket(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"connect"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemNetSocketsConnectSocket::Execute(Frame& frame)
{
    try
    {
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        IntegralValue nodeValue = frame.Local(0).GetValue();
        Assert(nodeValue.GetType() == ValueType::objectReference, "object reference expected");
        std::string node = memoryPool.GetUtf8String(nodeValue.Value());
        IntegralValue serviceValue = frame.Local(1).GetValue();
        Assert(serviceValue.GetType() == ValueType::objectReference, "object reference expected");
        std::string service = memoryPool.GetUtf8String(serviceValue.Value());
        int32_t socketHandle = ConnectSocket(node, service);
        frame.OpStack().Push(MakeIntegralValue<int32_t>(socketHandle, ValueType::intType));
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SocketError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSocketException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemNetSocketsCloseSocket : public VmFunction
{
public:
    VmSystemNetSocketsCloseSocket(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemNetSocketsCloseSocket::VmSystemNetSocketsCloseSocket(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"closesocket"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemNetSocketsCloseSocket::Execute(Frame& frame)
{
    try
    {
        IntegralValue socketValue = frame.Local(0).GetValue();
        Assert(socketValue.GetType() == ValueType::intType, "int expected");
        int32_t socketHandle = socketValue.AsInt();
        CloseSocket(socketHandle);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SocketError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSocketException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemNetSocketsBindSocket : public VmFunction
{
public:
    VmSystemNetSocketsBindSocket(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemNetSocketsBindSocket::VmSystemNetSocketsBindSocket(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"bind"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemNetSocketsBindSocket::Execute(Frame& frame)
{
    try
    {
        IntegralValue socketValue = frame.Local(0).GetValue();
        Assert(socketValue.GetType() == ValueType::intType, "int expected");
        int32_t socketHandle = socketValue.AsInt();
        IntegralValue portValue = frame.Local(1).GetValue();
        Assert(portValue.GetType() == ValueType::intType, "int expected");
        int port = portValue.AsInt();
        BindSocket(socketHandle, port);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SocketError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSocketException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemNetSocketsListenSocket : public VmFunction
{
public:
    VmSystemNetSocketsListenSocket(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemNetSocketsListenSocket::VmSystemNetSocketsListenSocket(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"listen"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemNetSocketsListenSocket::Execute(Frame& frame)
{
    try
    {
        IntegralValue socketValue = frame.Local(0).GetValue();
        Assert(socketValue.GetType() == ValueType::intType, "int expected");
        int32_t socketHandle = socketValue.AsInt();
        IntegralValue backlogValue = frame.Local(1).GetValue();
        Assert(backlogValue.GetType() == ValueType::intType, "int expected");
        int backlog = backlogValue.AsInt();
        ListenSocket(socketHandle, backlog);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SocketError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSocketException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemNetSocketsAcceptSocket : public VmFunction
{
public:
    VmSystemNetSocketsAcceptSocket(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemNetSocketsAcceptSocket::VmSystemNetSocketsAcceptSocket(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"accept"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemNetSocketsAcceptSocket::Execute(Frame& frame)
{
    try
    {
        IntegralValue socketValue = frame.Local(0).GetValue();
        Assert(socketValue.GetType() == ValueType::intType, "int expected");
        int32_t socketHandle = socketValue.AsInt();
        int32_t acceptedSocketHandle = AcceptSocket(socketHandle);
        frame.OpStack().Push(MakeIntegralValue<int32_t>(acceptedSocketHandle, ValueType::intType));
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SocketError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSocketException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemNetSocketsShutdownSocket : public VmFunction
{
public:
    VmSystemNetSocketsShutdownSocket(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemNetSocketsShutdownSocket::VmSystemNetSocketsShutdownSocket(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"shutdown"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemNetSocketsShutdownSocket::Execute(Frame& frame)
{
    try
    {
        IntegralValue socketValue = frame.Local(0).GetValue();
        Assert(socketValue.GetType() == ValueType::intType, "int expected");
        int32_t socketHandle = socketValue.AsInt();
        IntegralValue modeValue = frame.Local(1).GetValue();
        Assert(modeValue.GetType() == ValueType::byteType, "byte expected");
        ShutdownMode mode = static_cast<ShutdownMode>(modeValue.AsByte());
        ShutdownSocket(socketHandle, mode);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SocketError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSocketException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemNetSocketsSendSocket : public VmFunction
{
public:
    VmSystemNetSocketsSendSocket(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemNetSocketsSendSocket::VmSystemNetSocketsSendSocket(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"send"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemNetSocketsSendSocket::Execute(Frame& frame)
{
    try
    {
        IntegralValue socketValue = frame.Local(0).GetValue();
        Assert(socketValue.GetType() == ValueType::intType, "int expected");
        int32_t socketHandle = socketValue.AsInt();
        IntegralValue bufferValue = frame.Local(1).GetValue();
        Assert(bufferValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference buffer(bufferValue.Value());
        std::vector<uint8_t> bytes = GetManagedMemoryPool().GetBytes(buffer);
        IntegralValue countValue = frame.Local(2).GetValue();
        Assert(countValue.GetType() == ValueType::intType, "int expected");
        int len = countValue.AsInt();
        const char* buf = reinterpret_cast<const char*>(&bytes[0]);
        int bytesSent = SendSocket(socketHandle, buf, len, 0);
        frame.OpStack().Push(MakeIntegralValue<int32_t>(bytesSent, ValueType::intType));
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SocketError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSocketException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemNetSocketsReceiveSocket : public VmFunction
{
public:
    VmSystemNetSocketsReceiveSocket(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemNetSocketsReceiveSocket::VmSystemNetSocketsReceiveSocket(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"recv"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemNetSocketsReceiveSocket::Execute(Frame& frame)
{
    try
    {
        IntegralValue socketValue = frame.Local(0).GetValue();
        Assert(socketValue.GetType() == ValueType::intType, "int expected");
        int32_t socketHandle = socketValue.AsInt();
        IntegralValue bufferValue = frame.Local(1).GetValue();
        Assert(bufferValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference buffer(bufferValue.Value());
        std::vector<uint8_t> bytes;
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        bytes.resize(memoryPool.GetNumArrayElements(buffer));
        char* buf = reinterpret_cast<char*>(&bytes[0]);
        int len = int(bytes.size());
        int32_t bytesReceived = ReceiveSocket(socketHandle, buf, len, 0); 
        if (bytesReceived > 0)
        {
            memoryPool.SetBytes(buffer, bytes, bytesReceived);
        }
        frame.OpStack().Push(MakeIntegralValue<int32_t>(bytesReceived, ValueType::intType));
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const SocketError& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSocketException(ex, frame);
    }
    catch (const SystemException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowSystemException(ex, frame);
    }
}

class VmSystemGetEnvironmentVariable : public VmFunction
{
public:
    VmSystemGetEnvironmentVariable(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemGetEnvironmentVariable::VmSystemGetEnvironmentVariable(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"getenv"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemGetEnvironmentVariable::Execute(Frame& frame)
{
    IntegralValue environmentVarNameValue = frame.Local(0).GetValue();
    Assert(environmentVarNameValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference environmentVarName(environmentVarNameValue.Value());
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    std::string envVarName = memoryPool.GetUtf8String(environmentVarName);
    char* value = std::getenv(envVarName.c_str());
    if (value)
    {
        std::string s(value);
        std::u32string us = ToUtf32(s);
        ObjectReference envVarValue = memoryPool.CreateString(frame.GetThread(), us);
        frame.OpStack().Push(envVarValue);
    }
    else
    {
        frame.OpStack().Push(ObjectReference(0));
    }
}

class VmSystemGetPathSeparatorChar : public VmFunction
{
public:
    VmSystemGetPathSeparatorChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemGetPathSeparatorChar::VmSystemGetPathSeparatorChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"pathsep"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemGetPathSeparatorChar::Execute(Frame& frame)
{
#ifdef _WIN32
    char32_t s = ';';
#else
    char32_t s = ':';
#endif
    frame.OpStack().Push(MakeIntegralValue<char32_t>(s, ValueType::charType));
}

class VmSystemIOInternalGetCurrentWorkingDirectory : public VmFunction
{
public:
    VmSystemIOInternalGetCurrentWorkingDirectory(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIOInternalGetCurrentWorkingDirectory::VmSystemIOInternalGetCurrentWorkingDirectory(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"cwd"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIOInternalGetCurrentWorkingDirectory::Execute(Frame& frame)
{
    std::string workingDirectory = boost::filesystem::current_path().generic_string();
    std::u32string s = ToUtf32(workingDirectory);
    ObjectReference reference = GetManagedMemoryPool().CreateString(frame.GetThread(), s);
    frame.OpStack().Push(reference);
}

class VmSystemThreadingStartThreadWithThreadStartFunction : public VmFunction
{
public:
    VmSystemThreadingStartThreadWithThreadStartFunction(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingStartThreadWithThreadStartFunction::VmSystemThreadingStartThreadWithThreadStartFunction(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"tsf"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemThreadingStartThreadWithThreadStartFunction::Execute(Frame& frame)
{
    try
    {
        IntegralValue dlg = frame.Local(0).GetValue();
        Assert(dlg.GetType() == ValueType::functionPtr, "function pointer expected");
        Function* fun = dlg.AsFunctionPtr();
        if (!fun)
        {
            throw NullReferenceException("provided delegate is null");
        }
        int threadId = GetMachine().StartThread(fun, RunThreadKind::function, ObjectReference(0), ObjectReference(0));
        ClassData* threadClassData = ClassDataTable::GetClassData(U"System.Threading.Thread");
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex(), std::defer_lock_t());
        ObjectReference threadReference = memoryPool.CreateObject(frame.GetThread(), threadClassData->Type(), lock);
        memoryPool.SetField(threadReference, 0, IntegralValue(threadClassData), lock);
        memoryPool.SetField(threadReference, 1, MakeIntegralValue<int32_t>(threadId, ValueType::intType), lock);
        frame.OpStack().Push(threadReference);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
}

class VmSystemThreadingStartThreadWithThreadStartMethod : public VmFunction
{
public:
    VmSystemThreadingStartThreadWithThreadStartMethod(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingStartThreadWithThreadStartMethod::VmSystemThreadingStartThreadWithThreadStartMethod(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"tsm"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemThreadingStartThreadWithThreadStartMethod::Execute(Frame& frame)
{
    try
    {
        IntegralValue clsdlg = frame.Local(0).GetValue();
        Assert(clsdlg.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference classDelegateObject(clsdlg.Value());
        if (classDelegateObject.IsNull())
        {
            throw NullReferenceException("provided class delegate is null");
        }
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex(), std::defer_lock_t());
        IntegralValue classObjectValue = memoryPool.GetField(classDelegateObject, 1, lock);
        Assert(classObjectValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference receiver(classObjectValue.Value());
        IntegralValue functionValue = memoryPool.GetField(classDelegateObject, 2, lock);
        Assert(functionValue.GetType() == ValueType::functionPtr, "function pointer expected");
        Function* fun = functionValue.AsFunctionPtr();
        int threadId = GetMachine().StartThread(fun, RunThreadKind::method, receiver, ObjectReference(0));
        ClassData* threadClassData = ClassDataTable::GetClassData(U"System.Threading.Thread");
        ObjectReference threadReference = memoryPool.CreateObject(frame.GetThread(), threadClassData->Type(), lock);
        memoryPool.SetField(threadReference, 0, IntegralValue(threadClassData), lock);
        memoryPool.SetField(threadReference, 1, MakeIntegralValue<int32_t>(threadId, ValueType::intType), lock);
        frame.OpStack().Push(threadReference);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
}

class VmSystemThreadingStartThreadWithParameterizedThreadStartFunction : public VmFunction
{
public:
    VmSystemThreadingStartThreadWithParameterizedThreadStartFunction(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingStartThreadWithParameterizedThreadStartFunction::VmSystemThreadingStartThreadWithParameterizedThreadStartFunction(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"ptsf"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemThreadingStartThreadWithParameterizedThreadStartFunction::Execute(Frame& frame)
{
    try
    {
        IntegralValue dlg = frame.Local(0).GetValue();
        Assert(dlg.GetType() == ValueType::functionPtr, "function pointer expected");
        Function* fun = dlg.AsFunctionPtr();
        if (!fun)
        {
            throw NullReferenceException("provided delegate is null");
        }
        IntegralValue argValue = frame.Local(1).GetValue();
        Assert(argValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference arg(argValue.Value());
        int threadId = GetMachine().StartThread(fun, RunThreadKind::functionWithParam, ObjectReference(0), arg);
        ClassData* threadClassData = ClassDataTable::GetClassData(U"System.Threading.Thread");
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex(), std::defer_lock_t());
        ObjectReference threadReference = memoryPool.CreateObject(frame.GetThread(), threadClassData->Type(), lock);
        memoryPool.SetField(threadReference, 0, IntegralValue(threadClassData), lock);
        memoryPool.SetField(threadReference, 1, MakeIntegralValue<int32_t>(threadId, ValueType::intType), lock);
        frame.OpStack().Push(threadReference);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
}

class VmSystemThreadingStartThreadWithParameterizedThreadStartMethod : public VmFunction
{
public:
    VmSystemThreadingStartThreadWithParameterizedThreadStartMethod(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingStartThreadWithParameterizedThreadStartMethod::VmSystemThreadingStartThreadWithParameterizedThreadStartMethod(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"ptsm"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemThreadingStartThreadWithParameterizedThreadStartMethod::Execute(Frame& frame)
{
    try
    {
        IntegralValue clsdlg = frame.Local(0).GetValue();
        Assert(clsdlg.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference classDelegateObject(clsdlg.Value());
        if (classDelegateObject.IsNull())
        {
            throw NullReferenceException("provided class delegate is null");
        }
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex(), std::defer_lock_t());
        void* object = memoryPool.GetObject(classDelegateObject, lock);
        IntegralValue classObjectValue = memoryPool.GetField(classDelegateObject, 1, lock);
        Assert(classObjectValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference receiver(classObjectValue.Value());
        IntegralValue functionValue = memoryPool.GetField(classDelegateObject, 2, lock);
        Assert(functionValue.GetType() == ValueType::functionPtr, "function pointer expected");
        Function* fun = functionValue.AsFunctionPtr();
        IntegralValue argValue = frame.Local(1).GetValue();
        Assert(argValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference arg(argValue.Value());
        int threadId = GetMachine().StartThread(fun, RunThreadKind::methodWithParam, receiver, arg);
        ClassData* threadClassData = ClassDataTable::GetClassData(U"System.Threading.Thread");
        ObjectReference threadReference = memoryPool.CreateObject(frame.GetThread(), threadClassData->Type(), lock);
        memoryPool.SetField(threadReference, 0, IntegralValue(threadClassData), lock);
        memoryPool.SetField(threadReference, 1, MakeIntegralValue<int32_t>(threadId, ValueType::intType), lock);
        frame.OpStack().Push(threadReference);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
}

class VmSystemThreadingJoinThread : public VmFunction
{
public:
    VmSystemThreadingJoinThread(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingJoinThread::VmSystemThreadingJoinThread(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"join"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

struct WaitingSetter
{
    WaitingSetter(Thread& thread_) : thread(thread_)
    {
        prevState = thread.GetState();
        if (RunningNativeCode())
        {
#ifdef SHADOW_STACK_GC
            thread.SetFunctionStack(RtGetFunctionStack());
#endif
        }
        thread.SetState(ThreadState::waiting);
    }
    ~WaitingSetter()
    {
        thread.SetState(prevState);
    }
    Thread& thread;
    ThreadState prevState;
};

void VmSystemThreadingJoinThread::Execute(Frame& frame)
{
    try
    {
        IntegralValue threadValue = frame.Local(0).GetValue();
        ObjectReference threadRefefence(threadValue.Value());
        if (threadRefefence.IsNull())
        {
            throw NullReferenceException("tried to join null thread");
        }
        IntegralValue threadIdValue = GetManagedMemoryPool().GetField(threadRefefence, 1);
        Assert(threadIdValue.GetType() == ValueType::intType, "int expected");
        int threadId = threadIdValue.AsInt();
        WaitingSetter waiting(frame.GetThread());
        GetMachine().JoinThread(threadId);
        GetMachine().GetGarbageCollector().WaitForIdle(frame.GetThread());
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
    catch (const ThreadingException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowThreadingException(ex, frame);
    }
}

class VmSystemThreadingEnterMonitor : public VmFunction
{
public:
    VmSystemThreadingEnterMonitor(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingEnterMonitor::VmSystemThreadingEnterMonitor(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"enterm"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemThreadingEnterMonitor::Execute(Frame& frame)
{
    try
    {
        IntegralValue lockValue = frame.Local(0).GetValue();
        ObjectReference lockReference(lockValue.Value());
        if (lockReference.IsNull())
        {
            throw NullReferenceException("tried to enter monitor with null object reference");
        }
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        void* lockObject = memoryPool.GetObject(lockReference, lock);
        ManagedAllocationHeader* header = GetAllocationHeader(lockObject);
        header->Reference();
        lock.unlock();
        Thread& thread = frame.GetThread();
        WaitingSetter waiting(thread);
        EnterMonitor(header);
        lockObject = memoryPool.GetObject(lockReference, lock);
        header = GetAllocationHeader(lockObject);
        header->Unreference();
        GetMachine().GetGarbageCollector().WaitForIdle(thread);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
}

class VmSystemThreadingExitMonitor : public VmFunction
{
public:
    VmSystemThreadingExitMonitor(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingExitMonitor::VmSystemThreadingExitMonitor(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"exitm"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemThreadingExitMonitor::Execute(Frame& frame)
{
    try
    {
        IntegralValue lockValue = frame.Local(0).GetValue();
        ObjectReference lockReference(lockValue.Value());
        if (lockReference.IsNull())
        {
            throw NullReferenceException("tried to exit monitor with null object reference");
        }
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        void* lockObject = memoryPool.GetObject(lockReference, lock);
        ManagedAllocationHeader* header = GetAllocationHeader(lockObject);
        header->Reference();
        lock.unlock();
        ExitMonitor(header);
        lockObject = memoryPool.GetObject(lockReference, lock);
        header = GetAllocationHeader(lockObject);
        header->Unreference();
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
}

class VmSystemThreadingConditionVariableConstructor : public VmFunction
{
public:
    VmSystemThreadingConditionVariableConstructor(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingConditionVariableConstructor::VmSystemThreadingConditionVariableConstructor(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"initcv"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemThreadingConditionVariableConstructor::Execute(Frame& frame)
{
    try
    {
        IntegralValue condVarValue = frame.Local(0).GetValue();
        ObjectReference condVarRefefence(condVarValue.Value());
        if (condVarRefefence.IsNull())
        {
            throw NullReferenceException("tried to construct a condition variable with a null reference");
        }
        int32_t condVarId = CreateConditionVariable();
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        void* condVarObject = memoryPool.GetObject(condVarRefefence, lock);
        ManagedAllocationHeader* header = GetAllocationHeader(condVarObject);
        header->SetConditionVariable();
        memoryPool.SetField(condVarRefefence, 1, MakeIntegralValue<int32_t>(condVarId, ValueType::intType), lock);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
}

class VmSystemThreadingConditionVariableNotifyOne : public VmFunction
{
public:
    VmSystemThreadingConditionVariableNotifyOne(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingConditionVariableNotifyOne::VmSystemThreadingConditionVariableNotifyOne(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"notifyone"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemThreadingConditionVariableNotifyOne::Execute(Frame& frame)
{
    try
    {
        IntegralValue condVarValue = frame.Local(0).GetValue();
        ObjectReference condVarRefefence(condVarValue.Value());
        if (condVarRefefence.IsNull())
        {
            throw NullReferenceException("tried to notify a condition variable with a null object reference");
        }
        IntegralValue condVarIdValue = GetManagedMemoryPool().GetField(condVarRefefence, 1);
        Assert(condVarIdValue.GetType() == ValueType::intType, "int expected");
        int32_t condVarId = condVarIdValue.AsInt();
        NotifyOne(condVarId);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
}

class VmSystemThreadingConditionVariableNotifyAll : public VmFunction
{
public:
    VmSystemThreadingConditionVariableNotifyAll(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingConditionVariableNotifyAll::VmSystemThreadingConditionVariableNotifyAll(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"notifyall"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemThreadingConditionVariableNotifyAll::Execute(Frame& frame)
{
    try
    {
        IntegralValue condVarValue = frame.Local(0).GetValue();
        ObjectReference condVarRefefence(condVarValue.Value());
        if (condVarRefefence.IsNull())
        {
            throw NullReferenceException("tried to notify a condition variable with a null object reference");
        }
        IntegralValue condVarIdValue = GetManagedMemoryPool().GetField(condVarRefefence, 1);
        Assert(condVarIdValue.GetType() == ValueType::intType, "int expected");
        int32_t condVarId = condVarIdValue.AsInt();
        NotifyAll(condVarId);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
}

class VmSystemThreadingConditionVariableWait : public VmFunction
{
public:
    VmSystemThreadingConditionVariableWait(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingConditionVariableWait::VmSystemThreadingConditionVariableWait(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"waitcv"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemThreadingConditionVariableWait::Execute(Frame& frame)
{
    try
    {
        IntegralValue condVarValue = frame.Local(0).GetValue();
        ObjectReference condVarRefefence(condVarValue.Value());
        if (condVarRefefence.IsNull())
        {
            throw NullReferenceException("tried to wait a condition variable whose value is null object reference");
        }
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        IntegralValue condVarIdValue = memoryPool.GetField(condVarRefefence, 1, lock);
        Assert(condVarIdValue.GetType() == ValueType::intType, "int expected");
        int32_t condVarId = condVarIdValue.AsInt();
        IntegralValue lockObjectValue = frame.Local(1).GetValue();
        ObjectReference lockObjectReference(lockObjectValue.Value());
        if (lockObjectReference.IsNull())
        {
            throw NullReferenceException("tried to wait a condition variable with null lock object");
        }
        void* lockObject = memoryPool.GetObject(lockObjectReference, lock);
        ManagedAllocationHeader* header = GetAllocationHeader(lockObject);
        if (header->LockId() == lockNotAllocated)
        {
            throw ThreadingException("condition variable must be waited inside a lock statement");
        }
        int32_t lockId = header->LockId();
        lock.unlock();
        Thread& thread = frame.GetThread();
        WaitingSetter waiting(thread);
        WaitConditionVariable(condVarId, lockId);
        GetMachine().GetGarbageCollector().WaitForIdle(thread);
    }
    catch (const ThreadingException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowThreadingException(ex, frame);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
}

class VmSystemThreadingConditionVariableWaitFor : public VmFunction
{
public:
    VmSystemThreadingConditionVariableWaitFor(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingConditionVariableWaitFor::VmSystemThreadingConditionVariableWaitFor(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"waitcvfor"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemThreadingConditionVariableWaitFor::Execute(Frame& frame)
{
    try
    {
        IntegralValue condVarValue = frame.Local(0).GetValue();
        ObjectReference condVarRefefence(condVarValue.Value());
        if (condVarRefefence.IsNull())
        {
            throw NullReferenceException("tried to wait a condition variable whose value is null object reference");
        }
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        IntegralValue condVarIdValue = memoryPool.GetField(condVarRefefence, 1, lock);
        Assert(condVarIdValue.GetType() == ValueType::intType, "int expected");
        int32_t condVarId = condVarIdValue.AsInt();
        IntegralValue lockObjectValue = frame.Local(1).GetValue();
        ObjectReference lockObjectReference(lockObjectValue.Value());
        if (lockObjectReference.IsNull())
        {
            throw NullReferenceException("tried to wait a condition variable with null lock object");
        }
        void* lockObject = memoryPool.GetObject(lockObjectReference, lock);
        ManagedAllocationHeader* header = GetAllocationHeader(lockObject);
        if (header->LockId() == lockNotAllocated)
        {
            throw ThreadingException("condition variable must be waited inside a lock statement");
        }
        IntegralValue durationValue(frame.Local(1).GetValue());
        ObjectReference durationRef(durationValue.Value());
        IntegralValue nanosecondValue = memoryPool.GetField(durationRef, 1, lock);
        int64_t nanosecondCount = nanosecondValue.AsLong();
        if (nanosecondCount < 0) return;
        std::chrono::nanoseconds duration{ nanosecondCount };
        Thread& thread = frame.GetThread();
        WaitingSetter waiting(thread);
        int32_t lockId = header->LockId();
        lock.unlock();
        CondVarStatus status = WaitConditionVariable(condVarId, lockId, duration);
        GetMachine().GetGarbageCollector().WaitForIdle(thread);
        frame.OpStack().Push(MakeIntegralValue<uint8_t>(uint8_t(status), ValueType::byteType));
    }
    catch (const std::exception& ex)
    {
        if (RunningNativeCode())
        {
            throw SystemException(ex.what());
        }
        ThrowSystemException(SystemException(ex.what()), frame);
    }
    catch (const ThreadingException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowThreadingException(ex, frame);
    }
    catch (const NullReferenceException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        ThrowNullReferenceException(ex, frame);
    }
}

class VmSystemThreadingHardwareConcurrency : public VmFunction
{
public:
    VmSystemThreadingHardwareConcurrency(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemThreadingHardwareConcurrency::VmSystemThreadingHardwareConcurrency(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"cores"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemThreadingHardwareConcurrency::Execute(Frame& frame)
{
    int32_t cores = std::thread::hardware_concurrency();
    frame.OpStack().Push(MakeIntegralValue<int32_t>(cores, ValueType::intType));
}

class VmSystemTimePointNow : public VmFunction
{
public:
    VmSystemTimePointNow(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemTimePointNow::VmSystemTimePointNow(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"now"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemTimePointNow::Execute(Frame& frame)
{
    try
    {
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        ClassData* timePointClassData = ClassDataTable::GetClassData(U"System.TimePoint");
        ObjectType* timePointType = timePointClassData->Type();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        ObjectReference timePointReference = memoryPool.CreateObject(frame.GetThread(), timePointType, lock);
        memoryPool.SetField(timePointReference, 0, IntegralValue(timePointClassData), lock);
        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock().now();
        int64_t nanoseconds = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count();
        memoryPool.SetField(timePointReference, 1, MakeIntegralValue<int64_t>(nanoseconds, ValueType::longType), lock);
        frame.OpStack().Push(timePointReference);
    }
    catch (const std::exception& ex)
    {
        if (RunningNativeCode())
        {
            throw SystemException(ex.what());
        }
        ThrowSystemException(SystemException(ex.what()), frame);
    }
}

class VmSystemSleepFor : public VmFunction
{
public:
    VmSystemSleepFor(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemSleepFor::VmSystemSleepFor(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"sleep"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemSleepFor::Execute(Frame& frame)
{
    try
    {
        IntegralValue durationValue = frame.Local(0).GetValue();
        ObjectReference durationRef(durationValue.Value());
        IntegralValue nanosecondValue = GetManagedMemoryPool().GetField(durationRef, 1);
        int64_t nanosecondCount = nanosecondValue.AsLong();
        if (nanosecondCount < 0) return;
        std::chrono::nanoseconds duration{ nanosecondCount };
        Thread& thread = frame.GetThread();
        WaitingSetter waiting(thread);
        std::this_thread::sleep_for(duration);
        GetMachine().GetGarbageCollector().WaitForIdle(thread);
    }
    catch (const std::exception& ex)
    {
        if (RunningNativeCode())
        {
            throw SystemException(ex.what());
        }
        ThrowSystemException(SystemException(ex.what()), frame);
    }
}

class VmSystemRethrow : public VmFunction
{
public:
    VmSystemRethrow(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemRethrow::VmSystemRethrow(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"rethrow"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemRethrow::Execute(Frame& frame)
{
    try
    {
        IntegralValue exceptionValue = frame.Local(0).GetValue();
        uint64_t exception = exceptionValue.AsULong();
        throw CapturedException(exception);
    }
    catch (const CapturedException& ex)
    {
        if (RunningNativeCode())
        {
            throw;
        }
        frame.GetThread().HandleException(ex.GetException());
    }
}

class VmSystemInitRand : public VmFunction
{
public:
    VmSystemInitRand(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemInitRand::VmSystemInitRand(ConstantPool& constantPool) 
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"initrand"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemInitRand::Execute(Frame& frame)
{
    IntegralValue seedValue = frame.Local(0).GetValue();
    Assert(seedValue.GetType() == ValueType::uintType, "uint expected");
    uint32_t seed = seedValue.AsUInt();
    InitMt(seed);
}

class VmSystemRandom : public VmFunction
{
public:
    VmSystemRandom(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemRandom::VmSystemRandom(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"random"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemRandom::Execute(Frame& frame)
{
    uint32_t r = Random();
    frame.OpStack().Push(MakeIntegralValue<uint32_t>(r, ValueType::uintType));
}

class VmSystemRand64 : public VmFunction
{
public:
    VmSystemRand64(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemRand64::VmSystemRand64(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"rand64"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemRand64::Execute(Frame& frame)
{
    uint64_t r = Random64();
    frame.OpStack().Push(MakeIntegralValue<uint64_t>(r, ValueType::ulongType));
}

class VmSystemUIntAsFloat : public VmFunction
{
public:
    VmSystemUIntAsFloat(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemUIntAsFloat::VmSystemUIntAsFloat(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"uintasfloat"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemUIntAsFloat::Execute(Frame& frame)
{
    IntegralValue uintValue = frame.Local(0).GetValue();
    Assert(uintValue.GetType() == ValueType::uintType, "uint expected");
    IntegralValue floatValue = MakeIntegralValue<float>(*static_cast<float*>(uintValue.ValuePtr()), ValueType::floatType);
    frame.OpStack().Push(floatValue);
}

class VmSystemFloatAsUInt : public VmFunction
{
public:
    VmSystemFloatAsUInt(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemFloatAsUInt::VmSystemFloatAsUInt(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"floatasuint"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemFloatAsUInt::Execute(Frame& frame)
{
    IntegralValue floatValue = frame.Local(0).GetValue();
    Assert(floatValue.GetType() == ValueType::floatType, "float expected");
    IntegralValue uintValue = MakeIntegralValue<uint32_t>(*static_cast<uint32_t*>(floatValue.ValuePtr()), ValueType::uintType);
    frame.OpStack().Push(uintValue);
}

class VmSystemULongAsDouble : public VmFunction
{
public:
    VmSystemULongAsDouble(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemULongAsDouble::VmSystemULongAsDouble(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"ulongasdouble"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemULongAsDouble::Execute(Frame& frame)
{
    IntegralValue ulongValue = frame.Local(0).GetValue();
    Assert(ulongValue.GetType() == ValueType::ulongType , "ulong expected");
    IntegralValue doubleValue = MakeIntegralValue<double>(*static_cast<double*>(ulongValue.ValuePtr()), ValueType::doubleType);
    frame.OpStack().Push(doubleValue);
}

class VmSystemDoubleAsULong : public VmFunction
{
public:
    VmSystemDoubleAsULong(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemDoubleAsULong::VmSystemDoubleAsULong(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"doubleasulong"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemDoubleAsULong::Execute(Frame& frame)
{
    IntegralValue doubleValue = frame.Local(0).GetValue();
    Assert(doubleValue.GetType() == ValueType::doubleType, "double expected");
    IntegralValue ulongValue = MakeIntegralValue<uint64_t>(*static_cast<uint64_t*>(doubleValue.ValuePtr()), ValueType::ulongType);
    frame.OpStack().Push(ulongValue);
}

class VmSystemCurrentDate : public VmFunction
{
public:
    VmSystemCurrentDate(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemCurrentDate::VmSystemCurrentDate(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"curdate"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemCurrentDate::Execute(Frame& frame)
{
    time_t rawTime;
    struct tm* timeInfo;
    time(&rawTime);
    timeInfo = gmtime(&rawTime);
    int year = 1900 + timeInfo->tm_year;
    int month = 1 + timeInfo->tm_mon;
    int day = timeInfo->tm_mday;
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    ClassData* dateClassData = ClassDataTable::GetClassData(U"System.Date");
    ObjectType* dateType = dateClassData->Type();
    ObjectReference date = memoryPool.CreateObject(frame.GetThread(), dateType);
    memoryPool.SetField(date, 0, IntegralValue(dateClassData));
    memoryPool.SetField(date, 1, MakeIntegralValue<uint16_t>(static_cast<uint16_t>(year), ValueType::ushortType));
    memoryPool.SetField(date, 2, MakeIntegralValue<uint8_t>(static_cast<uint8_t>(month), ValueType::byteType));
    memoryPool.SetField(date, 3, MakeIntegralValue<uint8_t>(static_cast<uint8_t>(day), ValueType::byteType));
    frame.OpStack().Push(date);
}

class VmSystemCurrentUtcTime : public VmFunction
{
public:
    VmSystemCurrentUtcTime(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemCurrentUtcTime::VmSystemCurrentUtcTime(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"curutctime"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemCurrentUtcTime::Execute(Frame& frame)
{
    time_t rawTime;
    struct tm* timeInfo;
    time(&rawTime);
    timeInfo = gmtime(&rawTime);
    int hour = timeInfo->tm_hour;
    int min = timeInfo->tm_min;
    int sec = timeInfo->tm_sec;
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    ClassData* timeOfDayClassData = ClassDataTable::GetClassData(U"System.TimeOfDay");
    ObjectType* timeOfDayType = timeOfDayClassData->Type();
    ObjectReference timeOfDay = memoryPool.CreateObject(frame.GetThread(), timeOfDayType);
    memoryPool.SetField(timeOfDay, 0, IntegralValue(timeOfDayClassData));
    memoryPool.SetField(timeOfDay, 1, MakeIntegralValue<uint8_t>(static_cast<uint8_t>(hour), ValueType::byteType));
    memoryPool.SetField(timeOfDay, 2, MakeIntegralValue<uint8_t>(static_cast<uint8_t>(min), ValueType::byteType));
    memoryPool.SetField(timeOfDay, 3, MakeIntegralValue<uint8_t>(static_cast<uint8_t>(sec), ValueType::byteType));
    ObjectReference zoneObjectRef = memoryPool.CreateString(frame.GetThread(), U"UT");
    memoryPool.SetField(timeOfDay, 4, zoneObjectRef);
    frame.OpStack().Push(timeOfDay);
}

class VmSystemCurrentLocalTime : public VmFunction
{
public:
    VmSystemCurrentLocalTime(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemCurrentLocalTime::VmSystemCurrentLocalTime(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"curlocaltime"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemCurrentLocalTime::Execute(Frame& frame)
{
    time_t rawTime;
    struct tm* timeInfo;
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    int hour = timeInfo->tm_hour;
    int min = timeInfo->tm_min;
    int sec = timeInfo->tm_sec;
    ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
    ClassData* timeOfDayClassData = ClassDataTable::GetClassData(U"System.TimeOfDay");
    ObjectType* timeOfDayType = timeOfDayClassData->Type();
    ObjectReference timeOfDay = memoryPool.CreateObject(frame.GetThread(), timeOfDayType);
    memoryPool.SetField(timeOfDay, 0, IntegralValue(timeOfDayClassData));
    memoryPool.SetField(timeOfDay, 1, MakeIntegralValue<uint8_t>(static_cast<uint8_t>(hour), ValueType::byteType));
    memoryPool.SetField(timeOfDay, 2, MakeIntegralValue<uint8_t>(static_cast<uint8_t>(min), ValueType::byteType));
    memoryPool.SetField(timeOfDay, 3, MakeIntegralValue<uint8_t>(static_cast<uint8_t>(sec), ValueType::byteType));
    frame.OpStack().Push(timeOfDay);
}

class VmFunctionPool
{
public:
    static void Init();
    static void Done();
    static VmFunctionPool& Instance() { Assert(instance, "vm function pool not initialized"); return *instance; }
    void CreateVmFunctions(ConstantPool& constantPool);
private:
    static std::unique_ptr<VmFunctionPool> instance;
    std::vector<std::unique_ptr<VmFunction>> vmFunctions;
    VmFunctionPool();
};

std::unique_ptr<VmFunctionPool> VmFunctionPool::instance;

VmFunctionPool::VmFunctionPool()
{
}

void VmFunctionPool::Init()
{
    instance.reset(new VmFunctionPool());
}

void VmFunctionPool::Done()
{
    instance.reset();
}

void VmFunctionPool::CreateVmFunctions(ConstantPool& constantPool)
{
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemPlatform(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemPowDoubleInt(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsCSpaceChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsCAlphaChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsCAlnumChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsCDigitChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsCHexDigitChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsCPunctuationChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsCPrintableChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsCLowerChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsCUpperChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemToCLowerChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemToCUpperChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemObjectToString(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemObjectGetHashCode(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemObjectEqual(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemStringConstructorCharArray(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOOpenFile(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOCloseFile(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOWriteByteToFile(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOWriteFile(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOReadByteFromFile(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOReadFile(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOSeekFile(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOTellFile(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOFileExists(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOLastWriteTimeLess(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemNetSocketsCreateSocket(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemNetSocketsConnectSocket(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemNetSocketsCloseSocket(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemNetSocketsBindSocket(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemNetSocketsListenSocket(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemNetSocketsAcceptSocket(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemNetSocketsShutdownSocket(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemNetSocketsSendSocket(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemNetSocketsReceiveSocket(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemGetEnvironmentVariable(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemGetPathSeparatorChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOInternalGetCurrentWorkingDirectory(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingStartThreadWithThreadStartFunction(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingStartThreadWithThreadStartMethod(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingStartThreadWithParameterizedThreadStartFunction(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingStartThreadWithParameterizedThreadStartMethod(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingJoinThread(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingEnterMonitor(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingExitMonitor(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingConditionVariableConstructor(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingConditionVariableNotifyOne(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingConditionVariableNotifyAll(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingConditionVariableWait(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingConditionVariableWaitFor(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemTimePointNow(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemSleepFor(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemThreadingHardwareConcurrency(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemRethrow(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemInitRand(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemRandom(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemRand64(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemUIntAsFloat(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemULongAsDouble(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemFloatAsUInt(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemDoubleAsULong(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemCurrentDate(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemCurrentUtcTime(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemCurrentLocalTime(constantPool)));
}

void InitVmFunctions(ConstantPool& vmFunctionNamePool)
{
    VmFunctionPool::Init();
    VmFunctionPool::Instance().CreateVmFunctions(vmFunctionNamePool);
}

void DoneVmFunctions()
{
    VmFunctionPool::Done();
}

} } // namespace cminor::vmlib
