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
    // Calculate logging record size
    size_t size = sizeof(uint64_t) + sizeof(uint64_t) + sizeof(Level) + sizeof(uint8_t) + record.logger.second + sizeof(uint16_t) + record.message.second + sizeof(uint32_t) + record.buffer.second + sizeof(uint32_t) + record.raw.second;

    // Process all buffered logging records if the buffer limit is reached
    size_t offset = _buffer.size();
    if ((offset + size) <= _limit)
        ProcessBufferedRecords();

    // Resize the buffer to required size
    _buffer.resize(offset + size);

    // Get the buffer start position
    uint8_t* buffer = _buffer.data() + offset;

    // Serialize logging record
    std::memcpy(buffer, &record.timestamp, sizeof(uint64_t));
    buffer += sizeof(uint64_t);
    std::memcpy(buffer, &record.thread, sizeof(uint64_t));
    buffer += sizeof(uint64_t);
    std::memcpy(buffer, &record.level, sizeof(Level));
    buffer += sizeof(Level);
    std::memcpy(buffer, &record.logger.second, sizeof(uint8_t));
    buffer += sizeof(uint8_t);
    std::memcpy(buffer, record.logger.first, record.logger.second);
    buffer += record.logger.second;
    std::memcpy(buffer, &record.message.second, sizeof(uint16_t));
    buffer += sizeof(uint16_t);
    std::memcpy(buffer, record.message.first, record.message.second);
    buffer += record.message.second;
    std::memcpy(buffer, &record.buffer.second, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    std::memcpy(buffer, record.buffer.first, record.buffer.second);
    buffer += record.buffer.second;
    std::memcpy(buffer, &record.raw.second, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    std::memcpy(buffer, record.raw.first, record.raw.second);
    buffer += record.buffer.second;

    return true;
}

void BufferedProcessor::ProcessBufferedRecords()
{
    // Get the buffer start and end positions
    uint8_t* buffer = _buffer.data();
    uint8_t* end = _buffer.data() + _buffer.size();

    // Process all buffered logging records
    while (buffer < end)
    {
        // Deserialize logging record
        Record record;
        std::memcpy(&record.timestamp, buffer, sizeof(uint64_t));
        buffer += sizeof(uint64_t);
        std::memcpy(&record.thread, buffer, sizeof(uint64_t));
        buffer += sizeof(uint64_t);
        std::memcpy(&record.level, buffer, sizeof(Level));
        buffer += sizeof(Level);
        std::memcpy(&record.logger.second, buffer, sizeof(uint8_t));
        buffer += sizeof(uint8_t);
        record.logger.first = (char*)buffer;
        buffer += record.logger.second;
        std::memcpy(&record.message.second, buffer, sizeof(uint16_t));
        buffer += sizeof(uint16_t);
        record.message.first = (char*)buffer;
        buffer += record.message.second;
        std::memcpy(&record.buffer.second, buffer, sizeof(uint32_t));
        buffer += sizeof(uint32_t);
        record.buffer.first = buffer;
        buffer += record.buffer.second;
        std::memcpy(&record.raw.second, buffer, sizeof(uint32_t));
        buffer += sizeof(uint32_t);
        record.raw.first = buffer;
        buffer += record.raw.second;

        // Process logging record
        Processor::ProcessRecord(record);
    }

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
