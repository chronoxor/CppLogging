/*!
    \file error_appender.cpp
    \brief Error (stderr) appender implementation
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#include "logging/appenders/error_appender.h"

#include <cstdio>

namespace CppLogging {

void ErrorAppender::AppendRecord(Record& record)
{
    // Skip logging records without layout
    if (record.raw.empty())
        return;

    // Append logging record content
    std::fwrite(record.raw.data(), 1, record.raw.size() - 1, stderr);
}

void ErrorAppender::Flush()
{
    // Flush stream
    std::fflush(stderr);
}

} // namespace CppLogging
