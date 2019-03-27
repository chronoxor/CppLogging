//
// Created by Ivan Shynkarenka on 28.09.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppCommon;
using namespace CppLogging;

const auto settings = CppBenchmark::Settings().ThreadsRange(1, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

class BinaryConfigFixture
{
protected:
    BinaryConfigFixture()
    {
        auto binary_sink = std::make_shared<AsyncWaitFreeProcessor>(std::make_shared<BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<FileAppender>(_file));
        Config::ConfigLogger("binary", binary_sink);
    }

    ~BinaryConfigFixture()
    {
        if (_file.IsFileExists())
            File::Remove(_file);
    }

private:
    File _file{"test.bin.log"};
};

BENCHMARK_THREADS_FIXTURE(BinaryConfigFixture, "Format(int, double, string)", settings)
{
    thread_local Logger logger = Config::CreateLogger("binary");
    logger.Info("Test {}.{}.{} message"_format(context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name()));
}

BENCHMARK_THREADS_FIXTURE(BinaryConfigFixture, "Serialize(int, double, string)", settings)
{
    thread_local Logger logger = Config::CreateLogger("binary");
    logger.Info("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_MAIN()
