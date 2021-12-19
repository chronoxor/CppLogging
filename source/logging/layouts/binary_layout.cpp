/*!
    \file binary_layout.cpp
    \brief Binary layout implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/layouts/binary_layout.h"

#include <cstring>
#include <vector>

namespace CppLogging {

void BinaryLayout::LayoutRecord(Record& record)
{
    // Calculate logging record size
    uint32_t size = (uint32_t)(sizeof(uint64_t) + sizeof(uint64_t) + sizeof(Level) + sizeof(uint8_t) + record.logger.size() + sizeof(uint16_t) + record.message.size() + sizeof(uint32_t) + record.buffer.size());

    // Resize the raw buffer to the required size
    record.raw.resize(sizeof(uint32_t) + size + 1);

    // Get the raw buffer start position
    uint8_t* buffer = record.raw.data();

    // Serialize the logging record
    std::memcpy(buffer, &size, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    std::memcpy(buffer, &record.timestamp, sizeof(uint64_t));
    buffer += sizeof(uint64_t);
    std::memcpy(buffer, &record.thread, sizeof(uint64_t));
    buffer += sizeof(uint64_t);
    std::memcpy(buffer, &record.level, sizeof(Level));
    buffer += sizeof(Level);

    // Serialize the logger name
    uint8_t logger_size = (uint8_t)record.logger.size();
    std::memcpy(buffer, &logger_size, sizeof(uint8_t));
    buffer += sizeof(uint8_t);
    std::memcpy(buffer, record.logger.data(), record.logger.size());
    buffer += record.logger.size();

    // Serialize the logging message
    uint16_t message_size = (uint16_t)record.message.size();
    std::memcpy(buffer, &message_size, sizeof(uint16_t));
    buffer += sizeof(uint16_t);
    std::memcpy(buffer, record.message.data(), record.message.size());
    buffer += record.message.size();

    // Serialize the logging buffer
    uint32_t buffer_size = (uint32_t)record.buffer.size();
    std::memcpy(buffer, &buffer_size, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    std::memcpy(buffer, record.buffer.data(), record.buffer.size());
    buffer += record.buffer.size();

    // Write the last zero byte
    *buffer = 0;
}

} // namespace CppLogging
