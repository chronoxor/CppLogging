//
// Created by Ivan Shynkarenka on 01.08.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

#include <functional>
#include <thread>
#include <vector>

using namespace CppLogging;

const uint64_t items_to_produce = 10000000;
const auto settings = CppBenchmark::Settings().ParamRange(8, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

class BinaryConfigFixture
{
protected:
    BinaryConfigFixture()
    {
        auto async = std::make_shared<CppLogging::AsyncProcessor>();
        async->appenders().push_back(std::make_shared<CppLogging::NullAppender>());

        auto binary_sink = std::make_shared<CppLogging::Processor>();
        binary_sink->layouts().push_back(std::make_shared<CppLogging::TextLayout>());
        binary_sink->processors().push_back(async);

        CppLogging::Config::ConfigLogger("binary", binary_sink);
    }
};

void test(CppBenchmark::Context& context, const std::function<void()>& wait_strategy)
{
    const int producers_count = context.x();

    Logger logger = CppLogging::Config::CreateLogger("binary");

    // Start producer threads
    std::vector<std::thread> producers;
    for (int producer = 0; producer < producers_count; ++producer)
    {
        producers.push_back(std::thread([&logger, &wait_strategy, producer, producers_count]()
        {
            uint64_t items = (items_to_produce / producers_count);
            for (uint64_t i = 0; i < items; ++i)
            {
                logger.Info("Test message");
            }
        }));
    }

    // Wait for all producers threads
    for (auto& producer : producers)
        producer.join();

    // Update benchmark metrics
    context.metrics().AddIterations(items_to_produce - 1);
    context.metrics().AddItems(items_to_produce);
    context.metrics().AddBytes(items_to_produce * sizeof(Record));
}

BENCHMARK_FIXTURE(BinaryConfigFixture, "AsyncProcessor-binary", settings)
{
    test(context, []{ std::this_thread::yield(); });
}

BENCHMARK_MAIN()
