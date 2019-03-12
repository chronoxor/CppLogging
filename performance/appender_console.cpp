//
// Created by Ivan Shynkarenka on 29.07.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

class ConsoleConfigFixture
{
protected:
    ConsoleConfigFixture()
    {
        auto sink = std::make_shared<Processor>(std::make_shared<TextLayout>());
        sink->appenders().push_back(std::make_shared<ConsoleAppender>());
        Config::ConfigLogger("test", sink);
    }
};

BENCHMARK_FIXTURE(ConsoleConfigFixture, "ConsoleAppender")
{
    static Logger logger = Config::CreateLogger("test");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
