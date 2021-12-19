//
// Created by Ivan Shynkarenka on 29.07.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppLogging;

class BinaryConfigFixture : public virtual CppBenchmark::Fixture
{
protected:
    void Initialize(CppBenchmark::Context& context) override
    {
        auto binary_sink = std::make_shared<Processor>(std::make_shared<BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("binary", binary_sink);
        Config::Startup();
    }
};

class HashConfigFixture : public virtual CppBenchmark::Fixture
{
protected:
    void Initialize(CppBenchmark::Context& context) override
    {
        auto hash_sink = std::make_shared<Processor>(std::make_shared<HashLayout>());
        hash_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("hash", hash_sink);
        Config::Startup();
    }
};

class TextConfigFixture : public virtual CppBenchmark::Fixture
{
protected:
    void Initialize(CppBenchmark::Context& context) override
    {
        auto text_sink = std::make_shared<Processor>(std::make_shared<TextLayout>());
        text_sink->appenders().push_back(std::make_shared<NullAppender>());
        Config::ConfigLogger("text", text_sink);
        Config::Startup();
    }
};

BENCHMARK_FIXTURE(BinaryConfigFixture, "NullAppender-binary")
{
    static Logger logger = Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_FIXTURE(HashConfigFixture, "NullAppender-hash")
{
    static Logger logger = Config::CreateLogger("hash");
    logger.Info("Test message");
}

BENCHMARK_FIXTURE(TextConfigFixture, "NullAppender-text")
{
    static Logger logger = Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
