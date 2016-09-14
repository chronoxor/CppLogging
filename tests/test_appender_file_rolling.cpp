//
// Created by Ivan Shynkarenka on 13.09.2016.
//

#include "catch.hpp"

#include "logging/appenders/rolling_file_appender.h"

using namespace CppCommon;
using namespace CppLogging;

TEST_CASE("Rolling file appender with size policy", "[CppLogging]")
{
    {
        RollingFileAppender appender(".", "test", "log", 10, 3);

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

    File::Remove("test.log");
    File::Remove("test.1.log");
    File::Remove("test.2.log");
    File::Remove("test.3.log");
}
