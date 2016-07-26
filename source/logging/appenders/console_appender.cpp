/*!
    \file console_appender.cpp
    \brief Logging console appender implementation
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#include "logging/appenders/console_appender.h"

#include "errors/exceptions.h"

#include <cstdio>

namespace CppLogging {

void ConsoleAppender::AppendRecord(Record& record)
{
    // Skip logging records without layout
    if (record.raw.first == nullptr)
        return;

    std::fwrite(record.raw.first, 1, record.raw.second, stdout);
}

void ConsoleAppender::Flush()
{
    std::fflush(stdout);
}

} // namespace CppLogging
