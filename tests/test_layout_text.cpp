//
// Created by Ivan Shynkarenka on 22.07.2016
//

#include "test.h"

#include "logging/layouts/text_layout.h"
#include "system/environment.h"

using namespace CppCommon;
using namespace CppLogging;

TEST_CASE("Text layout", "[CppLogging]")
{
    Record record;
    record.timestamp = 1468408953123456789ll;
    record.thread = 0x98ABCDEF;
    record.level = Level::WARN;
    record.logger = "Test logger";
    record.message = "Test message";

    TextLayout layout1;
    layout1.LayoutRecord(record);
    REQUIRE(record.raw.size() > 0);

    std::string utc_sample = "2016-07-13T11:22:33.123Z - 456.789 - [0x98ABCDEF] - WARN  - Test logger - Test message - " + Environment::EndLine();

    TextLayout layout2("{UtcYear}-{UtcMonth}-{UtcDay}T{UtcHour}:{UtcMinute}:{UtcSecond}.{Millisecond}{UtcTimezone} - {Microsecond}.{Nanosecond} - [{Thread}] - {Level} - {Logger} - {Message} - {EndLine}");
    layout2.LayoutRecord(record);
    REQUIRE(std::string(record.raw.begin(), record.raw.end() - 1) == utc_sample);

    TextLayout layout3("{UtcDateTime} - {Microsecond}.{Nanosecond} - [{Thread}] - {Level} - {Logger} - {Message} - {EndLine}");
    layout3.LayoutRecord(record);
    REQUIRE(std::string(record.raw.begin(), record.raw.end() - 1) == utc_sample);
}
