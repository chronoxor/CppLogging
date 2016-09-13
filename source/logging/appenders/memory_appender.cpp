/*!
    \file memory_appender.cpp
    \brief Memory appender implementation
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#include "logging/appenders/memory_appender.h"

namespace CppLogging {

void MemoryAppender::AppendRecord(Record& record)
{
    // Skip logging records without layout
    if (record.raw.empty())
        return;

    // Append logging record content into the allocated memory buffer
    _buffer.insert(_buffer.end(), record.raw.begin(), record.raw.end());
}

} // namespace CppLogging
