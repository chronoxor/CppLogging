//
// Created by Ivan Shynkarenka on 09.07.2016
//

#include "test.h"

#include "logging/layouts/binary_layout.h"

#include <cstring>

using namespace CppLogging;

namespace {

Record ParseBinaryLayout(const std::vector<uint8_t>& raw)
{
    Record record;

    // Get the buffer start position
    const uint8_t* buffer = raw.data();

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

    uint8_t logger_size;
    std::memcpy(&logger_size, buffer, sizeof(uint8_t));
    buffer += sizeof(uint8_t);
    record.logger.insert(record.logger.begin(), buffer, buffer + logger_size);
    buffer += logger_size;

    uint16_t message_size;
    std::memcpy(&message_size, buffer, sizeof(uint16_t));
    buffer += sizeof(uint16_t);
    record.message.insert(record.message.begin(), buffer, buffer + message_size);
    buffer += message_size;

    uint32_t buffer_size;
    std::memcpy(&buffer_size, buffer, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    record.buffer.insert(record.buffer.begin(), buffer, buffer + buffer_size);
    buffer += buffer_size;

    // Skip the last zero byte
    ++buffer;

    return record;
}

bool CompareRecords(const Record& record1, const Record& record2)
{
    if (record1.timestamp != record2.timestamp)
        return false;
    if (record1.thread != record2.thread)
        return false;
    if (record1.level != record2.level)
        return false;
    if (record1.logger != record2.logger)
        return false;
    if (record1.message != record2.message)
        return false;
    if (record1.buffer != record2.buffer)
        return false;
    return true;
}

} // namespace

TEST_CASE("Binary layout", "[CppLogging]")
{
    Record record;
    record.logger = "Test logger";
    record.message = "Test message";
    record.buffer.resize(1024, 123);

    BinaryLayout layout;
    layout.LayoutRecord(record);
    REQUIRE(record.raw.size() > 0);

    Record clone = ParseBinaryLayout(record.raw);
    REQUIRE(CompareRecords(clone, record));
}
