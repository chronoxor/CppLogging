/*!
    \file file_appender.cpp
    \brief File appender implementation
    \author Ivan Shynkarenka
    \date 07.09.2016
    \copyright MIT License
*/

#include "logging/appenders/file_appender2.h"

namespace CppLogging {

FileAppender2::FileAppender2(const CppCommon::File& file, bool truncate, bool auto_flush)
    : _retry(0), _truncate(truncate), _auto_flush(auto_flush)
{
    _file = fopen(file.native().c_str(), "ab");
}

void FileAppender2::AppendRecord(Record& record)
{
    // Skip logging records without layout
    if (record.raw.empty())
        return;

    if (CheckFileIsOpened())
    {
        if (std::fwrite(record.raw.data(), 1, record.raw.size() - 1, _file) != (record.raw.size() - 1))
            return;
        // Perform auto-flush if enabled
        if (_auto_flush)
            std::fflush(_file);
    }
}

void FileAppender2::Flush()
{
    if (CheckFileIsOpened())
    {
        // Try to flush the opened file
        std::fflush(_file);
    }
}

bool FileAppender2::CheckFileIsOpened()
{
    return true;
}

} // namespace CppLogging
