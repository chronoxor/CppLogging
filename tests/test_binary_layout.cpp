//
// Created by Ivan Shynkarenka on 09.07.2016.
//

#include "catch.hpp"

#include "logging/layouts/binary_layout.h"

using namespace CppLogging;

Record ParseBinaryLayout(const std::pair<void*, size_t>& layout)
{
    Record record;

    // Get the buffer start position
    uint8_t* buffer = (uint8_t*)layout.first;

    // Deserialize logging record
    uint32_t size;
    std::memcpy(&size, buffer, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
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

    return record;
}

bool operator==(const Record& record1, const Record& record2)
{
    if (record1.timestamp != record2.timestamp)
        return false;
    if (record1.thread != record2.thread)
        return false;
    if (record1.level != record2.level)
        return false;
/*
    if (record1.logger.second != record2.logger.second)
        return false;
    if (std::strncmp(record1.logger.first, record2.logger.first, record1.logger.second) != 0)
        return false;
    if (record1.message.second != record2.message.second)
        return false;
    if (std::strncmp(record1.message.first, record2.message.first, record1.message.second) != 0)
        return false;
    if (record1.buffer.second != record2.buffer.second)
        return false;
    if (std::memcmp(record1.buffer.first, record2.buffer.first, record1.buffer.second) != 0)
        return false;
*/
    return true;
}

TEST_CASE("Binary layout", "[CppLogging]")
{
    char logger[] = "Test logger";
    char message[] = "Test message";
    uint8_t buffer[1024];

    Record record;
    record.logger = std::make_pair(logger, (uint8_t)std::strlen(logger));
    record.message = std::make_pair(message, (uint16_t)std::strlen(message));
    record.buffer = std::make_pair(buffer, (uint32_t)sizeof(buffer));

    BinaryLayout layout;
    auto result = layout.LayoutRecord(record);
    REQUIRE(result.first != nullptr);
    REQUIRE(result.second > 0);

    Record clone = ParseBinaryLayout(result);
    REQUIRE(clone == record);
}
