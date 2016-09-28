//
// Created by Ivan Shynkarenka on 29.07.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

const uint64_t iterations = 1000000;

class ConsoleConfigFixture
{
protected:
    ConsoleConfigFixture()
    {
        auto sink = std::make_shared<Processor>();
        sink->layouts().push_back(std::make_shared<TextLayout>());
        sink->appenders().push_back(std::make_shared<ConsoleAppender>());
        Config::ConfigLogger("test", sink);
    }
};

BENCHMARK_FIXTURE(ConsoleConfigFixture, "ConsoleAppender", iterations)
{
    static Logger logger = Config::CreateLogger("test");
    logger.Info("Test message {}", context.metrics().total_iterations());
}

BENCHMARK_MAIN()
