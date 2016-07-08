/*!
    \file binary_layout.cpp
    \brief Logging binary layout implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/layouts/binary_layout.h"

#include <cstring>

namespace CppLogging {

std::pair<void*, size_t> BinaryLayout::LayoutRecord(const Record& record)
{
    // Resize the buffer to required size
    _buffer.resize(record.size());

    // Get the buffer start position
    uint8_t* buffer = _buffer.data();

    // Serialize logging record
    std::memcpy(buffer, &record.timestamp(), sizeof(uint64_t));
    buffer += sizeof(uint64_t);
    std::memcpy(buffer, &record.thread(), sizeof(uint64_t));
    buffer += sizeof(uint64_t);
    std::memcpy(buffer, &record.level(), sizeof(Level));
    buffer += sizeof(Level);
    std::memcpy(buffer, record.logger().first, record.logger().second);
    buffer += record.logger().second;
    std::memcpy(buffer, record.message().first, record.message().second);
    buffer += record.message().second;
    std::memcpy(buffer, record.buffer().first, record.buffer().second);
    buffer += record.buffer().second;

    // Return the serialized buffer
    return std::make_pair(_buffer.data(), _buffer.size());
}

} // namespace CppLogging
