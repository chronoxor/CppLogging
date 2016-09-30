//
// Created by Ivan Shynkarenka on 08.09.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

const uint64_t iterations = 1000000;

class BinaryConfigFixture
{
protected:
    BinaryConfigFixture()
    {
        auto binary_sink = std::make_shared<Processor>();
        binary_sink->layouts().push_back(std::make_shared<BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<FileAppender>(CppCommon::File("test.bin.log")));
        Config::ConfigLogger("binary", binary_sink);
    }

    ~BinaryConfigFixture()
    {
        CppCommon::File::Remove("test.bin.log");
    }
};

class TextConfigFixture
{
protected:
    TextConfigFixture()
    {
        auto text_sink = std::make_shared<Processor>();
        text_sink->layouts().push_back(std::make_shared<TextLayout>());
        text_sink->appenders().push_back(std::make_shared<FileAppender>(CppCommon::File("test.log")));
        Config::ConfigLogger("text", text_sink);
    }

    ~TextConfigFixture()
    {
        CppCommon::File::Remove("test.log");
    }
};

BENCHMARK_FIXTURE(BinaryConfigFixture, "FileAppender-binary", iterations)
{
    static Logger logger = Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_FIXTURE(TextConfigFixture, "FileAppender-text", iterations)
{
    static Logger logger = Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
