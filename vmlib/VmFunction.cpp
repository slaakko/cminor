// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/vmlib/VmFunction.hpp>
#include <cminor/vmlib/File.hpp>
#include <cminor/vmlib/Threading.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/machine/Type.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/Runtime.hpp>
#include <boost/filesystem.hpp>
#include <cctype>
#include <chrono>
#include <thread>

namespace cminor { namespace vmlib {

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
#pragma warning(disable : 4244)
    frame.OpStack().Push(IntegralValue(result, ValueType::doubleType));
#pragma warning(default : 4244)
}

class VmSystemIsSpaceChar : public VmFunction
{
public:
    VmSystemIsSpaceChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsSpaceChar::VmSystemIsSpaceChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"isspace"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsSpaceChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isSpace = std::isspace(char(c));
    frame.OpStack().Push(IntegralValue(isSpace, ValueType::boolType));
}

class VmSystemIsLetterChar : public VmFunction
{
public:
    VmSystemIsLetterChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsLetterChar::VmSystemIsLetterChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"isletter"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsLetterChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isLetter = std::isalpha(char(c));
    frame.OpStack().Push(IntegralValue(isLetter, ValueType::boolType));
}

class VmSystemIsDigitChar : public VmFunction
{
public:
    VmSystemIsDigitChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsDigitChar::VmSystemIsDigitChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"isdigit"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsDigitChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isDigit = std::isdigit(char(c));
    frame.OpStack().Push(IntegralValue(isDigit, ValueType::boolType));
}

class VmSystemIsHexDigitChar : public VmFunction
{
public:
    VmSystemIsHexDigitChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsHexDigitChar::VmSystemIsHexDigitChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"isxdigit"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsHexDigitChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isHexDigit = std::isxdigit(char(c));
    frame.OpStack().Push(IntegralValue(isHexDigit, ValueType::boolType));
}

class VmSystemIsPunctuationChar : public VmFunction
{
public:
    VmSystemIsPunctuationChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsPunctuationChar::VmSystemIsPunctuationChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"ispunct"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsPunctuationChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isPunctuation = std::ispunct(char(c));
    frame.OpStack().Push(IntegralValue(isPunctuation, ValueType::boolType));
}

class VmSystemIsPrintableChar : public VmFunction
{
public:
    VmSystemIsPrintableChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemIsPrintableChar::VmSystemIsPrintableChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"isprint"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemIsPrintableChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    bool isPrintable = std::isprint(char(c));
    frame.OpStack().Push(IntegralValue(isPrintable, ValueType::boolType));
}

class VmSystemToLowerChar : public VmFunction
{
public:
    VmSystemToLowerChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemToLowerChar::VmSystemToLowerChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"tolower"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemToLowerChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    c = std::tolower(char(c));
    frame.OpStack().Push(IntegralValue(c, ValueType::charType));
}

class VmSystemToUpperChar : public VmFunction
{
public:
    VmSystemToUpperChar(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemToUpperChar::VmSystemToUpperChar(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"toupper"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemToUpperChar::Execute(Frame& frame)
{
    IntegralValue value = frame.Local(0).GetValue();
    char32_t c = value.AsChar();
    c = std::toupper(char(c));
    frame.OpStack().Push(IntegralValue(c, ValueType::charType));
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
        frame.OpStack().Push(IntegralValue(0, ValueType::ulongType));
    }
    else
    {
        ManagedMemoryPool& memoryPool = GetManagedMemoryPool();
        std::unique_lock<std::recursive_mutex> lock(memoryPool.AllocationsMutex());
        void* object = memoryPool.GetObject(reference, lock);
        ManagedAllocationHeader* header = GetAllocationHeader(object);
        ObjectHeader* objectHeader = &header->objectHeader;
        uint64_t hashCode = objectHeader->HashCode();
        frame.OpStack().Push(IntegralValue(hashCode, ValueType::ulongType));
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
        frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
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
        frame.OpStack().Push(IntegralValue(result, ValueType::boolType));
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
        memoryPool.SetField(stringRef, 1, IntegralValue(stringLength, ValueType::intType), lock);
        memoryPool.SetField(stringRef, 2, IntegralValue(stringCharactersHandle.Value(), ValueType::allocationHandle), lock);
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
        frame.OpStack().Push(IntegralValue(fileHandle, ValueType::intType));
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
        frame.OpStack().Push(IntegralValue(value, ValueType::intType));
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
        frame.OpStack().Push(IntegralValue(result, ValueType::intType));
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
        frame.OpStack().Push(IntegralValue(exists, ValueType::boolType));
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
        frame.OpStack().Push(IntegralValue(less, ValueType::boolType));
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
        utf32_string us = ToUtf32(s);
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
    frame.OpStack().Push(IntegralValue(s, ValueType::charType));
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
    utf32_string s = ToUtf32(workingDirectory);
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
        memoryPool.SetField(threadReference, 1, IntegralValue(threadId, ValueType::intType), lock);
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
        memoryPool.SetField(threadReference, 1, IntegralValue(threadId, ValueType::intType), lock);
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
        memoryPool.SetField(threadReference, 1, IntegralValue(threadId, ValueType::intType), lock);
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
        memoryPool.SetField(threadReference, 1, IntegralValue(threadId, ValueType::intType), lock);
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
            thread.SetFunctionStack(RtGetFunctionStack());
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
        memoryPool.SetField(condVarRefefence, 1, IntegralValue(condVarId, ValueType::intType), lock);
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
        frame.OpStack().Push(IntegralValue(static_cast<uint8_t>(status), ValueType::byteType));
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
    int cores = std::thread::hardware_concurrency();
    frame.OpStack().Push(IntegralValue(cores, ValueType::intType));
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
        memoryPool.SetField(timePointReference, 1, IntegralValue(static_cast<uint64_t>(nanoseconds), ValueType::longType), lock);
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
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemPowDoubleInt(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsSpaceChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsLetterChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsDigitChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsHexDigitChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsPunctuationChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIsPrintableChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemToLowerChar(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemToUpperChar(constantPool)));
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
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOFileExists(constantPool)));
    vmFunctions.push_back(std::unique_ptr<VmFunction>(new VmSystemIOLastWriteTimeLess(constantPool)));
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
