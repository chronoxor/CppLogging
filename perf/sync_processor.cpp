//
// Created by Ivan Shynkarenka on 30.07.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

const int iterations = 1000000;
const auto settings = CppBenchmark::Settings().Iterations(iterations).ThreadsRange(1, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

class ConfigFixture
{
protected:
    ConfigFixture()
    {
        auto binary_sink = std::make_shared<CppLogging::SyncProcessor>();
        binary_sink->layouts().push_back(std::make_shared<CppLogging::BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<CppLogging::NullAppender>());
        CppLogging::Config::ConfigLogger("binary", binary_sink);

        auto text_sink = std::make_shared<CppLogging::SyncProcessor>();
        text_sink->layouts().push_back(std::make_shared<CppLogging::TextLayout>());
        text_sink->appenders().push_back(std::make_shared<CppLogging::NullAppender>());
        CppLogging::Config::ConfigLogger("text", text_sink);
    }
};

BENCHMARK_THREADS_FIXTURE(ConfigFixture, "SyncProcessor-binary", settings)
{
    static Logger logger = CppLogging::Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_THREADS_FIXTURE(ConfigFixture, "SyncProcessor-text", settings)
{
    static Logger logger = CppLogging::Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
