//
// Created by Ivan Shynkarenka on 29.07.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

#include <cstring>

using namespace CppLogging;

const int iterations = 10000000;

Logger CreateLogger(bool binary)
{
    auto sink = std::make_shared<CppLogging::Processor>();
    if (binary)
        sink->layouts().push_back(std::make_shared<CppLogging::BinaryLayout>());
    else
        sink->layouts().push_back(std::make_shared<CppLogging::TextLayout>());
    sink->appenders().push_back(std::make_shared<CppLogging::NullAppender>());
    CppLogging::Config::ConfigLogger("test", sink);
    return CppLogging::Config::CreateLogger("test");
}

BENCHMARK("NullAppender-binary", iterations)
{
    static Logger logger = CreateLogger(true);
    logger.Info("Test message");
}

BENCHMARK("NullAppender-text", iterations)
{
    static Logger logger = CreateLogger(false);
    logger.Info("Test message");
}

BENCHMARK_MAIN()
