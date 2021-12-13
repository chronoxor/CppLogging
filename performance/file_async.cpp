//
// Created by Ivan Shynkarenka on 09.09.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppCommon;
using namespace CppLogging;

const auto settings = CppBenchmark::Settings().ThreadsRange(1, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

class LogConfigFixture
{
protected:
    LogConfigFixture()
    {
        auto async_wait_binary_sink = std::make_shared<AsyncWaitProcessor>(std::make_shared<BinaryLayout>());
        async_wait_binary_sink->appenders().push_back(std::make_shared<FileAppender>(_binary_file_1));
        Config::ConfigLogger("async-wait-binary", async_wait_binary_sink);

        auto async_wait_free_binary_sink = std::make_shared<AsyncWaitFreeProcessor>(std::make_shared<BinaryLayout>());
        async_wait_free_binary_sink->appenders().push_back(std::make_shared<FileAppender>(_binary_file_2));
        Config::ConfigLogger("async-wait-free-binary", async_wait_free_binary_sink);

        auto async_wait_text_sink = std::make_shared<AsyncWaitProcessor>(std::make_shared<TextLayout>());
        async_wait_text_sink->appenders().push_back(std::make_shared<FileAppender>(_text_file_1));
        Config::ConfigLogger("async-wait-text", async_wait_text_sink);

        auto async_wait_free_text_sink = std::make_shared<AsyncWaitProcessor>(std::make_shared<TextLayout>());
        async_wait_free_text_sink->appenders().push_back(std::make_shared<FileAppender>(_text_file_2));
        Config::ConfigLogger("async-wait-free-text", async_wait_free_text_sink);

        Config::Startup();
    }

    ~LogConfigFixture()
    {
        Config::Shutdown();
        if (_binary_file_1.IsFileExists())
            File::Remove(_binary_file_1);
        if (_binary_file_2.IsFileExists())
            File::Remove(_binary_file_2);
        if (_text_file_1.IsFileExists())
            File::Remove(_text_file_1);
        if (_text_file_2.IsFileExists())
            File::Remove(_text_file_2);
    }

private:
    File _binary_file_1{"test1.bin.log"};
    File _binary_file_2{"test2.bin.log"};
    File _text_file_1{"test1.log"};
    File _text_file_2{"test2.log"};
};

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "FileAsyncWait-binary", settings)
{
    thread_local Logger logger = Config::CreateLogger("async-wait-binary");
    logger.Info("Test message {}-{}-{}", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "FileAsyncWaitFree-binary", settings)
{
    thread_local Logger logger = Config::CreateLogger("async-wait-free-binary");
    logger.Info("Test message {}-{}-{}", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "FileAsyncWait-text", settings)
{
    thread_local Logger logger = Config::CreateLogger("async-wait-text");
    logger.Info("Test message {}-{}-{}", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "FileAsyncWaitFree-text", settings)
{
    thread_local Logger logger = Config::CreateLogger("async-wait-free-text");
    logger.Info("Test message {}-{}-{}", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_MAIN()
