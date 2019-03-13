//
// Created by Ivan Shynkarenka on 09.09.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppCommon;
using namespace CppLogging;

const auto settings = CppBenchmark::Settings().ThreadsRange(1, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

class BinaryWaitConfigFixture
{
protected:
    BinaryWaitConfigFixture()
    {
        auto binary_sink = std::make_shared<AsyncWaitProcessor>(std::make_shared<BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<FileAppender>(File("test.bin.log")));
        Config::ConfigLogger("binary", binary_sink);
    }

    ~BinaryWaitConfigFixture()
    {
        File::Remove("test.bin.log");
    }
};

class BinaryWaitFreeConfigFixture
{
protected:
    BinaryWaitFreeConfigFixture()
    {
        auto binary_sink = std::make_shared<AsyncWaitFreeProcessor>(std::make_shared<BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<FileAppender>(File("test.bin.log")));
        Config::ConfigLogger("binary", binary_sink);
    }

    ~BinaryWaitFreeConfigFixture()
    {
        File::Remove("test.bin.log");
    }
};

class TextWaitConfigFixture
{
protected:
    TextWaitConfigFixture()
    {
        auto text_sink = std::make_shared<AsyncWaitProcessor>(std::make_shared<TextLayout>());
        text_sink->appenders().push_back(std::make_shared<FileAppender>(File("test.log")));
        Config::ConfigLogger("text", text_sink);
    }

    ~TextWaitConfigFixture()
    {
        File::Remove("test.log");
    }
};

class TextWaitFreeConfigFixture
{
protected:
    TextWaitFreeConfigFixture()
    {
        auto text_sink = std::make_shared<AsyncWaitFreeProcessor>(std::make_shared<TextLayout>());
        text_sink->appenders().push_back(std::make_shared<FileAppender>(File("test.log")));
        Config::ConfigLogger("text", text_sink);
    }

    ~TextWaitFreeConfigFixture()
    {
        File::Remove("test.log");
    }
};

BENCHMARK_THREADS_FIXTURE(BinaryWaitConfigFixture, "FileAsyncWait-binary", settings)
{
    thread_local Logger logger = Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_THREADS_FIXTURE(BinaryWaitFreeConfigFixture, "FileAsyncWaitFree-binary", settings)
{
    thread_local Logger logger = Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_THREADS_FIXTURE(TextWaitConfigFixture, "FileAsyncWait-text", settings)
{
    thread_local Logger logger = Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_THREADS_FIXTURE(TextWaitFreeConfigFixture, "FileAsyncWaitFree-text", settings)
{
    thread_local Logger logger = Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
