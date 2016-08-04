//
// Created by Ivan Shynkarenka on 29.07.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

const int iterations = 1000000;

class ConsoleConfigFixture
{
protected:
    ConsoleConfigFixture()
    {
        auto sink = std::make_shared<CppLogging::Processor>();
        sink->layouts().push_back(std::make_shared<CppLogging::TextLayout>());
        sink->appenders().push_back(std::make_shared<CppLogging::ConsoleAppender>());
        CppLogging::Config::ConfigLogger("test", sink);
    }
};

BENCHMARK_FIXTURE(ConsoleConfigFixture, "ConsoleAppender", iterations)
{
    static Logger logger = CppLogging::Config::CreateLogger("test");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
