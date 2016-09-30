//
// Created by Ivan Shynkarenka on 28.09.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

const uint64_t iterations = 1000000;
const auto settings = CppBenchmark::Settings().Iterations(iterations).ThreadsRange(1, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

class TextConfigFixture
{
protected:
    TextConfigFixture() : _file("test.log")
    {
        auto text_sink = std::make_shared<AsyncProcessor>();
        text_sink->layouts().push_back(std::make_shared<TextLayout>());
        text_sink->appenders().push_back(std::make_shared<FileAppender>(_file));
        Config::ConfigLogger("text", text_sink);
    }

    ~TextConfigFixture()
    {
        if (_file.IsFileExists())
            CppCommon::File::Remove(_file);
    }

private:
    CppCommon::File _file;
};

BENCHMARK_THREADS_FIXTURE(TextConfigFixture, "PreFormat", settings)
{
    thread_local Logger logger = Config::CreateLogger("text");
    logger.Info("Test message {}"_format(context.metrics().total_iterations()).c_str());
}

BENCHMARK_THREADS_FIXTURE(TextConfigFixture, "PostFormat", settings)
{
    thread_local Logger logger = Config::CreateLogger("text");
    logger.Info("Test message {}", context.metrics().total_iterations());
}

BENCHMARK_MAIN()
