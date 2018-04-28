//
// Created by Ivan Shynkarenka on 13.09.2016
//

#include "test.h"

#include "logging/appenders/rolling_file_appender.h"

using namespace CppCommon;
using namespace CppLogging;

TEST_CASE("Rolling file appender with size-based policy", "[CppLogging]")
{
    {
        RollingFileAppender appender(".", "test", "log", 10, 3, true);

        Record record;
        record.raw.resize(11);

        for (int i = 0; i < 10; ++i)
        {
            appender.AppendRecord(record);
            appender.Flush();
        }
    }

    REQUIRE(File("test.1.log.zip").IsFileExists());
    REQUIRE(File("test.1.log.zip").size() > 0);
    REQUIRE(File("test.2.log.zip").IsFileExists());
    REQUIRE(File("test.2.log.zip").size() > 0);
    REQUIRE(File("test.3.log.zip").IsFileExists());
    REQUIRE(File("test.3.log.zip").size() > 0);
    REQUIRE(!File("test.4.log.zip").IsFileExists());

    File::RemoveIf(".", ".*.log.zip");
}

TEST_CASE("Rolling file appender with time-based policy", "[CppLogging]")
{
    {
        RollingFileAppender appender(".", TimeRollingPolicy::SECOND, "{UtcDateTime}.log", true);

        Record record;
        record.raw.resize(11);

        for (int i = 0; i < 3; ++i)
        {
            record.timestamp = Timestamp::utc();
            appender.AppendRecord(record);
            appender.Flush();

            // Sleep for one second
            Thread::Sleep(1000);
        }
    }

    REQUIRE(Directory(".").GetFiles(".*log.zip").size() == 3);

    File::RemoveIf(".", ".*.log.zip");
}
