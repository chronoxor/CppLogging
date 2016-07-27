/*!
    \file memory_appender.cpp
    \brief Logging memory appender implementation
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#include "logging/appenders/memory_appender.h"

#include <cstdio>

namespace CppLogging {

void MemoryAppender::AppendRecord(Record& record)
{
    // Skip logging records without layout
    if (record.raw.first == nullptr)
        return;

    // Resize the buffer to required size
    _buffer.resize(record.raw.second);

    // Get the buffer start position
    uint8_t* buffer = _buffer.data();

    // Copy logging record content into the allocated memory buffer
    std::memcpy(buffer, record.raw.first, record.raw.second);
}

} // namespace CppLogging
