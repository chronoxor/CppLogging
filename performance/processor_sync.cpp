//
// Created by Ivan Shynkarenka on 30.07.2016
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
        auto sync_null_sink = std::make_shared<SyncProcessor>(std::make_shared<NullLayout>());
        sync_null_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("sync-null", sync_null_sink);

        auto sync_binary_sink = std::make_shared<SyncProcessor>(std::make_shared<BinaryLayout>());
        sync_binary_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("sync-binary", sync_binary_sink);

        auto sync_text_sink = std::make_shared<SyncProcessor>(std::make_shared<TextLayout>());
        sync_text_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("sync-text", sync_text_sink);

        Config::Startup();
    }
};

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "SyncProcessor-null", settings)
{
    thread_local Logger logger = Config::CreateLogger("sync-null");
    logger.Info("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "SyncProcessor-binary", settings)
{
    thread_local Logger logger = Config::CreateLogger("sync-binary");
    logger.Info("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "SyncProcessor-text", settings)
{
    thread_local Logger logger = Config::CreateLogger("sync-text");
    logger.Info("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_MAIN()
