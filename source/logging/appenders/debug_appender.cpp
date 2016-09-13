/*!
    \file debug_appender.cpp
    \brief Debug appender implementation
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#include "logging/appenders/debug_appender.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <cstdio>
#endif

namespace CppLogging {

void DebugAppender::AppendRecord(Record& record)
{
    // Skip logging records without layout
    if (record.raw.empty())
        return;

#if defined(_WIN32) || defined(_WIN64)
    // Append logging record content
    OutputDebugStringA((LPCSTR)record.raw.data());
#else
    // Append logging record content
    std::fwrite(record.raw.data(), 1, record.raw.size() - 1, stdout);
#endif
}

void DebugAppender::Flush()
{
#if defined(_WIN32) || defined(_WIN64)
    // Do nothing here...
#else
    // Flush stream
    std::fflush(stdout);
#endif
}

} // namespace CppLogging
