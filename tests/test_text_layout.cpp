//
// Created by Ivan Shynkarenka on 22.07.2016.
//

#include "catch.hpp"

#include "logging/layouts/text_layout.h"

#include <cstring>

using namespace CppLogging;

TEST_CASE("Text layout", "[CppLogging]")
{
    char logger[] = "Test logger";
    char message[] = "Test message";
    uint8_t buffer[1024];

    Record record;
    record.timestamp = 1468408953123456789ll;
    record.thread = 0x98ABCDEF;
    record.level = Level::WARN;
    record.logger = std::make_pair(logger, (uint8_t)std::strlen(logger));
    record.message = std::make_pair(message, (uint16_t)std::strlen(message));
    record.buffer = std::make_pair(buffer, (uint32_t)sizeof(buffer));

    TextLayout layout1;
    auto result1 = layout1.LayoutRecord(record);
    REQUIRE(result1.first != nullptr);
    REQUIRE(result1.second > 0);

#if defined(_WIN32) || defined(_WIN64)
    char utc_sample[] = "2016-07-13T11:22:33.123Z - 456.789 - [0x98ABCDEF] - WARN  - Test logger - Test message - \r\n";
#elif defined(unix) || defined(__unix) || defined(__unix__)
    char utc_sample[] = "2016-07-13T11:22:33.123Z - 456.789 - [0x98ABCDEF] - WARN  - Test logger - Test message - \n";
#endif

    TextLayout layout2("{UtcYear}-{UtcMonth}-{UtcDay}T{UtcHour}:{UtcMinute}:{UtcSecond}.{Millisecond}{UtcTimezone} - {Microsecond}.{Nanosecond} - [{Thread}] - {Level} - {Logger} - {Message} - {EndLine}");
    auto result2 = layout2.LayoutRecord(record);
    REQUIRE(result2.second == std::strlen(utc_sample));
    REQUIRE(std::memcmp(result2.first, utc_sample, std::strlen(utc_sample)) == 0);

    TextLayout layout3("{UtcDateTime} - {Microsecond}.{Nanosecond} - [{Thread}] - {Level} - {Logger} - {Message} - {EndLine}");
    auto result3 = layout3.LayoutRecord(record);
    REQUIRE(result3.second == std::strlen(utc_sample));
    REQUIRE(std::memcmp(result3.first, utc_sample, std::strlen(utc_sample)) == 0);
}
