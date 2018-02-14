//
// Created by Ivan Shynkarenka on 08.09.2016
//

#include "test.h"

#include "logging/appenders/file_appender.h"

using namespace CppCommon;
using namespace CppLogging;

TEST_CASE("File appender", "[CppLogging]")
{
    File file("test.log");
    {
        FileAppender appender(file, true, true);

        Record record;
        record.raw.resize(11);

        appender.AppendRecord(record);
        appender.Flush();
    }
    REQUIRE(file.IsFileExists());
    REQUIRE(file.size() == 10);
    File::Remove(file);
}
