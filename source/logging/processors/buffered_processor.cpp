/*!
    \file buffered_processor.cpp
    \brief Buffered logging processor definition
    \author Ivan Shynkarenka
    \date 28.07.2016
    \copyright MIT License
*/

#include "logging/processors/buffered_processor.h"

namespace CppLogging {

bool BufferedProcessor::ProcessRecord(Record& record)
{
    // Process all buffered logging records if the buffer limit is reached
    if ((_buffer.size() + 1) > _limit)
        ProcessBufferedRecords();

    // Move the given logging record into the buffer
    _buffer.emplace_back(std::move(record));

    // Always return false to stop further logging record processing
    return false;
}

void BufferedProcessor::ProcessBufferedRecords()
{
    // Process all buffered logging records
    for (auto& record : _buffer)
        Processor::ProcessRecord(record);

    // Clear buffer
    _buffer.clear();
}

void BufferedProcessor::Flush()
{
    // Process all buffered logging records
    ProcessBufferedRecords();

    // Flush the logging processor
    Processor::Flush();
}

} // namespace CppLogging
