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

class BinaryLayout::Impl
{
public:
    Impl() : _buffer(1024)
    {
    }

    ~Impl()
    {
    }

    std::pair<void*, size_t> LayoutRecord(const Record& record)
    {
        // Calculate logging record size
        uint32_t size = sizeof(uint64_t) + sizeof(uint64_t) + sizeof(Level) + sizeof(uint8_t) + record.logger.second + sizeof(uint16_t) + record.message.second + sizeof(uint32_t) + record.buffer.second;

        // Resize the buffer to required size
        _buffer.resize(size);

        // Get the buffer start position
        uint8_t* buffer = _buffer.data();

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

        // Return the serialized buffer
        return std::make_pair(_buffer.data(), _buffer.size());
    }

private:
    std::vector<uint8_t> _buffer;
};

BinaryLayout::BinaryLayout() : _pimpl(new Impl())
{
}

BinaryLayout::~BinaryLayout()
{
}

std::pair<void*, size_t> BinaryLayout::LayoutRecord(const Record& record)
{
    return _pimpl->LayoutRecord(record);
}

} // namespace CppLogging
