//
// Created by Ivan Shynkarenka on 29.07.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

const uint64_t iterations = 10000000;

class BinaryConfigPreset
{
protected:
    BinaryConfigPreset()
    {
        auto binary_sink = std::make_shared<Processor>();
        binary_sink->layouts().push_back(std::make_shared<BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("binary", binary_sink);
    }
};

class TextConfigPreset
{
protected:
    TextConfigPreset()
    {
        auto text_sink = std::make_shared<Processor>();
        text_sink->layouts().push_back(std::make_shared<TextLayout>());
        text_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("text", text_sink);
    }
};

BENCHMARK_PRESET(BinaryConfigPreset, "NullAppender-binary", iterations)
{
    static Logger logger = Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_PRESET(TextConfigPreset, "NullAppender-text", iterations)
{
    static Logger logger = Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
