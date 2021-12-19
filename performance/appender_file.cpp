//
// Created by Ivan Shynkarenka on 08.09.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

using namespace CppCommon;
using namespace CppLogging;

class BinaryConfigFixture : public virtual CppBenchmark::Fixture
{
protected:
    void Initialize(CppBenchmark::Context& context) override
    {
        auto binary_sink = std::make_shared<Processor>(std::make_shared<BinaryLayout>());
        binary_sink->appenders().push_back(std::make_shared<FileAppender>(_file));
        Config::ConfigLogger("binary", binary_sink);
        Config::Startup();
    }

    void Cleanup(CppBenchmark::Context& context) override
    {
        Config::Shutdown();
        if (_file.IsFileExists())
            File::Remove(_file);
    }

private:
    File _file{"test.bin.log"};
};

class HashConfigFixture : public virtual CppBenchmark::Fixture
{
protected:
    void Initialize(CppBenchmark::Context& context) override
    {
        auto hash_sink = std::make_shared<Processor>(std::make_shared<HashLayout>());
        hash_sink->appenders().push_back(std::make_shared<FileAppender>(_file));
        Config::ConfigLogger("hash", hash_sink);
        Config::Startup();
    }

    void Cleanup(CppBenchmark::Context& context) override
    {
        Config::Shutdown();
        if (_file.IsFileExists())
            File::Remove(_file);
    }

private:
    File _file{"test.hash.log"};
};

class TextConfigFixture : public virtual CppBenchmark::Fixture
{
protected:
    void Initialize(CppBenchmark::Context& context) override
    {
        auto text_sink = std::make_shared<Processor>(std::make_shared<TextLayout>());
        text_sink->appenders().push_back(std::make_shared<FileAppender>(_file));
        Config::ConfigLogger("text", text_sink);
        Config::Startup();
    }

    void Cleanup(CppBenchmark::Context& context) override
    {
        Config::Shutdown();
        if (_file.IsFileExists())
            File::Remove(_file);
    }

private:
    File _file{"test.log"};
};

BENCHMARK_FIXTURE(BinaryConfigFixture, "FileAppender-binary")
{
    static Logger logger = Config::CreateLogger("binary");
    logger.Info("Test message");
}

BENCHMARK_FIXTURE(HashConfigFixture, "FileAppender-hash")
{
    static Logger logger = Config::CreateLogger("hash");
    logger.Info("Test message");
}

BENCHMARK_FIXTURE(TextConfigFixture, "FileAppender-text")
{
    static Logger logger = Config::CreateLogger("text");
    logger.Info("Test message");
}

BENCHMARK_MAIN()
