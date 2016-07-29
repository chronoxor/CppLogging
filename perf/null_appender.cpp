//
// Created by Ivan Shynkarenka on 29.07.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

#include <cstring>

using namespace CppLogging;

const int iterations = 10000000;

class ConfigFixture
{
protected:
    ConfigFixture()
    {
        auto binary_sink = std::make_shared<CppLogging::Processor>();
        binary_sink->layouts().push_back(std::make_shared<CppLogging::BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<CppLogging::NullAppender>());
        CppLogging::Config::ConfigLogger("binary", binary_sink);

        auto text_sink = std::make_shared<CppLogging::Processor>();
        text_sink->layouts().push_back(std::make_shared<CppLogging::TextLayout>());
        text_sink->appenders().push_back(std::make_shared<CppLogging::NullAppender>());
        CppLogging::Config::ConfigLogger("text", text_sink);
    }
};

BENCHMARK_FIXTURE(ConfigFixture, "NullAppender-binary", iterations)
{
    static Logger logger = CppLogging::Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_FIXTURE(ConfigFixture, "NullAppender-text", iterations)
{
    static Logger logger = CppLogging::Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
