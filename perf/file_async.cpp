//
// Created by Ivan Shynkarenka on 09.09.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

const uint64_t iterations = 1000000;
const auto settings = CppBenchmark::Settings().Iterations(iterations).ThreadsRange(1, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

class BinaryConfigFixture : public virtual CppBenchmark::FixtureThreads
{
protected:
    BinaryConfigFixture()
    {
        auto file_sink = std::make_shared<AsyncProcessor>();
        file_sink->appenders().push_back(std::make_shared<FileAppender>(CppCommon::File("test.bin.log")));
        auto binary_sink = std::make_shared<Processor>();
        binary_sink->layouts().push_back(std::make_shared<BinaryLayout>());
        binary_sink->processors().push_back(file_sink);
        Config::ConfigLogger("binary", binary_sink);
    }

    ~BinaryConfigFixture()
    {
        CppCommon::File::Remove("test.bin.log");
    }

    void Cleanup(CppBenchmark::ContextThreads& context) override
    {
        // Update benchmark metrics
        context.metrics().AddIterations(context.threads() * iterations - 1);
    }
};

class TextConfigFixture : public virtual CppBenchmark::FixtureThreads
{
protected:
    TextConfigFixture()
    {
        auto file_sink = std::make_shared<AsyncProcessor>();
        file_sink->appenders().push_back(std::make_shared<FileAppender>(CppCommon::File("test.log")));
        auto text_sink = std::make_shared<Processor>();
        text_sink->layouts().push_back(std::make_shared<TextLayout>());
        text_sink->processors().push_back(file_sink);
        Config::ConfigLogger("text", text_sink);
    }

    ~TextConfigFixture()
    {
        CppCommon::File::Remove("test.log");
    }

    void Cleanup(CppBenchmark::ContextThreads& context) override
    {
        // Update benchmark metrics
        context.metrics().AddIterations(context.threads() * iterations - 1);
    }
};

BENCHMARK_THREADS_FIXTURE(BinaryConfigFixture, "FileAsync-binary", settings)
{
    thread_local Logger logger = Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_THREADS_FIXTURE(TextConfigFixture, "FileAsync-text", settings)
{
    thread_local Logger logger = Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
