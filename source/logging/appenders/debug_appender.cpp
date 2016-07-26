/*!
    \file debug_appender.cpp
    \brief Logging debug appender implementation
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#include "logging/appenders/debug_appender.h"

#include "errors/exceptions.h"

#include <cstdio>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace CppLogging {

void DebugAppender::AppendRecord(Record& record)
{
    // Skip logging records without layout
    if (record.raw.first == nullptr)
        return;

#if defined(_WIN32) || defined(_WIN64)
    OutputDebugStringA((LPCSTR)record.raw.first);
#else
    std::fwrite(record.raw.first, 1, record.raw.second, stdout);
#endif
}

void DebugAppender::Flush()
{
#if defined(_WIN32) || defined(_WIN64)
    // Do nothing here...
#else
    std::fflush(stdout);
#endif
}

} // namespace CppLogging
