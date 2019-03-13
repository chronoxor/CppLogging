//
// Created by Ivan Shynkarenka on 01.08.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

const auto settings = CppBenchmark::Settings().ThreadsRange(1, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

class NullWaitConfigFixture
{
protected:
    NullWaitConfigFixture()
    {
        auto null_sink = std::make_shared<AsyncWaitProcessor>(std::make_shared<NullLayout>());
        null_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("null", null_sink);
    }
};

class NullWaitFreeConfigFixture
{
protected:
    NullWaitFreeConfigFixture()
    {
        auto null_sink = std::make_shared<AsyncWaitFreeProcessor>(std::make_shared<NullLayout>());
        null_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("null", null_sink);
    }
};

class BinaryWaitConfigFixture
{
protected:
    BinaryWaitConfigFixture()
    {
        auto binary_sink = std::make_shared<AsyncWaitProcessor>(std::make_shared<BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("binary", binary_sink);
    }
};

class BinaryWaitFreeConfigFixture
{
protected:
    BinaryWaitFreeConfigFixture()
    {
        auto binary_sink = std::make_shared<AsyncWaitFreeProcessor>(std::make_shared<BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("binary", binary_sink);
    }
};

class TextWaitConfigFixture
{
protected:
    TextWaitConfigFixture()
    {
        auto text_sink = std::make_shared<AsyncWaitProcessor>(std::make_shared<TextLayout>());
        text_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("text", text_sink);
    }
};

class TextWaitFreeConfigFixture
{
protected:
    TextWaitFreeConfigFixture()
    {
        auto text_sink = std::make_shared<AsyncWaitFreeProcessor>(std::make_shared<TextLayout>());
        text_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("text", text_sink);
    }
};

BENCHMARK_THREADS_FIXTURE(NullWaitConfigFixture, "AsyncWaitProcessor-null", settings)
{
    thread_local Logger logger = Config::CreateLogger("null");
    logger.Info("Test message");
}

BENCHMARK_THREADS_FIXTURE(NullWaitFreeConfigFixture, "AsyncWaitFreeProcessor-null", settings)
{
    thread_local Logger logger = Config::CreateLogger("null");
    logger.Info("Test message");
}

BENCHMARK_THREADS_FIXTURE(BinaryWaitConfigFixture, "AsyncWaitProcessor-binary", settings)
{
    thread_local Logger logger = Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_THREADS_FIXTURE(BinaryWaitFreeConfigFixture, "AsyncWaitFreeProcessor-binary", settings)
{
    thread_local Logger logger = Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_THREADS_FIXTURE(TextWaitConfigFixture, "AsyncWaitProcessor-text", settings)
{
    thread_local Logger logger = Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_THREADS_FIXTURE(TextWaitFreeConfigFixture, "AsyncWaitFreeProcessor-text", settings)
{
    thread_local Logger logger = Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
