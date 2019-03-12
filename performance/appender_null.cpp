//
// Created by Ivan Shynkarenka on 29.07.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

class BinaryConfigFixture
{
protected:
    BinaryConfigFixture()
    {
        auto binary_sink = std::make_shared<Processor>(std::make_shared<BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("binary", binary_sink);
    }
};

class TextConfigFixture
{
protected:
    TextConfigFixture()
    {
        auto text_sink = std::make_shared<Processor>(std::make_shared<TextLayout>());
        text_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("text", text_sink);
    }
};

BENCHMARK_FIXTURE(BinaryConfigFixture, "NullAppender-binary")
{
    static Logger logger = Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_FIXTURE(TextConfigFixture, "NullAppender-text")
{
    static Logger logger = Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
