// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/vmlib/VmFunction.hpp>
#include <cminor/vmlib/File.hpp>
#include <cminor/machine/Function.hpp>
#include <cminor/machine/Class.hpp>
#include <cminor/machine/Type.hpp>
#include <cminor/machine/Machine.hpp>
#include <cminor/machine/RunTime.hpp>
#include <boost/filesystem.hpp>
#include <cctype>

namespace cminor { namespace vmlib {

class VmSystemPowDoubleInt : public VmFunction
{
public:
    VmSystemPowDoubleInt(ConstantPool& constantPool);
    void Execute(Frame& frame) override;
};

VmSystemPowDoubleInt::VmSystemPowDoubleInt(ConstantPool& constantPool)
{
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Pow.Double.Int"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.IsSpace.Char"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.IsLetter.Char"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.IsDigit.Char"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.IsHexDigit.Char"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.IsPunctuation.Char"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.IsPrintable.Char"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.ToLower.Char"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.ToUpper.Char"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Object.ToString"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Object.GetHashCode"));
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
        MemPtr memPtr = GetManagedMemoryPool().GetMemPtr(reference);
        uint64_t hashCode = memPtr.HashCode();
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Object.Equal"));
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
            Object& left = GetManagedMemoryPool().GetObject(leftRef);
            MemPtr leftMem = left.GetMemPtr();
            Object& right = GetManagedMemoryPool().GetObject(rightRef);
            MemPtr rightMem = right.GetMemPtr();
            result = leftMem == rightMem;
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Object.Less"));
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
            Object& left = GetManagedMemoryPool().GetObject(leftRef);
            MemPtr leftMem = left.GetMemPtr();
            Object& right = GetManagedMemoryPool().GetObject(rightRef);
            MemPtr rightMem = right.GetMemPtr();
            result = leftMem < rightMem;
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"System.String.Constructor.CharArray"));
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
        std::pair<AllocationHandle, int32_t> stringCharactersHandleStringSLengthPair = GetManagedMemoryPool().CreateStringCharsFromCharArray(frame.GetThread(), charArrayRef);
        AllocationHandle stringCharactersHandle = stringCharactersHandleStringSLengthPair.first;
        int32_t stringLength = stringCharactersHandleStringSLengthPair.second;
        Object& str = GetManagedMemoryPool().GetObject(stringRef);
        str.SetField(IntegralValue(stringLength, ValueType::intType), 1);
        str.SetField(IntegralValue(stringCharactersHandle.Value(), ValueType::allocationHandle), 2);
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"System.IO.OpenFile"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"System.IO.CloseFile"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"System.IO.WriteByteToFile"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"System.IO.WriteFile"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"System.IO.ReadByteFromFile"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"System.IO.ReadFile"));
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
        bytes.resize(GetManagedMemoryPool().GetNumArrayElements(buffer));
        int32_t result = ReadFile(fileHandle, &bytes[0], int32_t(bytes.size()));
        if (result > 0)
        {
            GetManagedMemoryPool().SetBytes(buffer, bytes, result);
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"System.IO.File.Exists"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"System.IO.File.LastWriteTimeLess"));
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
        std::string firstFilePath = GetManagedMemoryPool().GetUtf8String(firstFilePathStr);
        IntegralValue secondFilePathValue = frame.Local(1).GetValue();
        Assert(secondFilePathValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference secondFilePathStr(secondFilePathValue.Value());
        std::string secondFilePath = GetManagedMemoryPool().GetUtf8String(secondFilePathStr);
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.GetEnvironmentVariable"));
    SetName(name);
    VmFunctionTable::RegisterVmFunction(this);
}

void VmSystemGetEnvironmentVariable::Execute(Frame& frame)
{
    IntegralValue environmentVarNameValue = frame.Local(0).GetValue();
    Assert(environmentVarNameValue.GetType() == ValueType::objectReference, "object reference expected");
    ObjectReference environmentVarName(environmentVarNameValue.Value());
    std::string envVarName = GetManagedMemoryPool().GetUtf8String(environmentVarName);
    char* value = std::getenv(envVarName.c_str());
    if (value)
    {
        std::string s(value);
        utf32_string us = ToUtf32(s);
        ObjectReference envVarValue = GetManagedMemoryPool().CreateString(frame.GetThread(), us);
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.GetPathSeparatorChar"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.IO.InternalGetCurrentWorkingDirectory"));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Threading.StartThread.ThreadStartFunction"));
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
        ObjectReference threadReference = GetManagedMemoryPool().CreateObject(frame.GetThread(), threadClassData->Type());
        GetManagedMemoryPool().SetField(threadReference, 0, IntegralValue(threadClassData));
        GetManagedMemoryPool().SetField(threadReference, 1, IntegralValue(threadId, ValueType::intType));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Threading.StartThread.ThreadStartMethod"));
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
        Object& object = GetManagedMemoryPool().GetObject(classDelegateObject);
        IntegralValue classObjectValue = object.GetField(1);
        Assert(classObjectValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference receiver(classObjectValue.Value());
        IntegralValue functionValue = object.GetField(2);
        Assert(functionValue.GetType() == ValueType::functionPtr, "function pointer expected");
        Function* fun = functionValue.AsFunctionPtr();
        int threadId = GetMachine().StartThread(fun, RunThreadKind::method, receiver, ObjectReference(0));
        ClassData* threadClassData = ClassDataTable::GetClassData(U"System.Threading.Thread");
        ObjectReference threadReference = GetManagedMemoryPool().CreateObject(frame.GetThread(), threadClassData->Type());
        GetManagedMemoryPool().SetField(threadReference, 0, IntegralValue(threadClassData));
        GetManagedMemoryPool().SetField(threadReference, 1, IntegralValue(threadId, ValueType::intType));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Threading.StartThread.ParameterizedThreadStartFunction"));
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
        ObjectReference threadReference = GetManagedMemoryPool().CreateObject(frame.GetThread(), threadClassData->Type());
        GetManagedMemoryPool().SetField(threadReference, 0, IntegralValue(threadClassData));
        GetManagedMemoryPool().SetField(threadReference, 1, IntegralValue(threadId, ValueType::intType));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Threading.StartThread.ParameterizedThreadStartMethod"));
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
        Object& object = GetManagedMemoryPool().GetObject(classDelegateObject);
        IntegralValue classObjectValue = object.GetField(1);
        Assert(classObjectValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference receiver(classObjectValue.Value());
        IntegralValue functionValue = object.GetField(2);
        Assert(functionValue.GetType() == ValueType::functionPtr, "function pointer expected");
        Function* fun = functionValue.AsFunctionPtr();
        IntegralValue argValue = frame.Local(1).GetValue();
        Assert(argValue.GetType() == ValueType::objectReference, "object reference expected");
        ObjectReference arg(argValue.Value());
        int threadId = GetMachine().StartThread(fun, RunThreadKind::methodWithParam, receiver, arg);
        ClassData* threadClassData = ClassDataTable::GetClassData(U"System.Threading.Thread");
        ObjectReference threadReference = GetManagedMemoryPool().CreateObject(frame.GetThread(), threadClassData->Type());
        GetManagedMemoryPool().SetField(threadReference, 0, IntegralValue(threadClassData));
        GetManagedMemoryPool().SetField(threadReference, 1, IntegralValue(threadId, ValueType::intType));
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
    Constant name = constantPool.GetConstant(constantPool.Install(U"Vm.System.Threading.JoinThread"));
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
            throw new NullReferenceException("tried to join null thread");
        }
        IntegralValue threadIdValue = GetManagedMemoryPool().GetField(threadRefefence, 1);
        Assert(threadIdValue.GetType() == ValueType::intType, "int expected");
        int threadId = threadIdValue.AsInt();
        WaitingSetter waiting(frame.GetThread());
        GetMachine().JoinThread(threadId);
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
