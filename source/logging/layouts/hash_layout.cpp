/*!
    \file hash_layout.cpp
    \brief Hash layout implementation
    \author Ivan Shynkarenka
    \date 13.12.2021
    \copyright MIT License
*/

#include "logging/layouts/hash_layout.h"

#include <cstring>
#include <vector>

namespace CppLogging {

uint32_t HashLayout::Hash(std::string_view message)
{
    const uint32_t FNV_PRIME = 16777619u;
    const uint32_t OFFSET_BASIS = 2166136261u;

    uint32_t hash = OFFSET_BASIS;
    for (size_t i = 0; i < message.size(); ++i)
    {
        hash ^= message[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

void HashLayout::LayoutRecord(Record& record)
{
    // Calculate logging record size
    uint32_t size = (uint32_t)(sizeof(uint64_t) + sizeof(uint64_t) + sizeof(Level) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + record.buffer.size());

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

    // Serialize the logger name hash
    uint32_t logger_hash = Hash(record.logger);
    std::memcpy(buffer, &logger_hash, sizeof(uint32_t));
    buffer += sizeof(uint32_t);

    // Serialize the logging message hash
    uint32_t message_hash = Hash(record.message);
    std::memcpy(buffer, &message_hash, sizeof(uint32_t));
    buffer += sizeof(uint32_t);

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
