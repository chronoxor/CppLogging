//
// Created by Ivan Shynkarenka on 30.07.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

const auto settings = CppBenchmark::Settings().ThreadsRange(1, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

class NullConfigFixture
{
protected:
    NullConfigFixture()
    {
        auto null_sink = std::make_shared<SyncProcessor>(std::make_shared<NullLayout>());
        null_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("null", null_sink);
    }
};

class BinaryConfigFixture
{
protected:
    BinaryConfigFixture()
    {
        auto binary_sink = std::make_shared<SyncProcessor>(std::make_shared<BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("binary", binary_sink);
    }
};

class TextConfigFixture
{
protected:
    TextConfigFixture()
    {
        auto text_sink = std::make_shared<SyncProcessor>(std::make_shared<TextLayout>());
        text_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("text", text_sink);
    }
};

BENCHMARK_THREADS_FIXTURE(NullConfigFixture, "SyncProcessor-null", settings)
{
    thread_local Logger logger = Config::CreateLogger("null");
    logger.Info("Test message");
}

BENCHMARK_THREADS_FIXTURE(BinaryConfigFixture, "SyncProcessor-binary", settings)
{
    thread_local Logger logger = Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_THREADS_FIXTURE(TextConfigFixture, "SyncProcessor-text", settings)
{
    thread_local Logger logger = Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
