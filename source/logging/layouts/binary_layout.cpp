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

std::pair<void*, size_t> BinaryLayout::LayoutRecord(Record& record)
{
    // Thread local buffer
    thread_local std::vector<uint8_t> local(1024);

    // Calculate the logging record size
    uint32_t size = sizeof(uint64_t) + sizeof(uint64_t) + sizeof(Level) + sizeof(uint8_t) + record.logger.second + sizeof(uint16_t) + record.message.second + sizeof(uint32_t) + record.buffer.second;

    // Resize the thread local buffer to the required size
    local.resize(sizeof(uint32_t) + size);

    // Get the thread local buffer start position
    uint8_t* buffer = local.data();

    // Serialize logging record
    std::memcpy(buffer, &size, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
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

    // Update raw field of the logging record and return
    record.raw = std::make_pair(local.data(), (uint32_t)local.size());
    return record.raw;
}

} // namespace CppLogging
