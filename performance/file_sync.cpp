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
        auto sync_binary_sink = std::make_shared<SyncProcessor>(std::make_shared<BinaryLayout>());
        sync_binary_sink->appenders().push_back(std::make_shared<FileAppender>(_binary_file));
        Config::ConfigLogger("sync-binary", sync_binary_sink);

        auto sync_text_sink = std::make_shared<SyncProcessor>(std::make_shared<TextLayout>());
        sync_text_sink->appenders().push_back(std::make_shared<FileAppender>(_text_file));
        Config::ConfigLogger("sync-text", sync_text_sink);

        Config::Startup();
    }

    ~LogConfigFixture()
    {
        Config::Shutdown();
        if (_binary_file.IsFileExists())
            File::Remove(_binary_file);
        if (_text_file.IsFileExists())
            File::Remove(_text_file);
    }

private:
    File _binary_file{"test.bin.log"};
    File _text_file{"test.log"};
};

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "FileSync-binary", settings)
{
    thread_local Logger logger = Config::CreateLogger("sync-binary");
    logger.Info("Test message {}-{}-{}", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_THREADS_FIXTURE(LogConfigFixture, "FileSync-text", settings)
{
    thread_local Logger logger = Config::CreateLogger("sync-text");
    logger.Info("Test message {}-{}-{}", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_MAIN()
