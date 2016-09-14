//
// Created by Ivan Shynkarenka on 13.09.2016.
//

#include "catch.hpp"

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

    REQUIRE(File("test.log").IsFileExists());
    REQUIRE(File("test.log").size() == 10);
    REQUIRE(File("test.1.log").IsFileExists());
    REQUIRE(File("test.1.log").size() == 10);
    REQUIRE(File("test.2.log").IsFileExists());
    REQUIRE(File("test.2.log").size() == 10);
    REQUIRE(File("test.3.log").IsFileExists());
    REQUIRE(File("test.3.log").size() == 10);
    REQUIRE(!File("test.4.log").IsFileExists());

    File::RemoveIf(".", ".*log");
}


TEST_CASE("Rolling file appender with time-based policy", "[CppLogging]")
{
    {
        RollingFileAppender appender(".", TimeRollingPolicy::SECOND, "{UtcDateTime}.log", true);

        Record record;
        record.raw.resize(11);

        for (int i = 0; i < 3; ++i)
        {
			record.timestamp = CppCommon::Timestamp::nano();
            appender.AppendRecord(record);
            appender.Flush();

			// Sleep for one second
			CppCommon::Thread::Sleep(1000);
        }
    }

    REQUIRE(Directory(".").GetFiles(".*log").size() == 3);

	File::RemoveIf(".", ".*log");
}