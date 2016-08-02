//
// Created by Ivan Shynkarenka on 01.08.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

const int iterations = 1000000;
const auto settings = CppBenchmark::Settings().Iterations(iterations).ThreadsRange(1, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

class BinaryConfigFixture
{
protected:
    BinaryConfigFixture()
    {
        auto binary_sink = std::make_shared<CppLogging::AsyncProcessor>();
        binary_sink->layouts().push_back(std::make_shared<CppLogging::BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<CppLogging::NullAppender>());
        CppLogging::Config::ConfigLogger("binary", binary_sink);
    }
};

class TextConfigFixture
{
protected:
    TextConfigFixture()
    {
        auto text_sink = std::make_shared<CppLogging::AsyncProcessor>();
        text_sink->layouts().push_back(std::make_shared<CppLogging::TextLayout>());
        text_sink->appenders().push_back(std::make_shared<CppLogging::NullAppender>());
        CppLogging::Config::ConfigLogger("text", text_sink);
    }
};

BENCHMARK_THREADS_FIXTURE(BinaryConfigFixture, "AsyncProcessor-binary", settings)
{
    static Logger logger = CppLogging::Config::CreateLogger("binary");
    static std::string message = "Test message";
    logger.Info(message);
}

BENCHMARK_THREADS_FIXTURE(TextConfigFixture, "AsyncProcessor-text", settings)
{
    static Logger logger = CppLogging::Config::CreateLogger("text");
    static std::string message = "Test message";
    logger.Info(message);
}

BENCHMARK_MAIN()
