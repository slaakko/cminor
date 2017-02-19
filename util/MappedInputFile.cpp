// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/util/MappedInputFile.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

namespace cminor { namespace util {

class MappedInputFileImpl
{
public:
    MappedInputFileImpl(const std::string& fileName_);
    const char* Data() const { return mappedFile.data(); }
    boost::iostreams::mapped_file_source::size_type Size() const { return mappedFile.size(); }
private:
    boost::iostreams::mapped_file_source mappedFile;
};

MappedInputFileImpl::MappedInputFileImpl(const std::string& fileName_) : mappedFile(fileName_)
{
}

MappedInputFile::MappedInputFile(const std::string& fileName_) : impl(new MappedInputFileImpl(fileName_))
{
}

MappedInputFile::~MappedInputFile()
{
    delete impl;
}

const char* MappedInputFile::Begin() const
{
    const char* start = impl->Data();
    if (impl->Size() >= 3)
    {
        if ((unsigned char)start[0] == (unsigned char)0xEF &&
            (unsigned char)start[1] == (unsigned char)0xBB &&
            (unsigned char)start[2] == (unsigned char)0xBF)
        {
            start += 3;
        }
    }
    return start;
}

const char* MappedInputFile::End() const
{
    return impl->Data() + impl->Size();
}

} } // namespace cminor::util
