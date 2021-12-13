//
// Created by Ivan Shynkarenka on 01.08.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

const auto settings = CppBenchmark::Settings().ThreadsRange(1, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

class LogConfigFixture
{
protected:
    LogConfigFixture()
    {
        auto async_wait_null_sink = std::make_shared<AsyncWaitProcessor>(std::make_shared<NullLayout>());
        async_wait_null_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("async-wait-null", async_wait_null_sink);

        auto async_wait_free_null_sink = std::make_shared<AsyncWaitFreeProcessor>(std::make_shared<NullLayout>());
        async_wait_free_null_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("async-wait-free-null", async_wait_free_null_sink);

        auto async_wait_binary_sink = std::make_shared<AsyncWaitProcessor>(std::make_shared<BinaryLayout>());
        async_wait_binary_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("async-wait-binary", async_wait_binary_sink);

        auto async_wait_free_binary_sink = std::make_shared<AsyncWaitFreeProcessor>(std::make_shared<BinaryLayout>());
        async_wait_free_binary_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("async-wait-free-binary", async_wait_free_binary_sink);

        auto async_wait_text_sink = std::make_shared<AsyncWaitProcessor>(std::make_shared<TextLayout>());
        async_wait_text_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("async-wait-text", async_wait_text_sink);

        auto async_wait_free_text_sink = std::make_shared<AsyncWaitFreeProcessor>(std::make_shared<TextLayout>());
        async_wait_free_text_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("async-wait-free-text", async_wait_free_text_sink);

        Config::Startup();
    }
};

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "AsyncWaitProcessor-null", settings)
{
    thread_local Logger logger = Config::CreateLogger("async-wait-null");
    logger.Info("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "AsyncWaitFreeProcessor-null", settings)
{
    thread_local Logger logger = Config::CreateLogger("async-wait-free-null");
    logger.Info("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "AsyncWaitProcessor-binary", settings)
{
    thread_local Logger logger = Config::CreateLogger("async-wait-binary");
    logger.Info("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "AsyncWaitFreeProcessor-binary", settings)
{
    thread_local Logger logger = Config::CreateLogger("async-wait-free-binary");
    logger.Info("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "AsyncWaitProcessor-text", settings)
{
    thread_local Logger logger = Config::CreateLogger("async-wait-text");
    logger.Info("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "AsyncWaitFreeProcessor-text", settings)
{
    thread_local Logger logger = Config::CreateLogger("async-wait-free-text");
    logger.Info("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_MAIN()
